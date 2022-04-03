#line 1 "/home/corrado/programming/arduino/portable_ft8/src/lib/AudioRecorder.cpp"
#include "AudioRecorder.hpp"
#include <AudioKitHAL.h>  //you have to manually install it. 
AudioKit kit;      

AudioRecorder::AudioRecorder(int timer_id) {    
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


void AudioRecorder::record (int t) {

  
      // open in read mode
    auto cfg = kit.defaultConfig(AudioInput); 
    cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;
    cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;
    kit.begin(cfg);

    const int headerSize = 44;
    byte header[headerSize];

    File file = SD.open("/prova8.wav", FILE_WRITE);
    if (!file) {
      Serial.println(" Error");
      return;
    }      
    
    Serial.println("Start recording...");

    int start_time = millis();
    int record_time;
    while ((record_time = (start_time - millis())) <=t) {
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
  
}