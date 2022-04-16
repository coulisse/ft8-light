#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/AudioRecorder.cpp"
#include "AudioRecorder.hpp"
#include <AudioKitHAL.h>  //you have to manually install it. 
AudioKit kit;  
//auto cfg  = kit.defaultConfig(); 

AudioRecorder::AudioRecorder() {    


    //SD Config
    if(!SD.begin(kit.pinSpiCs())){
        Serial.println("Card Mount Failed");
        return;
    } else {
        Serial.println("Card Mount Success");
    }

    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
      Serial.println("No SD card attached");
      return;
    }
    Serial.print("SD Card Type: ");  
    if(cardType == CARD_MMC){
      Serial.println("MMC");
    } else if(cardType == CARD_SD){
      Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
//    File root = SD.open("/");
//    printDirectory(root, 1);

      // open in read mode
    //LOGLEVEL_AUDIOKIT = AudioKitInfo; 

};


void AudioRecorder::begin() {
  
  auto cfg = kit.defaultConfig(); 
  LOGLEVEL_AUDIOKIT = AudioKitInfo;
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;
  cfg.bits_per_sample =  AUDIO_HAL_BIT_LENGTH_16BITS;
  //cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;  
  kit.begin(cfg);
  i2s_set_clk(I2S_NUM_0,rate,I2S_BITS_PER_SAMPLE_16BIT,I2S_CHANNEL_MONO);   //Setting MONO

}


String AudioRecorder::record (int t) {
  
    String file_name = FILE_WAV_PREFIX+String(this->getTime())+FILE_WAV_SUFFIX;
    
    Serial.println(file_name);

    File file = SD.open(file_name, FILE_WRITE);
    if (!file) {
      return ""; 
    }      

    int start_time = millis();
    int record_time=0;
    while (record_time <t) {
      kit.read(buffer, BUFFER_SIZE);
      file.write(buffer, BUFFER_SIZE);   
      record_time = millis()-start_time;
    }

    file.seek(0);
    CreateWavHeader(header, (t*rate/1000*2)-1);
    file.write(header, headerSize);
    file.close();  
 
    return file_name;
  
}

//void AudioRecorder::play (const char file_name[]) {
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

// Function that gets current epoch time
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
