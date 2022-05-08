//TODO: try to put decode in a module / object
//TODO: try to compile with a better code optimization
//TODO: check longer times

#include <Wire.h>
#include <U8g2lib.h>
#include "src/lib/TimeManager.hpp"
#include "src/lib/AudioRecorder.hpp"
#include "src/lib/config.h"
//-----------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "src/lib/ft8_lib/ft8/unpack.h"
#include "src/lib/ft8_lib/ft8/ldpc.h"
#include "src/lib/ft8_lib/ft8/decode.h"
#include "src/lib/ft8_lib/ft8/constants.h"
#include "src/lib/ft8_lib/ft8/encode.h"
#include "src/lib/ft8_lib/ft8/crc.h"
#include "src/lib/ft8_lib/common/common.h"
#include "src/lib/ft8_lib/common/wave.h"
#include "src/lib/ft8_lib/common/debug.h"
#include "src/lib/ft8_lib/fft/kiss_fftr.h"
#include "monitor_struct.h"

#define LOG_LEVEL LOG_INFO

struct shared_data {
    size_t bytes_read;
    int recording_time;
    int rate;
    uint8_t ** pcm_buffer;
} shared_data_t;


const int kMin_score = 10; // Minimum sync score threshold for candidates
const int kMax_candidates = 120;
const int kLDPC_iterations = 20;

const uint8_t kMax_decoded_messages = 50;

const uint8_t kFreq_osr = 2; // Frequency oversampling rate (bin subdivision)
const uint8_t kTime_osr = 2; // Time oversampling rate (symbol subdivision)


static float IRAM_ATTR hann_i(int i, int N)
{
    float x = sinf((float)M_PI * i / N);
    return x * x;
}

static float hamming_i(int i, int N)
{
    const float a0 = (float)25 / 46;
    const float a1 = 1 - a0;

    float x1 = cosf(2 * (float)M_PI * i / N);
    return a0 - a1 * x1;
}

static float blackman_i(int i, int N)
{
    const float alpha = 0.16f; // or 2860/18608
    const float a0 = (1 - alpha) / 2;
    const float a1 = 1.0f / 2;
    const float a2 = alpha / 2;

    float x1 = cosf(2 * (float)M_PI * i / N);
    float x2 = 2 * x1 * x1 - 1; // Use double angle formula

    return a0 - a1 * x1 + a2 * x2;
}

void waterfall_init(waterfall_t* me, int max_blocks, int num_bins, uint8_t time_osr, uint8_t freq_osr)
{
    size_t mag_size = max_blocks * time_osr * freq_osr * num_bins * sizeof(me->mag[0]);
    me->max_blocks = max_blocks;
    me->num_blocks = 0;
    me->num_bins = num_bins;
    me->time_osr = time_osr;
    me->freq_osr = freq_osr;
    me->block_stride = (time_osr * freq_osr * num_bins);
    log_v("Number of element in mag: %d", mag_size/sizeof(uint8_t));
    log_v("allocating local mag");
    //uint8_t *loc_mag = (uint8_t* )ps_malloc(mag_size);    
    me->mag = (uint8_t* )ps_malloc(mag_size);  
    //log_v("assign local mag to structure");
    //me->mag = loc_mag;
    log_d("Waterfall size = %zu", mag_size);
    log_v("Free PSRAM.: %d", ESP.getFreePsram());
}

void waterfall_free(waterfall_t* me)
{
    free(me->mag);
}

