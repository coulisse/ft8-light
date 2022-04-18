#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/AudioRecorder.hpp"
#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <SD.h>
#include "Wav.hpp"



class AudioRecorder  {
  enum status_t {recording, ready, disable, error};

    public:  
      AudioRecorder ();
      //~AudioRecorder ();
      void record (int t);
      //void play (const char file_name[]);
      void play (String file_name);
      void begin ();
      //status_t get_status();
      static status_t status;
    private: 
      unsigned long getTime();
      static const int BUFFER_SIZE = 4096;
      uint8_t buffer[BUFFER_SIZE];

      const String FILE_WAV_PREFIX = "/portable_ft8-";
      const String FILE_WAV_SUFFIX = ".wav";    

      const static int headerSize = 44;  
      byte header[headerSize]; 

      const static int rate = 11025;
      
      
      //void set_status(status_t status);    

        
           
};
#endif