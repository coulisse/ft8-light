#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/AudioRecorder.hpp"
#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <SD.h>
#include "Wav.hpp"



class AudioRecorder  {
  
    public:  
      struct record_t {
        size_t bytes_read;
        int recording_time;
        int rate;
        uint8_t * pcm_buffer;
      } record_data_t;
      AudioRecorder ();
      //~AudioRecorder ();
      record_t record (int t);
      //void play (const char file_name[]);
      void play (String file_name);
      void begin ();

    private: 
      unsigned long getTime();
      static const int BUFFER_SIZE = 4096;
      uint8_t buffer[BUFFER_SIZE];
      const static int rate = 11025;
      
      
      //void set_status(status_t status);    

        
           
};
#endif