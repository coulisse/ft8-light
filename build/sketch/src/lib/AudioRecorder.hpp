#line 1 "/home/corrado/programming/arduino/portable_ft8/src/lib/AudioRecorder.hpp"
#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <SD.h>
#include "Wav.hpp"


class AudioRecorder  {
    public:  
      AudioRecorder (int timer_id);
      //~AudioRecorder ();
      void record (int t);
      
      
    private: 
      static const int BUFFER_SIZE = 1024;
      uint8_t buffer[BUFFER_SIZE];
           
};
#endif