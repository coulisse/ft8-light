/**
 * @file TimeManager.hpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief Header file Time Manager for Ft8 operations 
 */
#ifndef TimeManager_h
#define TimeManager_h

#include <Arduino.h>


class TimeManager  {
      public:  
            TimeManager (int timer_id);
            //~TimeManager();
            void align_timer();
            
            struct clock {
                  int number;
                  bool raised;
            };
            clock getClock();
            static clock intrTimer;
            void resetClock(); 

      private: 
            int64_t get_time_mills();
            void setup_ntp();
            void printTime();
            int prv_timer_id;
};
#endif