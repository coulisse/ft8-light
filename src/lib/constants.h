/**
 * @file constants.h
 * @author Corrado Gerbaldo - iu1bow (corrado.gerbaldo@gmail.com)
 * @brief  constants definitions
  */

#define RECORDING_TIME 9500

/* define WRITEWAV if you would record on file .wav */
//#define WRITEWAV
#ifdef WRITEWAV 
const String FILE_WAV_PATH = "/recording";
const String FILE_WAV_PREFIX = "/ft8-light_";
const String FILE_WAV_SUFFIX = ".wav";   
#endif

/* task constant for bit check*/
#define TSK_DECODE_BIT (1UL<<0UL) //Unsigned long bit0 set to 1

/* Display related pin */
#define PIN_DISPLAY_CK 18
//#define PIN_DISPLAY_CK 13
#define PIN_DISPLAY_DT 23

/* Display strings */
#define DSP_INIT_WAIT "[ .. ]"
#define DSP_INIT_OK   "[ OK ]"
#define DSP_INIT_ERR  "[FAIL]"


#define DSP_MAX_WIDTH 128
#define DSP_MAX_HEIGHT 128


