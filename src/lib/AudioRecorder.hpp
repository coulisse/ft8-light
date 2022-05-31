/**
 * @file AudioRecorder.hpp
 * @author Corrado Gerbaldo (corrado.gerbaldo@gmail.com)
 * @brief Class used for managing audio 
 * 
 */
#ifndef AudioRecorder_h
#define AudioRecorder_h
#include <Arduino.h>
#include <SD.h>
#include "Wav.hpp"

/**
 * @brief Class used for managing audio 
 * 
 */
class AudioRecorder  {
  
    public:  

      AudioRecorder ();
      //~AudioRecorder ();
      /**
      * @brief record data from I2S stream and store it to a PCM buffer in psram 
      * @param t recording time
      * @param psd_pcm_buffer (by ref) PCM buffer in psram where store recorded audio
      * @param rate (by ref) recoding rate (bytes per second)
      * @param recording_time (by ref) recordin duration 
      * @param bytes_from (by ref) bytes start to read
      * @param bytes_read (by ref) bytes read and stored in to pcm buffer after recording
      */
      void record (int t, uint8_t*&psd_pcm_buffer, int &rate, int &recording_time, size_t &bytes_from, size_t &bytes_read);
      //void play (const char file_name[]);
      void play (String file_name);
      /**
       * @brief init of class
       * 
       */
      void begin ();

    private: 
      unsigned long getTime();
      static const int BUFFER_SIZE = 4096;
      uint8_t buffer[BUFFER_SIZE];
      const static int rate = 11025;
           
};
#endif