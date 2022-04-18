#include <Wire.h>
#include <U8g2lib.h>
#include "src/lib/TimeManager.hpp"
#include "src/lib/AudioRecorder.hpp"
#include "src/lib/config.h"


/* Display related */
#define PIN_DISPLAY_CK 18
#define PIN_DISPLAY_DT 23


enum ft_phase {decode, encode, nothing};
ft_phase phase = decode;

//U8G2 display config
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED

TimeManager tm(ssid,password,ntpServer);

AudioRecorder ar;


TaskHandle_t Task_ar;


void setup() {

  Serial.begin(115200);
  log_i("Total heap.: %d", ESP.getHeapSize());
  log_i("Free heap..: %d", ESP.getFreeHeap());
  log_i("Total PSRAM: %d", ESP.getPsramSize());
  log_i("Free PSRAM.: %d", ESP.getFreePsram());  

  //Display Init
  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Boot...");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display  
  
  
  //String recorded_file = ar.record(3000);
  //ar.play(recorded_file);
      
  //objects init
  if (!tm.align_timer()){
    log_e("initialization failed");
    while (true){
      //stop here
    }
  };
  ar.begin();
}


void tsk_decode(void * parameters) {  

  AudioRecorder::record_t * data = (AudioRecorder::record_t * ) parameters;

  //get parameters
  //TODO: use mutex
  size_t bytes_read =  data->bytes_read;
  int recording_time =  data->recording_time;
  int rate = data->rate;
  uint8_t *psd_pcm_buffer = (uint8_t* )ps_malloc(bytes_read);  
  memcpy(psd_pcm_buffer,data->pcm_buffer,bytes_read);
  free(data->pcm_buffer);

  String file_name=FILE_WAV_PATH+FILE_WAV_PREFIX+millis()+FILE_WAV_SUFFIX;
  char * file_name_c = new char[file_name.length() + 1];
  strcpy(file_name_c,file_name.c_str()); 
 
  //create header
  int headerSize = 44;  
  byte header[headerSize]; 
  CreateWavHeader(header, (recording_time*rate/1000*2)-1);

  //writing file
  File file = SD.open(file_name_c, FILE_WRITE);
  if (!file) {
    log_e("Could not write file");
    vTaskDelete(NULL);
  }    
  file.write(header, headerSize);
  file.write(psd_pcm_buffer, bytes_read);    
  log_d("PCM Bytes: %d", bytes_read);
  file.close();  
  free(psd_pcm_buffer);
  log_i ("recorded: %s", file_name_c);
  log_v("end task"); 
  vTaskDelete(NULL);
}



void loop() {
//https://www.appsloveworld.com/download-sample-wav-file-for-testing/

  TimeManager::clock tmpIntr=tm.getClock();
  if (tmpIntr.raised) {
    if (tmpIntr.number % 15 == 0) {
      //portENTER_CRITICAL(&timerMux);
      //portENTER_CRITICAL(&tm.timerMux);
      tm.resetClock();

      if (phase==decode) {
        
        AudioRecorder::record_t data = ar.record(14000);
        xTaskCreatePinnedToCore(tsk_decode,"Task1",10000,&data,2,NULL,0); 
        delay(100);

      } else if (phase == encode) {
        //ar.play("/sample_16000_16_mono.wav");
      }
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    } 
  }



}  
 