//void monitor_init(monitor_t* me, const monitor_config_t* cfg)
void monitor_init(monitor_t* me, const monitor_config_t* cfg)
{
    float slot_time = (cfg->protocol == PROTO_FT4) ? FT4_SLOT_TIME : FT8_SLOT_TIME;
    float symbol_period = (cfg->protocol == PROTO_FT4) ? FT4_SYMBOL_PERIOD : FT8_SYMBOL_PERIOD;
    // Compute DSP parameters that depend on the sample rate
    me->block_size = (int)(cfg->sample_rate * symbol_period); // samples corresponding to one FSK symbol
    me->subblock_size = me->block_size / cfg->time_osr;
    me->nfft = me->block_size * cfg->freq_osr;
    me->fft_norm = 2.0f / me->nfft;
    // const int len_window = 1.8f * me->block_size; // hand-picked and optimized

    me->window = (float *)malloc(me->nfft * sizeof(me->window[0]));
    for (int i = 0; i < me->nfft; ++i)
    {
        // window[i] = 1;
        me->window[i] = hann_i(i, me->nfft);
        // me->window[i] = blackman_i(i, me->nfft);
        // me->window[i] = hamming_i(i, me->nfft);
        // me->window[i] = (i < len_window) ? hann_i(i, len_window) : 0;
    }
    me->last_frame = (float *)malloc(me->nfft * sizeof(me->last_frame[0]));

    size_t fft_work_size;
    kiss_fftr_alloc(me->nfft, 0, 0, &fft_work_size);

    log_i( "Block size = %d", me->block_size);
    log_i( "Subblock size = %d", me->subblock_size);
    log_i( "N_FFT = %d", me->nfft);
    log_d( "FFT work area = %zu", fft_work_size);

    me->fft_work = malloc(fft_work_size);
    me->fft_cfg = kiss_fftr_alloc(me->nfft, 0, me->fft_work, &fft_work_size);

    const int max_blocks = (int)(slot_time / symbol_period);
    const int num_bins = (int)(cfg->sample_rate * symbol_period / 2);
    waterfall_init(&me->wf, max_blocks, num_bins, cfg->time_osr, cfg->freq_osr);
    me->wf.protocol = cfg->protocol;
    me->symbol_period = symbol_period;

    me->max_mag = -120.0f;
}

void monitor_free(monitor_t* me)
{
    waterfall_free(&me->wf);
    free(me->fft_work);
    free(me->last_frame);
    free(me->window);
}

// Compute FFT magnitudes (log wf) for a frame in the signal and update waterfall data
void monitor_process(monitor_t* me, const float* frame)
{

    // Check if we can still store more waterfall data
    if (me->wf.num_blocks >= me->wf.max_blocks)
        return;

    int offset = me->wf.num_blocks * me->wf.block_stride;
    int frame_pos = 0;

    // Loop over block subdivisions
    for (int time_sub = 0; time_sub < me->wf.time_osr; ++time_sub)
    {
        kiss_fft_scalar timedata[me->nfft];
        kiss_fft_cpx freqdata[me->nfft / 2 + 1];
        // Shift the new data into analysis frame
        for (int pos = 0; pos < me->nfft - me->subblock_size; ++pos)
        {
            me->last_frame[pos] = me->last_frame[pos + me->subblock_size];
        }
        for (int pos = me->nfft - me->subblock_size; pos < me->nfft; ++pos)
        {
            me->last_frame[pos] = frame[frame_pos];
            ++frame_pos;
        }
        // Compute windowed analysis frame
        for (int pos = 0; pos < me->nfft; ++pos)
        {
            timedata[pos] = me->fft_norm * me->window[pos] * me->last_frame[pos];
        }

        kiss_fftr(me->fft_cfg, timedata, freqdata);

        // Loop over two possible frequency bin offsets (for averaging)
        for (int freq_sub = 0; freq_sub < me->wf.freq_osr; ++freq_sub)
        {
            for (int bin = 0; bin < me->wf.num_bins; ++bin)
            {
                int src_bin = (bin * me->wf.freq_osr) + freq_sub;
                float mag2 = (freqdata[src_bin].i * freqdata[src_bin].i) + (freqdata[src_bin].r * freqdata[src_bin].r);
                float db = 10.0f * log10f(1E-12f + mag2);
                // Scale decibels to unsigned 8-bit range and clamp the value
                // Range 0-240 covers -120..0 dB in 0.5 dB steps
                int scaled = (int)(2 * db + 240);
                //log_v("offset %d, me->wf.mag[offset]= %d", offset, me->wf.mag[offset] );
                me->wf.mag[offset] = (scaled < 0) ? 0 : ((scaled > 255) ? 255 : scaled);
                ++offset;
                if (db > me->max_mag)
                    me->max_mag = db;
            }
        }
    }
    ++me->wf.num_blocks;
}

