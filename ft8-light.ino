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
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ PIN_DISPLAY_CK, /* data=*/ PIN_DISPLAY_DT, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
Config cfg;

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

    log_i("*********************************************");
    log_i("                START SETUP                  ");
    log_i("*********************************************");
    log_v("Total heap.: %d", ESP.getHeapSize());
    log_i("Free heap..: %d", ESP.getFreeHeap());
    log_v("Total PSRAM: %d", ESP.getPsramSize());
    log_v("Free PSRAM.: %d", ESP.getFreePsram());  

    bool rc=cfg.begin();
    while (!rc) {
        //stop
    }
    
    //Display Init
    u8g2.begin();
    u8g2.clearBuffer();					// clear the internal memory
    u8g2.setFont(u8g2_font_5x7_tf);	    // choose a suitable font
    u8g2.drawStr(0,10,"Boot...");	    // write something to the internal memory
    u8g2.drawStr(0,17,"Boot...");	    // write something to the internal memory
    u8g2.sendBuffer();					// transfer internal memory to the display  

    //Audio recording init
    ar.begin();

    //init main buffer for sharing  audio recording PCM
    data.pcm_buffer = (uint8_t** )ps_malloc(400000*sizeof(uint8_t));

    //creating decoding and recording tasks
    xEventGroup = xEventGroupCreate();
    log_v("init tsk_decode");
    xTaskCreatePinnedToCore(tsk_decode,"decoding  task",40000,&data,1,&xHandleDecode,0);  
    log_v("init tsk_record");
    xTaskCreatePinnedToCore(tsk_record,"recording task",30000,&data,1,&xHandleRecord,1); 
    delay(7000); 

    //init table for transcoded message 
    for (byte i=0;i<MAX_MESSAGES;i++) {
        data.message[i].freq=0;
        data.message[i].score=0;
        strncpy(data.message[i].text,"                        ",25); //TODO: replace 25
        data.message[i].time_sec=0;
        data.message[i].time_slot="000000";
    }  

    //objects timer init
    tm.begin((char *) cfg.wifi_ssid.c_str(),(char *) cfg.wifi_password.c_str(), (char *) cfg.ntp_server.c_str());
    if (!tm.align_timer()){
        log_e("initialization failed");
        while (true){
            //stop here
        }
    }
    log_i("*********************************************");
    log_i("                 END SETUP                   ");
    log_i("*********************************************");
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
      data.time_slot_tmp = tm.get_time_hhmmss();       
      
      if (phase==decode) {
        //TODO: record in a separate task
        log_v("before resume");
        vTaskResume(xHandleRecord);
        log_v("before create task decode");
        for (byte i=0;i<MAX_MESSAGES;i++) {
            log_d("%s %3d %+4.2f %4.0f ~  %s", data.message[i].time_slot, data.message[i].score, data.message[i].time_sec, data.message[i].freq , data.message[i].text);
        }

      } else if (phase == encode) {
        //ar.play("/sample_16000_16_mono.wav");
      }
      //portEXIT_CRITICAL(&tm.timerMux);
      //portEXIT_CRITICAL(&timerMux);
    } 
  }

}  
 