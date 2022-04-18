#include <Arduino.h>
#line 1 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
#include <Wire.h>
#include <U8g2lib.h>
#include "src/lib/TimeManager.hpp"
#include "src/lib/AudioRecorder.hpp"

/* Display related */
#define PIN_DISPLAY_CK 18
#define PIN_DISPLAY_DT 23


enum ft_phase {decode, encode, nothing};
ft_phase phase = decode;

//U8G2 display config
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED

TimeManager tm;
AudioRecorder ar;


TaskHandle_t Task_ar;

#line 23 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
void setup();
#line 53 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
void Task1code(void * parameters);
#line 63 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
void loop();
#line 23 "/home/corrado/programming/arduino/ft8-light/ft8-light.ino"
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


void Task1code(void * parameters) {  
  
  log_d("start task");
  ar.record(13200);
  log_d("end task"); 
  vTaskDelete(NULL);
}



void loop() {
//https://www.appsloveworld.com/download-sample-wav-file-for-testing/
//  delay(1000);  
  TimeManager::clock tmpIntr=tm.getClock();
  //Serial.printf("raised: %d, number: %d\n", tmpIntr.raised, tmpIntr.number);
  if (tmpIntr.raised) {
    if (tmpIntr.number % 15 == 0) {
      //portENTER_CRITICAL(&timerMux);
      //portENTER_CRITICAL(&tm.timerMux);
      tm.resetClock();
      /*
      u8g2.clearBuffer();					// clear the internal memory
      u8g2.drawStr(0,10,"Recording");
      u8g2.sendBuffer();					// transfer internal memory to the display
      */
      if (phase==decode) {
        //String recorded_file = ar.record(13200);  
        //uint8_t * pcm_buffer = ar.record(13200);  
        
        xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,1,NULL,0);
        //delay(100);
        //xTaskCreatePinnedToCore(Task2code,"Task2",10000,NULL,2,NULL,0); 

        //TODO: creare copia della ram, altrimenti si sovrascrive?!
                       
         /*
        if (recorded_file == "") {
          log_e("Error writing wav");
        }
        */
      } else if (phase == encode) {
        //ar.play("/sample_16000_16_mono.wav");
      }
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    } 
  }
    /*
    if (tmpIntr.number == 14) {
      Serial.println("decoding...");
      u8g2.clearBuffer();					// clear the internal memory
      u8g2.drawStr(0,10,"Decoding");
      u8g2.sendBuffer();					// transfer internal memory to the display
    }
    */


}  
 
