#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <SD.h>
#include "Wav.hpp"




class AudioRecorder  {
    public:  
      AudioRecorder ();
      //~AudioRecorder ();
      String record (int t);
      void play (const char file_name[]);
      
    private: 
      unsigned long getTime();
      static const int BUFFER_SIZE = 1024;
      uint8_t buffer[BUFFER_SIZE];

      const String FILE_WAV_PREFIX = "/portable_ft8-";
      const String FILE_WAV_SUFFIX = ".wav";      
           
};
#endif