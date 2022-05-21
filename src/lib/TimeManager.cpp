/**
 * @file TimeManager.cpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief Time Manager for Ft8 operations
 */
#include "TimeManager.hpp"
#include <WiFi.h>

// TIMER
hw_timer_t * timer = NULL;
//TimeManager::portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


/**
 * @brief Define the operation to do with timer
 */
TimeManager::clock TimeManager::intrTimer;
void IRAM_ATTR onTimer() {
  //portENTER_CRITICAL_ISR(&timerMux);
  portENTER_CRITICAL_ISR(&timerMux);
  TimeManager::intrTimer.number=TimeManager::intrTimer.number+1;
  TimeManager::intrTimer.raised=true;
  portEXIT_CRITICAL_ISR(&timerMux);
}
//create an alias methodPtrOnTimer for link pointer to method onTimer
void (*methodPtrOnTimer)() = &onTimer;

/**
 * @brief This method is used to pick the number of the seconds of the clock
 * 
 * When this method is called it return a structure and set "raised" to false 
 * in order to use it for check the first time is picked.
 *  
 * @return a structure of type "clock" with the number of second elapsed from 
 * the last clock and raised = true
  */
TimeManager::clock TimeManager::getClock(){
    //TimeManager::clock tmpIntr = TimeManager::intrTimer;
    //TimeManager::intrTimer.raised = false;
    clock tmpIntr = TimeManager::intrTimer;
    intrTimer.raised = false;
    return tmpIntr;
}

/**
 * @brief set clock to a number of second
 */
void TimeManager::resetClock() {
    intrTimer.raised=false;
    intrTimer.number=0;
}

/**
 * @return the number of milliseconds of current time
 */
int64_t TimeManager::get_time_mills() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_usec / 1000LL);
};    


/**
 * @brief class constructor
  */
TimeManager::TimeManager(char* id, char* pwd, char* ntpSrv, int timer_id) {    
    
    intrTimer.number=0;
    intrTimer.raised=true;
    prv_timer_id = timer_id;
    this->ssid = id;
    this->password = pwd;
    this->ntpServer=ntpSrv;
    
 //        timer = NULL;
 //        timerMux = portMUX_INITIALIZER_UNLOCKED;
};

/**
 * @brief used to allign timer.
 * 
 * Get NTP time
 * Allign second to 00 or 15 or 30 or 45 
 * Allign milliseconds to 000
 * Init internal timer setting clock to 1 second 
 * 
*/
bool TimeManager::align_timer() {
    //connecting to wifi and get ntp time

    if (!this->setup_ntp()) {
        log_e("Could not connect to wifi");
        return false;
    }

    Serial.print("Alligning time: ");
    struct tm time;
    if(!getLocalTime(&time)){
        log_e("Could not obtain time info");
        return false;
    }

    //align to seconds
    while (time.tm_sec !=59 && time.tm_sec !=14  && time.tm_sec !=29 && time.tm_sec !=44 ) {
        delay(1000);
        if(!getLocalTime(&time)){
            log_e("Could not obtain time info");            
            return false;
        }        
        Serial.print(time.tm_sec);
        Serial.print(" ");
    }
    this->disableWiFi();
    //align to milliseconds
    while (get_time_mills() != 0) {
    }
    Serial.println();
    //init timer
    timer = timerBegin(prv_timer_id, 80, true);
    timerAttachInterrupt(timer, methodPtrOnTimer, true);        
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);    
};

void TimeManager::disableWiFi(){
    adc_power_off();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

/**
 * @brief used to set up NTP.
 * 
 * Connect to wifi 
 * Get NTP time
 */
bool TimeManager::setup_ntp(){
    byte count = 0;
    const byte max_try = 20;

    WiFi.begin(ssid, password);   
    log_i("Connection wifi to %s network...",ssid);
    while (WiFi.status() != WL_CONNECTED &&  count < max_try) {
        delay(500);
        Serial.print(".");
        count +=1;
    }

    Serial.println();
    if (count < max_try) {
        log_i("Connected wifi with success");  
        configTime(0, 3600, ntpServer);
        Serial.println(printTime());
        return true;        
    } else {
        return false;
    };
};

/**
 * @brief Print current time with format "hh:mm:ss.ms"
 */
String TimeManager::printTime(){
    struct tm time;
    if(!getLocalTime(&time)){
        log_e("Could not obtain time info");
        return "";
    }
    //Serial.printf("time: %u:%u:%u.",time.tm_hour,time.tm_min, time.tm_sec);  
    //Serial.println(get_time_mills());

    char buff[28];
    snprintf(buff, sizeof(buff), "current time: %u:%u:%u",time.tm_hour,time.tm_min, time.tm_sec);
    String buffAsStdStr = buff;
    return buffAsStdStr;
};

String TimeManager::get_time_hhmmss(){
    struct tm time;
    if(!getLocalTime(&time)){
        log_e("Could not obtain time info");
        return "";
    }
    //Serial.printf("time: %u:%u:%u.",time.tm_hour,time.tm_min, time.tm_sec);  
    //Serial.println(get_time_mills());

    char buff[7];
    snprintf(buff, sizeof(buff), "%02u%02u%02u",time.tm_hour,time.tm_min, time.tm_sec);
    String buffAsStdStr = buff;
    return buff;    
}
