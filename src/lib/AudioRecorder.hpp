#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <SD.h>
#include "Wav.hpp"



class AudioRecorder  {
  
    public:  

      AudioRecorder ();
      //~AudioRecorder ();
      void record (int t, uint8_t*&psd_pcm_buffer, int &rate, int &recording_time, size_t &bytes_read);
      //void play (const char file_name[]);
      void play (String file_name);
      void begin ();

    private: 
      unsigned long getTime();
      static const int BUFFER_SIZE = 4096;
      uint8_t buffer[BUFFER_SIZE];
      const static int rate = 11025;
          
           
};
#endif