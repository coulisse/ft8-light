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
      //void play (const char file_name[]);
      void play (String file_name);
      void begin ();
      
    private: 
      unsigned long getTime();
      static const int BUFFER_SIZE = 2048;
      uint8_t buffer[BUFFER_SIZE];

      const String FILE_WAV_PREFIX = "/portable_ft8-";
      const String FILE_WAV_SUFFIX = ".wav";    

      const static int headerSize = 44;  
      byte header[headerSize]; 

      const static int rate = 11025;
      

        
           
};
#endif