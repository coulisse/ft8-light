/*
 * @file Config.hpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief configuration for APP based on SSD config file
 */
#ifndef config_h
#define config_h

#include <SD.h>
#define CONFIG_FILE "/config.ini"

/**
 * @brief parse line read from config file and put data in object properties
 */
class Config {
    public:
        Config ();
        /**
        * @brief read the config.ini and put values in object properties
        */
        bool begin();  
        //WIFI SSID
        String wifi_ssid;
        //WIFI password
        String wifi_password;
        //NTP server
        String ntp_server;      

    private:   
        enum ini_section {wifi, time};      
        void parse_line(String line, Config::ini_section* section);      
        
};

#endif