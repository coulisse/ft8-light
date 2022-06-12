/**
 * @file Config.cpp
 * @author Corrado Gerbaldo - IU1BOW
 * @brief configuration for APP based on SSD config file
 */
#include "Config.hpp"

Config::Config() {   

}

/**
 * @brief parse line read from config file and put data in object properties
 * @param line the record reads from file
 * @param section (by ref) the current ini section 
 */
void Config::parse_line(String line, Config::ini_section* section) {
    int len;
    line.trim();
    len = line.length();

    if (len>0 and line.charAt(0)!=';') {  //skip void and commented lines

        if (line.charAt(0)=='[') {
            //section found
            if (line.equals("[wifi]")) {
                *section = wifi;
            };

            if (line.equals("[time]")) { 
                *section = time;
            }
            if (line.equals("[radio]")) { 
                *section = time;
            }            
        } else {
            //property found
            int equal_pos=line.indexOf('=');
            if (equal_pos>1 && equal_pos<len) {
                String property;
                String value;
                property=line.substring(0,equal_pos);
                value=line.substring(equal_pos+1,len);

                if (*section==wifi) {
                    if (property.equals("ssid")) {
                        this->props.wifi.ssid=value;
                    }
                    if (property.equals("password")) {
                        this->props.wifi.password=value;
                    }
                }

                if (*section==time) {              
                    if (property.equals("ntpServer")) {
                        this->props.ntp.server=value;
                    }
                }

                if (*section==radio) {              
                    if (property.equals("callsign")) {
                        String call=value;
                        call.toUpperCase();
                        this->props.radio.callsign=call;
                    }
                }

            }

        }

    }    

}

/**
 * @brief read the config.ini and put values in object properties
 */
bool Config::begin() {

    bool rc_ok=true;
    String line;
    ini_section section;    
    
    File file = SD.open(CONFIG_FILE, FILE_READ);
    if (!file) {
        log_e("Could not read config file");
        rc_ok=false;
    }     

    while (file.available()) {
        line = file.readStringUntil('\n');
        this->parse_line(line,&section);
    }
    
    file.close(); //close file
    return rc_ok;
}

Config::properties Config::get_props() {
    return this->props;
}


