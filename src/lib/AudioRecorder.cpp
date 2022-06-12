/**
 * @file AudioRecorder.cpp
 * @author Corrado Gerbaldo (corrado.gerbaldo@gmail.com)
 * @brief Class used for managing audio 
 * 
 */

#include "AudioRecorder.hpp"
#include <AudioKitHAL.h>  //you have to manually install it. 
AudioKit kit;  
//auto cfg  = kit.defaultConfig(); 

AudioRecorder::AudioRecorder() {    

    //SD Config
    if(!SD.begin(kit.pinSpiCs())){
        log_e("Card Mount Failed");
        return;
    } else {
        log_i("Card Mount Success");
    }

    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
      log_e("No SD card attached");
      return;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
//    File root = SD.open("/");
//    printDirectory(root, 1);

      // open in read mode
    //LOGLEVEL_AUDIOKIT = AudioKitInfo; 

    #ifdef WRITEWAV 
    SD.rmdir("/recording");
    SD.mkdir("/recording");
    #endif

};

/**
 * @brief init of class
 * 
 */
void AudioRecorder::begin() { 
  auto cfg = kit.defaultConfig(); 
  LOGLEVEL_AUDIOKIT = AudioKitInfo;
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;
  cfg.bits_per_sample =  AUDIO_HAL_BIT_LENGTH_16BITS;
  //cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;  
  kit.begin(cfg);
  //i2s_set_clk(I2S_NUM_0,rate,I2S_BITS_PER_SAMPLE_16BIT,I2S_CHANNEL_MONO);   //Setting MONO
}

/**
* @brief record data from I2S stream and store it to a PCM buffer in psram 
* @param t recording time
* @param psd_pcm_buffer (by ref) PCM buffer in psram where store recorded audio
* @param rate (by ref) recoding rate (bytes per second)
* @param recording_time (by ref) recordin duration 
* @param bytes_from (by ref) bytes start to read
* @param bytes_read (by ref) bytes read and stored in to pcm buffer after recording
*/
void  AudioRecorder::record (int t, uint8_t*&psd_pcm_buffer, int &shr_rate, int &shr_recording_time, size_t &shr_bytes_from, size_t &shr_bytes_read) {
   //TODO: return data without buffer
  i2s_set_clk(I2S_NUM_0,this->rate,I2S_BITS_PER_SAMPLE_16BIT,I2S_CHANNEL_MONO);   //Setting MONO
  int start_time = millis();
  int recording_time=0;
  int total_bytes_read =0;
  int prev_dim = 0;
  int current_bytes_read=0;
  int start = 0;

  
  while (recording_time <t) {
    prev_dim = total_bytes_read;
    //TODO: try to read int instead of bytes 
    current_bytes_read=kit.read(buffer, BUFFER_SIZE);
    /*
    if (start==0) {
      start=1;
      for (int i=0;i<100;i++){
        Serial.write(buffer[i]);
      }
      Serial.println(" -----");
    }
    */
    
    total_bytes_read += current_bytes_read;
    memcpy(&psd_pcm_buffer[prev_dim],buffer,current_bytes_read);
    recording_time = millis()-start_time;
  }

  shr_rate=this->rate;
  shr_recording_time=recording_time;
  shr_bytes_from = 0;
  shr_bytes_read=total_bytes_read;

  return;  
}


void AudioRecorder::play (String file_name) {  

  // open in write  mode
  LOGLEVEL_AUDIOKIT = AudioKitInfo; 

  auto cfg = kit.defaultConfig();
  cfg.sample_rate = AUDIO_HAL_11K_SAMPLES;
  cfg.dac_output = AUDIO_HAL_DAC_OUTPUT_LINE2;
  kit.begin(cfg);  

  int n = file_name.length();
  char recorded_file_array[n + 1];
  strcpy(recorded_file_array, file_name.c_str());  
  File wavfile = SD.open(recorded_file_array);

  Serial.print("Begin to play2:"); 
  Serial.println(recorded_file_array);
  wavfile.seek(44);
  char buff_file[1024];
  while (size_t l = wavfile.readBytes (buff_file, 1024)) {
    kit.write(buff_file, l);
  }
  wavfile.close();
  Serial.println("Finish");
  //kit.end();
}

/**
 * @brief function used to get  current epoch time
 * @return unsigned long with time
 */
unsigned long AudioRecorder::getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