void monitor_reset(monitor_t* me)
{
    me->wf.num_blocks = 0;
    me->max_mag = 0;
}


//int decode_ft8(char* wav_path)
int IRAM_ATTR decode_ft8(uint8_t *pcm_buffer, size_t bytes_read, int sample_rate)
{
    
    log_v("entered in decoding");
    bool is_ft8 = true;

    //int32_t num_samples = 15 * sample_rate;
    int32_t num_samples = bytes_read / 2;
    log_v("Free PSRAM: %d", ESP.getFreePsram());
    int16_t raw_data;
    int32_t j=0;
    //todo: try with int16_t instead int32_t (j and i)
    //todo: try other methods for convert byte in int
    float *signal = (float* )ps_malloc(num_samples*sizeof(float));
    for (int i=0;i<num_samples;i++){
      raw_data =  pcm_buffer[j] | pcm_buffer[j+1] << 8;
      //signal[i]=raw_data/ 32768.0f;
      signal[i]=raw_data * (1.0f / 32768.0f);
      j+=2;
    };
    log_v("Free PSRAM.: %d", ESP.getFreePsram());
    log_i("Sample rate %d Hz, %d samples, %.3f seconds", sample_rate, num_samples, (double)num_samples / sample_rate);

    // Compute FFT over the whole signal and store it
    monitor_t mon;
    //monitor_t *mon = (monitor_t *)malloc(sizeof(monitor_t));  
    log_v("monitor allocated");
 
    monitor_config_t mon_cfg = {
        .f_min = 100,
        .f_max = 3000,
        .sample_rate = sample_rate,
        .time_osr = kTime_osr,
        .freq_osr = kFreq_osr,
        .protocol = is_ft8 ? PROTO_FT8 : PROTO_FT4
    };
    monitor_init(&mon, &mon_cfg);  
    
    log_d("Waterfall allocated %d symbols",mon.wf.max_blocks);
    //for (int frame_pos = 0; frame_pos +mon.block_size <= num_samples; frame_pos +=mon.block_size)
    for (int frame_pos = 0; frame_pos +mon.block_size <= num_samples; frame_pos +=mon.block_size)
    {
        vTaskDelay(1);
        monitor_process(&mon, signal + frame_pos);
    }
    log_v("after monitor loop");
    log_d( "Waterfall accumulated %d symbols",mon.wf.num_blocks);
    log_i( "Max magnitude: %.1f dB",mon.max_mag);
    
    // Find top candidates by Costas sync score and localize them in time and frequency
    candidate_t candidate_list[kMax_candidates];
    uint8_t num_candidates = (uint8_t) ft8_find_sync(&mon.wf, kMax_candidates, candidate_list, kMin_score);

    // Hash table for decoded messages (to check for duplicates)
    uint8_t num_decoded = 0;
    message_t decoded[kMax_decoded_messages];

    /*
    message_t* decoded_hashtable[kMax_decoded_messages];

    // Initialize hash table pointers
    for (int i = 0; i < kMax_decoded_messages; ++i)
    {
        decoded_hashtable[i] = NULL;
    }
    */

    message_t* decoded_hashtable[kMax_decoded_messages] = {};
    // Go over candidates and attempt to decode messages
    log_i("Number of candidates: %d", num_candidates);
    for (uint8_t idx = 0; idx < num_candidates; ++idx)
    {
        vTaskDelay(1);
        const candidate_t* cand = &candidate_list[idx];
        if (cand->score < kMin_score)
            continue;

        float freq_hz = (cand->freq_offset + (float)cand->freq_sub /mon.wf.freq_osr) /mon.symbol_period;
        float time_sec = (cand->time_offset + (float)cand->time_sub /mon.wf.time_osr) *mon.symbol_period;

        message_t message;
        decode_status_t status;
        ft8_decode(&mon.wf, cand, &message, kLDPC_iterations, &status);
        /*
        if (!ft8_decode(&mon.wf, cand, &message, kLDPC_iterations, &status))
        {
            
            // printf("000000 %3d %+4.2f %4.0f ~  ---\n", cand->score, time_sec, freq_hz);
            if (status.ldpc_errors > 0)
            {
                log_d( "LDPC decode: %d errors", status.ldpc_errors);
            }
            else if (status.crc_calculated != status.crc_extracted)
            {
                log_d( "CRC mismatch!");
            }
            else if (status.unpack_status != 0)
            {
                log_d( "Error while unpacking!");
            }
            continue;
        }
        */
        
         log_d( "Checking hash table for %4.1fs / %4.1fHz [%d]...", time_sec, freq_hz, cand->score);
        uint8_t idx_hash = message.hash % kMax_decoded_messages;
        
        bool found_empty_slot = false;
        bool found_duplicate = false;
        do
        {
            if (decoded_hashtable[idx_hash] == NULL)
            {
                log_d( "Found an empty slot");
                found_empty_slot = true;
            }
            else if ((decoded_hashtable[idx_hash]->hash == message.hash) && (0 == strcmp(decoded_hashtable[idx_hash]->text, message.text)))
            {
                log_d( "Found a duplicate [%s]", message.text);
                found_duplicate = true;
            }
            else
            {
                log_d( "Hash table clash!");
                // Move on to check the next entry in hash table
                idx_hash = (idx_hash + 1) % kMax_decoded_messages;
            }
        } while (!found_empty_slot && !found_duplicate);

        if (found_empty_slot)
        {
            
        
            // Fill the empty hashtable slot
            memcpy(&decoded[idx_hash], &message, sizeof(message));
            decoded_hashtable[idx_hash] = &decoded[idx_hash];
            ++num_decoded;

            // Fake WSJT-X-like output for now
            //int snr = 0; // TODO: compute SNR
            //printf("000000 %3d %+4.2f %4.0f ~  %s\n", cand->score, time_sec, freq_hz, message.text);
            log_i("000000 %3d %+4.2f %4.0f ~  %s", cand->score, time_sec, freq_hz, message.text);
        }
    }
    log_i("decoded %d messages", num_decoded);

    monitor_free(&mon);
    free(signal);

    return 0;
}



