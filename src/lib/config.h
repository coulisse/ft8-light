/**
 #constant definition
 */
//const char* ssid = "hyperline-11635";
#define ssid "hyperline-11635"

//const char* password =  "auj6xai6iN";
#define password "auj6xai6iN"
//const char* ntpServer = "time.google.com";
//const char* ntpServer = "pool.ntp.org";

#define ntpServer "pool.ntp.org"


/* define WRITEWAV if you would record on file .wav */
//#define WRITEWAV

#ifdef WRITEWAV 
const String FILE_WAV_PATH = "/recording";
const String FILE_WAV_PREFIX = "/ft8-light_";
const String FILE_WAV_SUFFIX = ".wav";   
#endif