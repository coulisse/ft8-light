/**
 * @file TimeManager.hpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief Header file Time Manager for Ft8 operations 
 */
#ifndef TimeManager_h
#define TimeManager_h

#include <Arduino.h>
#include "driver/adc.h"

/**
 * @brief class used for managing time in FT8
 * 
 */
class TimeManager  {
      public:  
            TimeManager ();
            //~TimeManager();
            /**
             * @brief used to allign timer.
             * Get NTP time
             * Allign second to 00 or 15 or 30 or 45 
             * Allign milliseconds to 000
             * Init internal timer setting clock to 1 second 
             * 
             * @param id WIFI SSID
             * @param pw WIFI password
             * @param ntpSrv ntp server
             * @param timer_id number of the timer (default 0)
             * 
            */            
            void begin(char* id, char* pwd, char* ntpServer, int timer_id=0);
            /**
             * @brief //connecting to wifi and get ntp time 
             * @return false in case of error
             */            
            bool align_timer();
            struct clock {
                  int number;
                  bool raised;
            };
            /**
             * @brief This method is used to pick the number of the seconds of the clock
             * 
             * When this method is called it return a structure and set "raised" to false 
             * in order to use it for check the first time is picked.
             *  
             * @return a structure of type "clock" with the number of second elapsed from 
             * the last clock and raised = true
             */
            clock getClock();
            static clock intrTimer;
            /**
             * @brief reset clock to zero seconds
             */
            void resetClock();
            /**
             * @brief get current time 
             * @return String with current time with format "hh:mm:ss.ms"
             */ 
            String printTime();
            /**
             * @brief get current time 
             * @return String with current time with format "hhmmss"
             */            
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