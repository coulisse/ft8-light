/**
 * @file TimeManager.hpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief Header file Time Manager for Ft8 operations 
 */
#ifndef TimeManager_h
#define TimeManager_h

#include <Arduino.h>
#include "driver/adc.h"


class TimeManager  {
      public:  
            //TimeManager (char* id, char* pwd, char* ntpServer, int timer_id=0);
            TimeManager ();
            //~TimeManager();
            void begin(char* id, char* pwd, char* ntpServer, int timer_id=0);
            bool align_timer();
            
            struct clock {
                  int number;
                  bool raised;
            };
            clock getClock();
            static clock intrTimer;
            void resetClock(); 
            String printTime();
            String get_time_hhmmss();
            
      private: 
            int64_t get_time_mills();
            bool setup_ntp();
            int prv_timer_id;
            char* ssid;
            char* password;
            char* ntpServer;
            void disableWiFi();

};
#endif