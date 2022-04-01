
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "src/lib/TimeManager.hpp"
#include "src/lib/AudioRecorder.hpp"
//#include "components/cpp_utils/TimeManager.hpp"

//./arduino-1.8.19/libraries/SD/src/utility/Sd2Card.h
/* SD card related */

#define PIN_AUDIO_KIT_SD_CARD_CS 13
#define PIN_AUDIO_KIT_SD_CARD_MISO 2
#define PIN_AUDIO_KIT_SD_CARD_MOSI 15
#define PIN_AUDIO_KIT_SD_CARD_CLK  14


/*
#define PIN_AUDIO_KIT_SD_CARD_CS 5
#define PIN_AUDIO_KIT_SD_CARD_MISO 19
#define PIN_AUDIO_KIT_SD_CARD_MOSI 23
#define PIN_AUDIO_KIT_SD_CARD_CLK  18
*/


//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#define TFT_CS        21
#define TFT_RST       -1 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        22
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED

//https://www.pschatzmann.ch/home/2021/12/06/the-ai-thinker-audio-kit-experience-or-nothing-is-right/
/* switch config
1=off * o on?
2=on 
3=on
4=off
5=off
*/

TimeManager tm(0);
AudioRecorder ar(1);


void setup() {
  Serial.begin(115200);
  SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO, PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);
  u8g2.begin();

  /*
  tft.setFont(NULL);
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.fillScreen(ST77XX_BLACK);

  //tft.setTextColor(color);
  tft.setRotation(1);
  tft.setTextWrap(false);
  int i;
  for (i=0;i<tft.height();i=i+8) {
      tft.setCursor(0, i); 
      tft.print("YXXXXXXXXXXXXXXXXXXXXXXXZ");
  }
*/

  // put your setup code here, to run once:
  if (!SD.begin(PIN_AUDIO_KIT_SD_CARD_CS)) {
    Serial.print("Failes to initialize SD! on pin ");
    Serial.println(PIN_AUDIO_KIT_SD_CARD_CS);
  }
  else {
    Serial.println("SD opened successfuly");
  }

  
  SPI.setClockDivider(SPI_CLOCK_DIV2); // This is becuase feeding SD Card with more than 40 Mhz, leads to unstable operation. 
                                       // (Also depends on SD class) ESP8266 & ESP32 SPI clock with no division is 80 Mhz.


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



  tm.align_timer();
  ar.setupI2S(); //audio driver
}

void loop() {

  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(1000);  

  ar.fetch();
  TimeManager::clock tmpIntr=tm.getClock();
  if (tmpIntr.raised) {
    //Serial.println (tmpIntr.number);
    if (tmpIntr.number == 15) {
    //if (tm.intrTimer.number > 15) {
      //portENTER_CRITICAL(&timerMux);
      //portENTER_CRITICAL(&tm.timerMux);
      tm.resetClock();
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
      Serial.println("TIME TIME 15");
    } 
    if (tmpIntr.number == 10) {
      Serial.println("TIME TIME 10");
    }
  }


}  
 
