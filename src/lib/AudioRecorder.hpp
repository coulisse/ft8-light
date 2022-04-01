#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <driver/i2s.h>
#include <vector>

#define ADC_INPUT ADC1_CHANNEL_0 //pin 32
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#define SAMPLEBLOCK 1024

class AudioRecorder  {
      public:  
        AudioRecorder (int timer_id);
        //~AudioRecorder ();
        void setupI2S ();
        void fetch ();
        
        
      private: 
        //ADC
        const int numBands =8;
        uint16_t samples[SAMPLEBLOCK]; //single chunk
        //std::array<uint16_t, SAMPLEBLOCK> samples;
        //uint16_t * signal; //total signal
        //uint16_t signal[1]; //total signal
        //std::vector<std::array<uint16_t, SAMPLEBLOCK>> signal;
        std::vector<uint16_t> signal;
        //const double samplingFrequency = 10000;
        //const double samplingFrequency = 44100;
        const double samplingFrequency = 22050;
        

        const i2s_port_t I2S_PORT = I2S_NUM_0;
        uint16_t offset = (int)ADC_INPUT * 0x1000 + 0xFFF;
        int prv_timer_id;
        
            
};
#endif