#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/AudioRecorder.cpp"
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

    SD.rmdir("/recording");
    SD.mkdir("/recording");

};


void AudioRecorder::begin() {
  
  auto cfg = kit.defaultConfig(); 
  LOGLEVEL_AUDIOKIT = AudioKitInfo;
  cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;
  cfg.bits_per_sample =  AUDIO_HAL_BIT_LENGTH_16BITS;
  //cfg.sample_rate = AUDIO_HAL_16K_SAMPLES;  
  kit.begin(cfg);
  //i2s_set_clk(I2S_NUM_0,rate,I2S_BITS_PER_SAMPLE_16BIT,I2S_CHANNEL_MONO);   //Setting MONO

}

AudioRecorder::record_t  AudioRecorder::record (int t) {
  
  
  //String file_name = FILE_WAV_PREFIX+String(this->getTime())+FILE_WAV_SUFFIX;
  log_d("recording");
  i2s_set_clk(I2S_NUM_0,this->rate,I2S_BITS_PER_SAMPLE_16BIT,I2S_CHANNEL_MONO);   //Setting MONO
  int start_time = millis();
  int record_time=0;
  uint8_t *psd_pcm_buffer = (uint8_t* )ps_malloc(1*sizeof(uint8_t));
  int total_bytes_read =0;
  int prev_dim = 0;
  int current_bytes_read=0;
  while (record_time <t) {
    prev_dim = total_bytes_read;
    current_bytes_read=kit.read(buffer, BUFFER_SIZE);
    total_bytes_read += current_bytes_read;
    ps_realloc(psd_pcm_buffer, total_bytes_read);    
    memcpy(&psd_pcm_buffer[prev_dim],buffer,current_bytes_read);
    record_time = millis()-start_time;
  }

  /*
  Serial.print("Audio: ");
  for (int i=0; i<100; i++ ) {
    Serial.write(psd_pcm_buffer[i]);
  }
  Serial.println();
  */
  record_t data;
  data.bytes_read = total_bytes_read;
  data.recording_time = t;
  data.rate=this->rate;
  data.pcm_buffer  = psd_pcm_buffer;

/*
  String file_name=FILE_WAV_PREFIX+this->getTime()+FILE_WAV_SUFFIX;
  Serial.println(file_name);   

  CreateWavHeader(header, (t*rate/1000*2)-1);
  File file = SD.open(file_name, FILE_WRITE);
  if (!file) {
    return data; 
  }    
  file.write(header, headerSize);
  file.write(psd_pcm_buffer, total_bytes_read);         
  file.close();  
  */
//  free(psd_pcm_buffer);

  return data; 
  
}
/*

String AudioRecorder::record (int t) {
  
    String file_name = FILE_WAV_PREFIX+String(this->getTime())+FILE_WAV_SUFFIX;
    
    Serial.println(file_name);

    File file = SD.open(file_name, FILE_WRITE);
    if (!file) {
      return ""; 
    }      
    i2s_set_clk(I2S_NUM_0,rate,I2S_BITS_PER_SAMPLE_16BIT,I2S_CHANNEL_MONO);   //Setting MONO
    int start_time = millis();
    int record_time=0;
    uint8_t *psd_pcm_buffer = (uint8_t* )ps_malloc(1*sizeof(uint8_t));
    int total_bytes_read =0;
    int prev_dim = 0;
    int current_bytes_read=0;
    while (record_time <t) {
      prev_dim = total_bytes_read;
      current_bytes_read=kit.read(buffer, BUFFER_SIZE);
      total_bytes_read += current_bytes_read;
      ps_realloc(psd_pcm_buffer, total_bytes_read);    
      memcpy(&psd_pcm_buffer[prev_dim],buffer,current_bytes_read);
      record_time = millis()-start_time;
    }
    CreateWavHeader(header, (t*rate/1000*2)-1);
    file.write(header, headerSize);
    file.write(psd_pcm_buffer, total_bytes_read);         
//    file.seek(0);
    file.close();  
    free(psd_pcm_buffer);
 
    return file_name;
  
}

*/
/*


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

*/



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
