#include "decoder.hpp"



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
    me->mag = (uint8_t* )ps_malloc(mag_size);  
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

    log_v("A0");
    // Check if we can still store more waterfall data
    if (me->wf.num_blocks >= me->wf.max_blocks)
        return;
log_v("A1");
    int offset = me->wf.num_blocks * me->wf.block_stride;
    int frame_pos = 0;
log_v("A2");
    // Loop over block subdivisions
    for (int time_sub = 0; time_sub < me->wf.time_osr; ++time_sub)
    {
log_v("A3");        
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
log_v("A5");
        kiss_fftr(me->fft_cfg, timedata, freqdata);

        // Loop over two possible frequency bin offsets (for averaging)
        for (int freq_sub = 0; freq_sub < me->wf.freq_osr; ++freq_sub)
        {
log_v("A5");            
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
log_v("A6");    
    ++me->wf.num_blocks;
log_v("A7");    
}

void monitor_reset(monitor_t* me)
{
    me->wf.num_blocks = 0;
    me->max_mag = 0;
}


//int decode_ft8(char* wav_path)
int IRAM_ATTR decode_ft8(uint8_t *pcm_buffer, size_t bytes_read, int sample_rate, message_list *message_display, String time_slot_tmp)
{
    
    log_v("entered in decoding");
    bool is_ft8 = true;

    //int32_t num_samples = 15 * sample_rate;
    int32_t num_samples = bytes_read / 2;
    log_v("Free PSRAM: %d", ESP.getFreePsram());
    int16_t raw_data;
    int32_t j=0;
    //todo: try with int16_t instead int32_t (j and i)
    log_v("Decoding bytes_read %d, sample_rate %d", bytes_read, sample_rate);
    /*
    Serial.print("Decoding:  ");
    for (int i=0;i<100;i++){
        Serial.write(pcm_buffer[i]);
    }
    Serial.println(" -----");
    */
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
    for (int frame_pos = 0; frame_pos +mon.block_size <= num_samples; frame_pos +=mon.block_size)
    {
        vTaskDelay(1);
        log_v("a");
        monitor_process(&mon, signal + frame_pos);
        log_v("b");
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
        
        if (!ft8_decode(&mon.wf, cand, &message, kLDPC_iterations, &status))
        {
            
            // printf("000000 %3d %+4.2f %4.0f ~  ---\n", cand->score, time_sec, freq_hz);
            if (status.ldpc_errors > 0)
            {
                log_v( "LDPC decode: %d errors", status.ldpc_errors);
            }
            else if (status.crc_calculated != status.crc_extracted)
            {
                log_v( "CRC mismatch!");
            }
            else if (status.unpack_status != 0)
            {
                log_v( "Error while unpacking!");
            }
            continue;
        }
        
        
         log_v( "Checking hash table for %4.1fs / %4.1fHz [%d]...", time_sec, freq_hz, cand->score);
        uint8_t idx_hash = message.hash % kMax_decoded_messages;
        
        bool found_empty_slot = false;
        bool found_duplicate = false;
        do
        {
            if (decoded_hashtable[idx_hash] == NULL)
            {
                log_v( "Found an empty slot");
                found_empty_slot = true;
            }
            else if ((decoded_hashtable[idx_hash]->hash == message.hash) && (0 == strcmp(decoded_hashtable[idx_hash]->text, message.text)))
            {
                log_v( "Found a duplicate [%s]", message.text);
                found_duplicate = true;
            }
            else
            {
                log_v( "Hash table clash!");
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
      
            memmove(&message_display[0], &message_display[1], (MAX_MESSAGES - 1) * sizeof(message_display[0])); //scroll table
            message_display[MAX_MESSAGES-1].freq = freq_hz;
            message_display[MAX_MESSAGES-1].score = cand->score;
            message_display[MAX_MESSAGES-1].time_sec = time_sec;
            strcpy(message_display[MAX_MESSAGES-1].text,message.text);
            message_display[MAX_MESSAGES-1].time_slot=time_slot_tmp;
        }
    } 

    log_i("decoded %d messages", num_decoded);

    monitor_free(&mon);
    free(signal);

    return 0;
}



