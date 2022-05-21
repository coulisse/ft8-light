
#ifndef decoder_h
#define decoder_h

#include <Arduino.h>

#include "ft8_lib/ft8/unpack.h"
#include "ft8_lib/ft8/ldpc.h"
#include "ft8_lib/ft8/decode.h"
#include "ft8_lib/ft8/constants.h"
#include "ft8_lib/ft8/encode.h"
#include "ft8_lib/ft8/crc.h"
#include "ft8_lib/common/common.h"
#include "ft8_lib/common/wave.h"
#include "ft8_lib/common/debug.h"
#include "ft8_lib/fft/kiss_fftr.h"
#include "./monitor_struct.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

struct message_list {
    String time_slot;    
    int16_t score;
    float time_sec;
    float freq;
    char text[25]; 
};

#define MAX_MESSAGES 50
struct shared_data {
    String time_slot_tmp;
    size_t bytes_from;
    size_t bytes_read;
    int recording_time;
    int rate;
    uint8_t ** pcm_buffer;
    message_list message[MAX_MESSAGES];
};


const int kMin_score = 10; // Minimum sync score threshold for candidates
const int kMax_candidates = 120;
const int kLDPC_iterations = 20;

const uint8_t kMax_decoded_messages = 50;

const uint8_t kFreq_osr = 2; // Frequency oversampling rate (bin subdivision)
const uint8_t kTime_osr = 2; // Time oversampling rate (symbol subdivision)

static float IRAM_ATTR hann_i(int i, int N);
static float hamming_i(int i, int N);
static float blackman_i(int i, int N);
void waterfall_init(waterfall_t* me, int max_blocks, int num_bins, uint8_t time_osr, uint8_t freq_osr);
void monitor_free(monitor_t* me);
void monitor_process(monitor_t* me, const float* frame);
void monitor_reset(monitor_t* me);
int IRAM_ATTR decode_ft8(uint8_t *pcm_buffer, size_t bytes_read, int sample_rate, message_list *message_display, String time_slot_tmp);

#endif