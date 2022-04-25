# 1 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
# 2 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 3 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 4 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 5 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 6 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
//-----------------------------------------------------------------------
# 8 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 9 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 10 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2

# 12 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2

# 14 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 15 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 16 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 17 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 18 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 19 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 20 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 21 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 22 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 23 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2
# 24 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 2

#define LOG_LEVEL LOG_INFO

const int kMin_score = 10; // Minimum sync score threshold for candidates
const int kMax_candidates = 120;
const int kLDPC_iterations = 20;

const int kMax_decoded_messages = 50;

const int kFreq_osr = 2; // Frequency oversampling rate (bin subdivision)
const int kTime_osr = 2; // Time oversampling rate (symbol subdivision)


static float hann_i(int i, int N)
{
    float x = sinf((float)3.14159265358979323846 * i / N);
    return x * x;
}

static float hamming_i(int i, int N)
{
    const float a0 = (float)25 / 46;
    const float a1 = 1 - a0;

    float x1 = cosf(2 * (float)3.14159265358979323846 * i / N);
    return a0 - a1 * x1;
}

static float blackman_i(int i, int N)
{
    const float alpha = 0.16f; // or 2860/18608
    const float a0 = (1 - alpha) / 2;
    const float a1 = 1.0f / 2;
    const float a2 = alpha / 2;

    float x1 = cosf(2 * (float)3.14159265358979323846 * i / N);
    float x2 = 2 * x1 * x1 - 1; // Use double angle formula

    return a0 - a1 * x1 + a2 * x2;
}

void waterfall_init(waterfall_t* me, int max_blocks, int num_bins, int time_osr, int freq_osr)
{
    size_t mag_size = max_blocks * time_osr * freq_osr * num_bins * sizeof(me->mag[0]);
    me->max_blocks = max_blocks;
    me->num_blocks = 0;
    me->num_bins = num_bins;
    me->time_osr = time_osr;
    me->freq_osr = freq_osr;
    me->block_stride = (time_osr * freq_osr * num_bins);
    //me->mag = (uint8_t  *)malloc(mag_size);
    ;
    //me->mag = (uint8_t  *)malloc(1024); 
    ;
    uint8_t *loc_mag = (uint8_t* )ps_malloc(mag_size);
    ;
    me->mag = loc_mag;
    //me->mag = (uint8_t  *)calloc(mag_size/sizeof(uint8_t),sizeof(uint8_t));

    ;
}

void waterfall_free(waterfall_t* me)
{
    free(me->mag);
}

