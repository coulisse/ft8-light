/**
 * @file ft8-light.ino
 * @author Corrado Gerbaldo - IU1BOW  - corrado.gerbaldo@gmail.com
 * @brief A portable companion for Ham Radio QRP - main module
 * 
 * See license in file LICENSE.md
 */

//TODO: try to compile with a better code optimization
//TODO: optimize speed

/* standard libraries */
#include <Wire.h>
#include <U8g2lib.h>

/* custom libraries  */
#include "src/lib/Config.hpp"
#include "src/lib/TimeManager.hpp"
#include "src/lib/AudioRecorder.hpp"
#include "src/lib/constants.h"
#include "src/lib/decoder.hpp"

/* Global variables and objects */
enum ft_phase {decode, encode, nothing};
ft_phase phase = decode;
//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
<<<<<<< HEAD
//U8G2_SSD1327_WS_128X128_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
U8G2_SSD1327_WS_128X128_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_DISPLAY_CK,PIN_DISPLAY_DT);
//U8G2_SSD1327_WS_128X128_F_2ND_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Config cfg; 
=======
U8G2_SSD1327_WS_128X128_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
//U8X8_SSD1327_WS_128X128_SW_I2C  u8x8(PIN_DISPLAY_CK,PIN_DISPLAY_DT,U8X8_PIN_NONE);
Config cfg;
>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb

TimeManager tm;
AudioRecorder ar;
shared_data data;  //working area used to sharing data between tasks

/* definitions for task management */
TaskHandle_t xHandleRecord;
TaskHandle_t xHandleDecode;
EventGroupHandle_t xEventGroup;


/**
 * @brief Task used to RECORDING audio.
 *  This is an infinite loop: each time it records data to shared data. 
 *  After recording it resume decoding task (using an event BIT) and auto suspend himself. 
 * @param parameters contains the data that will be shared between task by ref 
 * (like PCM binary buffer, or decoded messages)
 * 
 */
void tsk_record(void * parameters) {  

   for (;;) {
        log_v("Recording task resumed"); 
        shared_data* data = (shared_data*) parameters;
        
        ar.record(RECORDING_TIME,(uint8_t*&) data->pcm_buffer,data->rate,data->recording_time,data->bytes_from,data->bytes_read);
        log_v("rate %d, recording time %d, bytes read %d", data->rate,data->recording_time,data->bytes_read);   

        log_v("setting bit for decoding...");
        xEventGroupSetBits(xEventGroup,TSK_DECODE_BIT); //Event,bit
      
        log_v("Suspending recording task..."); 
        vTaskSuspend(NULL);
    }
}

/**
 * @brief Task used to DECODING audio.
 *  This is an infinite loop: it is always active and whenever a specific event BIT is setted at 
 *  at the end of recording task, it start to decode the PCM binary buffer shared in param data 
 * @param parameters contains the data that will be shared between task by ref 
 * (like PCM binary buffer, or decoded messages)
 * 
 */
void tsk_decode(void * parameters) {  

    const EventBits_t xBitsToWaitFor = TSK_DECODE_BIT;
    EventBits_t xEventGroupValue;

    for (;;) {
        xEventGroupValue = xEventGroupWaitBits(xEventGroup,xBitsToWaitFor,pdTRUE,pdTRUE,portMAX_DELAY);//EventGroup,receive, clear on exit, all bits?,wait
        if(xEventGroupValue & TSK_DECODE_BIT != 0) {
            xEventGroupClearBits(xEventGroup,xBitsToWaitFor);        
            //get parameters
            shared_data * data = (shared_data* ) parameters;

            size_t bytes_from = data->bytes_from;
            size_t bytes_read =  data->bytes_read;
            int recording_time =  data->recording_time;
            int rate = data->rate;
            uint8_t *lcl_pcm_buffer = (uint8_t* )ps_malloc(bytes_read);  

            memcpy(lcl_pcm_buffer,&data->pcm_buffer[bytes_from],bytes_read-bytes_from);
            vTaskDelay(1);  // one tick delay (15ms) in between reads for stability 
            
            #ifdef WRITEWAV 
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
            file.write(lcl_pcm_buffer, bytes_read);    
            log_d("PCM Bytes: %d", bytes_read);
            file.close();  
            log_i ("recorded: %s", file_name_c);

            #endif
            decode_ft8(lcl_pcm_buffer,bytes_read,rate, data->message,data->time_slot_tmp); 
            log_v("decoded");
            free(lcl_pcm_buffer);  
            log_v("end task"); 
        }
    }
}


/**
 * @brief Initial setup
 * 
 */
