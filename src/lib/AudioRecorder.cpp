#include "AudioRecorder.hpp"
#include <AudioKitHAL.h>  //you have to manually install it. 
AudioKit kit;      

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

};

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

String AudioRecorder::record (int t) {
  
      // open in read mode
    LOGLEVEL_AUDIOKIT = AudioKitInfo; 
    auto cfg = kit.defaultConfig(); 
    cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;
    cfg.bits_per_sample =  AUDIO_HAL_BIT_LENGTH_16BITS;
    cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;
    
  
    kit.begin(cfg);

    const int headerSize = 44;  //todo: define as global
    byte header[headerSize]; //todo: define as global

    String file_name = FILE_WAV_PREFIX+String(getTime())+FILE_WAV_SUFFIX;
    
    Serial.print("Start recording: ");
    Serial.println(file_name);

    File file = SD.open(file_name, FILE_WRITE);
    if (!file) {
      Serial.println(" Error writing wav");
      return ""; 
    }      
    
    int start_time = millis();
    int record_time;
    while ((record_time = (millis()-start_time)) <=t) {
      size_t len = kit.read(buffer, BUFFER_SIZE);
      file.write((const byte *)buffer, BUFFER_SIZE);   
    }

    file.seek(0);
    int waveDataSize= record_time * 16000 * 16 * 2 / 8;
    //int waveDataSize= record_time/1000 * 16000 * 16 * 2 / 8;
    CreateWavHeader(header, waveDataSize);
    file.write(header, headerSize);
    file.close();  
    Serial.println("Wav recorded");


    return file_name;
  
}

void AudioRecorder::play (const char file_name[]) {

  // open in write  mode
  LOGLEVEL_AUDIOKIT = AudioKitInfo; 

  auto cfg = kit.defaultConfig();
  //cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;
  cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;
  cfg.dac_output = AUDIO_HAL_DAC_OUTPUT_LINE2;
  kit.begin(cfg);  

  File wavfile = SD.open(file_name);

  Serial.println("Begin to play2:");
  wavfile.seek(44);
  char buff_file[1024];
  while (size_t l = wavfile.readBytes (buff_file, BUFFER_SIZE)) {
    kit.write(buff_file, l);
  }
  wavfile.close();
  Serial.println("Finish");
  //kit.end();
}
