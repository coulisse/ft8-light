/*
 * @file Config.hpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief configuration for APP based on SSD config file
 */
#ifndef config_h
#define config_h

#include <SD.h>
#define CONFIG_FILE "/config.ini"

class Config {
    public:
        Config ();
        bool begin();  
        String wifi_ssid;
        String wifi_password;
        String ntp_server;      


    private:   
        enum ini_section {wifi, time};      
        void parse_line(String line, Config::ini_section* section);      
        

};

#endif