void setup() {

    Serial.begin(115200);
    ar.begin();        
    //u8g2.setBusClock(4000000);
    u8g2.setBusClock(4000000);
    //display init
    u8g2.begin();
    
    /*
    u8g2.clearBuffer();					
    u8g2.setFont(u8g2_font_bubble_tr);
    u8g2.drawStr(0,40,"   FT8");	    // write something to the internal memory
    u8g2.drawStr(0,80,"  Light");	    // write something to the internal memory
    u8g2.drawStr(0,120," IU1BOW");	    // write something to the internal memory
    */

    u8g2.clearBuffer();					
    u8g2.drawXBMP( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, initial_screen_bits);
    u8g2.sendBuffer();    

    delay(3000);

    u8g2.clearDisplay();					
    u8g2.setFont(u8g2_font_5x7_tf);	    
    byte display_y=12;
    const byte display_inc=10;
    byte status_width = u8g2.getStrWidth(DSP_INIT_WAIT)+3;

<<<<<<< HEAD
=======
    //display init
    u8g2.begin();
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_5x7_tf);	    // choose a suitable font
    byte display_y=10;

>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
    log_i("*********************************************");
    log_i("                START SETUP                  ");
    log_i("*********************************************");

<<<<<<< HEAD
    u8g2.drawStr(0,display_y,"START SETUP");	    // write something to the internal memory
    u8g2.sendBuffer();


=======
    u8g2.drawStr(0,display_y,"Start setup");	    // write something to the internal memory
    u8g2.sendBuffer();

>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
    log_v("Total heap.: %d", ESP.getHeapSize());
    log_i("Free heap..: %d", ESP.getFreeHeap());
    log_v("Total PSRAM: %d", ESP.getPsramSize());
    log_i("Free PSRAM.: %d", ESP.getFreePsram());  

<<<<<<< HEAD
    display_y+=display_inc;
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_WAIT);
    u8g2.drawStr(status_width,display_y,"Load config");
    u8g2.sendBuffer();       
//    u8g2.print("Loading configuration");
    bool rc=cfg.begin();
    if (!rc) {
        log_e("configuration failed");
        u8g2.drawStr(0,display_y,DSP_INIT_ERR);
=======
    bool rc=cfg.begin();
    if (!rc) {
        log_e("configuration failed");
        u8g2.drawStr(0,display_y+=7,"ERROR: Configuration failed!!!");
>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
        u8g2.sendBuffer();	        
        while (true) {
            //stop
        }        
    }
<<<<<<< HEAD
    u8g2.drawStr(0,display_y,DSP_INIT_OK);
=======
    
    u8g2.drawStr(0,display_y+=7,"Configuration loaded");
>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
    u8g2.sendBuffer();	

    //Audio recording init
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_WAIT);
    u8g2.drawStr(status_width,display_y,"Audio mngr init");
    u8g2.sendBuffer();	

    //ar.begin();

    u8g2.drawStr(0,display_y,DSP_INIT_OK);
    u8g2.sendBuffer();	

    u8g2.drawStr(0,display_y+=7,"Audio manager initialized");
    u8g2.sendBuffer();	

    //init main buffer for sharing  audio recording PCM
    data.pcm_buffer = (uint8_t** )ps_malloc(400000*sizeof(uint8_t));

<<<<<<< HEAD
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_OK);
    u8g2.drawStr(status_width,display_y,"PSRAM buff alloc");
    u8g2.sendBuffer();	

    //creating decoding task
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_WAIT);
    u8g2.drawStr(status_width,display_y,"Decode task init");
    u8g2.sendBuffer();	    
=======
    u8g2.drawStr(0,display_y+=7,"PSRAM buffer allocated");
    u8g2.drawStr(0,display_y+=7,"Task initializing...");
    u8g2.sendBuffer();	

    //creating decoding and recording tasks
>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
    xEventGroup = xEventGroupCreate();
    log_v("init tsk_decode");
    xTaskCreatePinnedToCore(tsk_decode,"decoding  task",40000,&data,1,&xHandleDecode,0);  
    delay(15000);
    u8g2.drawStr(0,display_y,DSP_INIT_OK);     

    //creating recording tasks    
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_WAIT);
    u8g2.drawStr(status_width,display_y,"Record task init");
    u8g2.sendBuffer();	
    log_v("init tsk_record");
    xTaskCreatePinnedToCore(tsk_record,"recording task",30000,&data,1,&xHandleRecord,1); 
    delay(15000); 
    u8g2.drawStr(0,display_y,DSP_INIT_OK);
    u8g2.sendBuffer();	

    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_WAIT);
    u8g2.drawStr(status_width,display_y,"Clear msg table");
    u8g2.sendBuffer();	

    //init table for transcoded message 
    for (byte i=0;i<MAX_MESSAGES;i++) {
        data.message[i].freq=0;
        data.message[i].score=0;
        strncpy(data.message[i].text,"                        ",25); //TODO: replace 25
        data.message[i].time_sec=0;
        data.message[i].time_slot="    ";
    }  

