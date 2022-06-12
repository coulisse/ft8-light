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
        bool begin();  
        struct properties {
            //wifi structure
            struct wifi {
                String ssid;
                String password;
            } wifi;
            //ntp structure
            struct ntp {
                //ntp server
                String server;
            } ntp;
            struct radio {
                //callsign
                String callsign;
            } radio;            
        };
        
        properties get_props();

    private:   
        enum ini_section {wifi, time, radio};      
        void parse_line(String line, Config::ini_section* section);  
        properties props;        
        
};

#endif