//-----------------------------------------------------------------------




/* Display related */
#define PIN_DISPLAY_CK 18
#define PIN_DISPLAY_DT 23


enum ft_phase {decode, encode, nothing};
ft_phase phase = decode;

//U8G2 display config
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED

TimeManager tm(ssid,password,ntpServer);

AudioRecorder ar;

/* definitions for task management */
TaskHandle_t xHandleRecord;
TaskHandle_t xHandleDecode;
#define TASKRECORD_BIT (1UL<<0UL) //Unsigned long bit0 set to 1
EventGroupHandle_t xEventGroup;
shared_data data;

void setup() {




  Serial.begin(115200);
  log_v("Total heap.: %d", ESP.getHeapSize());
  log_i("Free heap..: %d", ESP.getFreeHeap());
  log_v("Total PSRAM: %d", ESP.getPsramSize());
  log_v("Free PSRAM.: %d", ESP.getFreePsram());  

  //Display Init
  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Boot...");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display  

  ar.begin();

  //tasks
    //data.pcm_buffer = (uint8_t** )ps_malloc(1*sizeof(uint8_t));
  data.pcm_buffer = (uint8_t** )ps_malloc(400000*sizeof(uint8_t));
  xEventGroup = xEventGroupCreate();
  log_v("init tsk_record");
  xTaskCreatePinnedToCore(tsk_record,"recording task",30000,&data,1,&xHandleRecord,1); 
  delay(15000);
  log_v("init tsk_decode");
  xTaskCreatePinnedToCore(tsk_decode,"decoding  task",40000,&data,1,&xHandleDecode,0);   
  delay(15000);


  //objects init
  if (!tm.align_timer()){
    log_e("initialization failed");
    while (true){
      //stop here
    }
  };


}