<<<<<<< HEAD
    u8g2.drawStr(0,display_y,DSP_INIT_OK);
    u8g2.sendBuffer();	

    //objects timer init
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,DSP_INIT_WAIT);
    u8g2.drawStr(status_width,display_y,"Time mngr init");
    u8g2.sendBuffer();    

    Config::properties props;
    props=cfg.get_props();
    tm.begin((char *) props.wifi.ssid.c_str(),(char *) props.wifi.password.c_str(), (char *) props.ntp.server.c_str());
    if (!tm.align_timer()){
        log_e("alligning timer failed");
        u8g2.drawStr(0,display_y,DSP_INIT_ERR);
=======
    u8g2.drawStr(0,display_y+=7,"Message table cleared");
    u8g2.sendBuffer();	

    //objects timer init
    u8g2.drawStr(0,display_y+=7,"Time manager initialization...");
    u8g2.sendBuffer();    
    tm.begin((char *) cfg.wifi_ssid.c_str(),(char *) cfg.wifi_password.c_str(), (char *) cfg.ntp_server.c_str());
    if (!tm.align_timer()){
        log_e("alligning timer failed");
        u8g2.drawStr(0,display_y+=7,"ERROR: alligning timer failed!!!");
>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
        u8g2.sendBuffer();	           
        while (true){
            //stop here
        }
    }
<<<<<<< HEAD
    u8g2.drawStr(0,display_y,DSP_INIT_OK);
    display_y+=display_inc;
    display_y+=display_inc;
    u8g2.drawStr(0,display_y,"END SETUP");
=======

    u8g2.drawStr(0,display_y+=7,"Setup terminated");
>>>>>>> bc7d4d692ccd683162b944d2d27b7a418ae0c2eb
    u8g2.sendBuffer();	    
    log_i("*********************************************");
    log_i("                 END SETUP                   ");
    log_i("*********************************************");

    u8g2.clearDisplay();	
}

/**
 * @brief main loop
 * 
 */
void loop() {

  TimeManager::clock tmpIntr=tm.getClock();
  if (tmpIntr.raised) {
    if (tmpIntr.number % 15 == 0) {
      //portENTER_CRITICAL(&timerMux);
      //portENTER_CRITICAL(&tm.timerMux);
      tm.resetClock();
      //data.time_slot_tmp = tm.get_time_hhmmss();       
      data.time_slot_tmp = tm.get_time_mmss();  

      //ar.play("/recording/test.wav");    
      
      if (phase==decode) {
        //TODO: record in a separate task
        log_v("before resume");
        vTaskResume(xHandleRecord);
        log_v("before create task decode");
        for (byte i=0;i<MAX_MESSAGES;i++) {
            log_d("%s %3d %+4.2f %4.0f ~  %s", data.message[i].time_slot, data.message[i].score, data.message[i].time_sec, data.message[i].freq , data.message[i].text);
        }

        //show decoded on display
        const byte start_element = 40;
        byte display_y=30;
        byte display_x=u8g2.getStrWidth((const char *) data.message[0].time_slot.c_str())+2;
        u8g2.clearBuffer();     
        u8g2.drawStr(0,10, "STATUS STATUS STATUS");
        u8g2.drawHLine(0,20,DSP_MAX_WIDTH);
        
        for (byte i=start_element;i<MAX_MESSAGES;i++) {
            const char * time_slot = (const char *) data.message[i].time_slot.c_str();
            u8g2.drawStr(2,display_y, (const char *) data.message[i].time_slot.c_str());
            u8g2.drawStr(2+display_x,display_y, data.message[i].text);
            display_y+=10;
        }
    /*

       display_y=DSP_MAX_HEIGHT;
        for (byte i=MAX_MESSAGES;i>start_element;i--) {
            const char * time_slot = (const char *) data.message[i].time_slot.c_str();
            u8g2.drawStr(2,display_y, (const char *) data.message[i].time_slot.c_str());
            u8g2.drawStr(2+display_x,display_y, data.message[i].text);
            display_y-=10;
        }
*/
        u8g2.sendBuffer();

      } else if (phase == encode) {
        //ar.play("/sample_16000_16_mono.wav");
      }
      
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    } 
  }

}  
 