//void monitor_init(monitor_t* me, const monitor_config_t* cfg)
void monitor_init(monitor_t* me, const monitor_config_t* cfg)
{
    float slot_time = (cfg->protocol == PROTO_FT4) ? (7.5f) /*|< FT4 slot period*/ : (15.0f) /*|< FT8 slot period*/;
    float symbol_period = (cfg->protocol == PROTO_FT4) ? (0.048f) /*|< FT4 symbol duration, defines tone deviation in Hz and symbol rate*/ : (0.160f) /*|< FT8 symbol duration, defines tone deviation in Hz and symbol rate*/;
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

    ;
    ;
    ;
    ;

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
void monitor_process(monitor_t* me, const uint8_t* frame)
//void monitor_process(monitor_t* me, const float* frame)
//void monitor_process(monitor_t* me, uint8_t* frame)
{
/*
    log_v("======================================");
    for (int i=0; i<100; i++){
      Serial.write(frame[i]);
    }
    Serial.println();
*/
    // Check if we can still store more waterfall data
    if (me->wf.num_blocks >= me->wf.max_blocks)
        return;

    int offset = me->wf.num_blocks * me->wf.block_stride;
    int frame_pos = 0;

    // Loop over block subdivisions
    for (int time_sub = 0; time_sub < me->wf.time_osr; ++time_sub)
    {
        float timedata[me->nfft];
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
int decode_ft8(uint8_t *pcm_buffer, size_t bytes_read)
{

    ;
    bool is_ft8 = true;
/*
    log_v("======================================");
    for (int i=0; i<100; i++){
      Serial.write(signal[i]);
    }
    Serial.println();
*/
    //int sample_rate = 12000;
    int sample_rate = 11025;
    int32_t num_samples = 15 * sample_rate;
    uint8_t *signal = pcm_buffer;
    /*
    float *signal = (float* )ps_malloc(num_samples*sizeof(float));
    memcpy(signal,pcm_buffer,bytes_read/sizeof(uint8_t)*sizeof(float));
    free(pcm_buffer);
    */

/*
    log_v("creating signal array of %d samples", num_samples);
    //float signal[num_samples];
    float *signal = (float* )ps_malloc(num_samples*sizeof(float));
    log_v("before loading wav");
    int rc = load_wav(signal, &num_samples, &sample_rate, wav_path);
    log_v("check return code");
    if (rc < 0)
    {
        return -1;
    } 
    log_v("after loading wav");
*/
    ;
    ;

    // Compute FFT over the whole signal and store it
    //monitor_t mon;
    //monitor_t *mon = (monitor_t *)ps_malloc(sizeof(monitor_t));
    //monitor_t *mon = (monitor_t *)ps_calloc(1,sizeof(monitor_t));
    monitor_t *mon = (monitor_t *)malloc(sizeof(monitor_t));
    ;

    monitor_config_t mon_cfg = {
        .f_min = 100,
        .f_max = 3000,
        .sample_rate = sample_rate,
        .time_osr = kTime_osr,
        .freq_osr = kFreq_osr,
        .protocol = is_ft8 ? PROTO_FT8 : PROTO_FT4
    };
    ;
    //monitor_init(&mon, &mon_cfg);
    monitor_init(mon, &mon_cfg);


    ;
    ;
    //for (int frame_pos = 0; frame_pos + mon->block_size <= num_samples; frame_pos += mon->block_size)
    for (uint8_t frame_pos = 0; frame_pos + mon->block_size <= num_samples; frame_pos += mon->block_size)
    {
        // Process the waveform data frame by frame - you could have a live loop here with data from an audio device
        //log_v("(frame_pos + mon->block_size) = %d < num_samples = %d", frame_pos + mon->block_size, num_samples);
        ;
        vTaskDelay(1);
        //monitor_process(&mon, signal + frame_pos);
        monitor_process(mon, signal + frame_pos);
    }
    ;
    ;
    ;

    // Find top candidates by Costas sync score and localize them in time and frequency
    candidate_t candidate_list[kMax_candidates];
    int num_candidates = ft8_find_sync(&mon->wf, kMax_candidates, candidate_list, kMin_score);

    // Hash table for decoded messages (to check for duplicates)
    int num_decoded = 0;
    message_t decoded[kMax_decoded_messages];
    message_t* decoded_hashtable[kMax_decoded_messages];

    // Initialize hash table pointers
    for (int i = 0; i < kMax_decoded_messages; ++i)
    {
        decoded_hashtable[i] = 
# 299 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 3 4
                              __null
# 299 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
                                  ;
    }

    // Go over candidates and attempt to decode messages
    for (int idx = 0; idx < num_candidates; ++idx)
    {
        const candidate_t* cand = &candidate_list[idx];
        if (cand->score < kMin_score)
            continue;

        float freq_hz = (cand->freq_offset + (float)cand->freq_sub / mon->wf.freq_osr) / mon->symbol_period;
        float time_sec = (cand->time_offset + (float)cand->time_sub / mon->wf.time_osr) * mon->symbol_period;

        message_t message;
        decode_status_t status;
        if (!ft8_decode(&mon->wf, cand, &message, kLDPC_iterations, &status))
        {
            // printf("000000 %3d %+4.2f %4.0f ~  ---\n", cand->score, time_sec, freq_hz);
            if (status.ldpc_errors > 0)
            {
                ;
            }
            else if (status.crc_calculated != status.crc_extracted)
            {
                ;
            }
            else if (status.unpack_status != 0)
            {
                ;
            }
            continue;
        }

        ;
        int idx_hash = message.hash % kMax_decoded_messages;
        bool found_empty_slot = false;
        bool found_duplicate = false;
        do
        {
            if (decoded_hashtable[idx_hash] == 
# 338 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 3 4
                                              __null
# 338 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
                                                  )
            {
                ;
                found_empty_slot = true;
            }
            else if ((decoded_hashtable[idx_hash]->hash == message.hash) && (0 == strcmp(decoded_hashtable[idx_hash]->text, message.text)))
            {
                ;
                found_duplicate = true;
            }
            else
            {
                ;
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
            int snr = 0; // TODO: compute SNR
            printf("000000 %3d %+4.2f %4.0f ~  %s\n", cand->score, time_sec, freq_hz, message.text);
        }
    }
    ;

    monitor_free(mon);

    return 0;
}



//-----------------------------------------------------------------------




/* Display related */
#define PIN_DISPLAY_CK 18
#define PIN_DISPLAY_DT 23


enum ft_phase {decode, encode, nothing};
ft_phase phase = decode;

//U8G2 display config
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2((&u8g2_cb_r0), /* clock=*/ 18, /* data=*/ 23, /* reset=*/ 255); // Adafruit Feather M0 Basic Proto + FeatherWing OLED

TimeManager tm("hyperline-11635","auj6xai6iN","pool.ntp.org");

AudioRecorder ar;


TaskHandle_t Task_ar;


void setup() {

  Serial.begin(115200);
  ;
  ;
  ;
  ;

  //Display Init
  u8g2.begin();
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(0,10,"Boot..."); // write something to the internal memory
  u8g2.sendBuffer(); // transfer internal memory to the display  
  ;
  monitor_t *prova = (monitor_t *)ps_malloc(sizeof(monitor_t));
  ;


  //String recorded_file = ar.record(3000);
  //ar.play(recorded_file);

  //objects init
  if (!tm.align_timer()){
    ;
    while (true){
      //stop here
    }
  };
  ar.begin();
}


void tsk_decode(void * parameters) {

  AudioRecorder::record_t * data = (AudioRecorder::record_t * ) parameters;

  //get parameters
  //TODO: use mutex
  size_t bytes_read = data->bytes_read;
  int recording_time = data->recording_time;
  int rate = data->rate;
  uint8_t *psd_pcm_buffer = (uint8_t* )ps_malloc(bytes_read);
  memcpy(psd_pcm_buffer,data->pcm_buffer,bytes_read);
  free(data->pcm_buffer);

  ;

  String file_name=FILE_WAV_PATH+FILE_WAV_PREFIX+millis()+FILE_WAV_SUFFIX;
  char * file_name_c = new char[file_name.length() + 1];
  strcpy(file_name_c,file_name.c_str());

  //create header
  int headerSize = 44;
  byte header[headerSize];
  CreateWavHeader(header, (recording_time*rate/1000*2)-1);

  //writing file
  File file = SD.open(file_name_c, "w");
  if (!file) {
    ;
    vTaskDelete(
# 462 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 3 4
               __null
# 462 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
                   );
  }
  file.write(header, headerSize);
  file.write(psd_pcm_buffer, bytes_read);
  ;
  file.close();


  //int d = decode_main(file_name_c,rate);
  ;
  ;
  ;
  /*
  log_v("======================================");
  for (int i=0; i<100; i++){
    Serial.write(psd_pcm_buffer[i]);
  }
  Serial.println();
  */
  vTaskDelay(50);
  decode_ft8(psd_pcm_buffer,bytes_read);
  //decode_ft8(file_name_c);

  free(psd_pcm_buffer);
  ;
  vTaskDelete(
# 487 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 3 4
             __null
# 487 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
                 );
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

        AudioRecorder::record_t data = ar.record(14000);
//        decode_ft8(data.pcm_buffer,data.bytes_read); //todo:remove
        xTaskCreatePinnedToCore(tsk_decode,"Task1",60000,&data,2,
# 506 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino" 3 4
                                                                __null
# 506 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
                                                                    ,0);
        delay(100);
        phase = encode; //todo: remove;

      } else if (phase == encode) {
        //ar.play("/sample_16000_16_mono.wav");
      }
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    }
  }



}