void tsk_record(void * parameters) {  


   for (;;) {
        shared_data* data = (shared_data*) parameters;
        //log_v("malloc buffer");
        //free(data->pcm_buffer);
        //data->pcm_buffer = (uint8_t** )ps_malloc(1*sizeof(uint8_t));
        //ps_realloc(data->pcm_buffer,1*sizeof(uint8_t));
        
        log_v("recording..");
        ar.record(RECORDING_TIME,(uint8_t*&) data->pcm_buffer,data->rate,data->recording_time,data->bytes_read);
        //ar.record(RECORDING_TIME,(uint8_t*&) parameters,r,rt,br);
        log_v("rate %d, recording time %d, bytes read %d", data->rate,data->recording_time,data->bytes_read);
    //    vTaskDelete(NULL);
        log_v("setting bit for decoding...");
        xEventGroupSetBits(xEventGroup,TASKRECORD_BIT); //Event,bit
        log_v("Suspending recording task..."); 
        vTaskSuspend(NULL);

        log_v("Recording task resumed"); 
    }
}



void tsk_decode(void * parameters) {  

    const EventBits_t xBitsToWaitFor = TASKRECORD_BIT;
    EventBits_t xEventGroupValue;

    for (;;) {
        xEventGroupValue = xEventGroupWaitBits(xEventGroup,xBitsToWaitFor,pdTRUE,pdTRUE,portMAX_DELAY);//EventGroup,receive, clear on exit, all bits?,wait
        if(xEventGroupValue & TASKRECORD_BIT != 0) {        
            //get parameters
            shared_data * data = (shared_data* ) parameters;

            size_t bytes_read =  data->bytes_read;
            int recording_time =  data->recording_time;
            int rate = data->rate;
            uint8_t *lcl_pcm_buffer = (uint8_t* )ps_malloc(bytes_read);  

            memcpy(lcl_pcm_buffer,data->pcm_buffer,bytes_read);
            //free(data->pcm_buffer);
            vTaskDelay(1);  // one tick delay (15ms) in between reads for stability 

            
            #ifdef WRITEWAV 
            String file_name=FILE_WAV_PATH+FILE_WAV_PREFIX+millis()+FILE_WAV_SUFFIX;
            char * file_name_c = new char[file_name.length() + 1];
            strcpy(file_name_c,file_name.c_str()); 

            //create header
            int headerSize = 44;  
            byte header[headerSize]; 
            CreateWavHeader(header, (recording_time*rate/1000*2)-1);

            //writing file
            File file = SD.open(file_name_c, FILE_WRITE);
            if (!file) {
                log_e("Could not write file");
                vTaskDelete(NULL);
            }    
            file.write(header, headerSize);
            file.write(lcl_pcm_buffer, bytes_read);    
            log_d("PCM Bytes: %d", bytes_read);
            file.close();  

            log_i ("recorded: %s", file_name_c);

            #endif
            //  vTaskDelay(50);
            decode_ft8(lcl_pcm_buffer,bytes_read,rate); //TODO: replace with data-> psd??
            log_v("decoded");
            free(lcl_pcm_buffer);  
            log_v("end task"); 
            //vTaskSuspend(NULL);
        }
    }
}



void loop() {
//https://www.appsloveworld.com/download-sample-wav-file-for-testing/

  TimeManager::clock tmpIntr=tm.getClock();
  if (tmpIntr.raised) {
    if (tmpIntr.number % 15 == 0) {
      //portENTER_CRITICAL(&timerMux);
      //portENTER_CRITICAL(&tm.timerMux);
      tm.resetClock();

      if (phase==decode) {
        //TODO: record in a separate task
        log_v("before resume");
        vTaskResume(xHandleRecord);
        //ar.record(RECORDING_TIME,(uint8_t*&) data.pcm_buffer,data.rate,data.recording_time,data.bytes_read);
        log_v("before create task decode");
        //xTaskCreatePinnedToCore(tsk_decode,"decodint task",40000,&data,1,NULL,0); 
        //xTaskCreatePinnedToCore(tsk_decode,"decoding task",40000,&data,1,&xHandleDecode,0); 
        //delay(80); //TODO: remove delay


      } else if (phase == encode) {
        //ar.play("/sample_16000_16_mono.wav");
      }
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    } 
  }



}  
 