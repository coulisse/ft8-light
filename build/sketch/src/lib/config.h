#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/config.h"
/**
 #constant definition
 */
//const char* ssid = "hyperline-11635";
#define ssid "hyperline-11635"

#define password "auj6xai6iN"
//const char* ntpServer = "time.google.com";
//const char* ntpServer = "pool.ntp.org";

#define ntpServer "pool.ntp.org"

#define RECORDING_TIME 12000
#define DECODING_TIME  5000

/* define WRITEWAV if you would record on file .wav */
//#define WRITEWAV

#ifdef WRITEWAV 
const String FILE_WAV_PATH = "/recording";
const String FILE_WAV_PREFIX = "/ft8-light_";
const String FILE_WAV_SUFFIX = ".wav";   
#endif

