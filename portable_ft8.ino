#include <Wire.h>
#include <U8g2lib.h>
#include "src/lib/TimeManager.hpp"
#include "src/lib/AudioRecorder.hpp"

/* Display related */
#define PIN_DISPLAY_CK 18
#define PIN_DISPLAY_DT 23


//U8G2 display config
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED

TimeManager tm(0);
AudioRecorder ar;
int  x=0; //todo: remove
void setup() {

  Serial.begin(115200);

  //Display Init
  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Boot...");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display  

  //ar.record(10000);
  //objects init
  
  tm.align_timer();
}

void loop() {


//https://www.appsloveworld.com/download-sample-wav-file-for-testing/
//  delay(1000);  

  TimeManager::clock tmpIntr=tm.getClock();
  if (tmpIntr.raised) {
    if (tmpIntr.number == 15) {
      //portENTER_CRITICAL(&timerMux);
      //portENTER_CRITICAL(&tm.timerMux);
      tm.resetClock();
      u8g2.clearBuffer();					// clear the internal memory
      u8g2.drawStr(0,10,"Recording");
      u8g2.sendBuffer();					// transfer internal memory to the display
      if (x==0) {
        String recorded_file = ar.record(10000);
        x=1;
      } else {
        //ar.play("/sample_16000_16_mono.wav");
        x=0;
      }
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    } 
    if (tmpIntr.number == 10) {
      Serial.println("decoding...");
      u8g2.clearBuffer();					// clear the internal memory
      u8g2.drawStr(0,10,"Decoding");
      u8g2.sendBuffer();					// transfer internal memory to the display
     // ar.play("/prova2.wav");
    }
  }

}  
 