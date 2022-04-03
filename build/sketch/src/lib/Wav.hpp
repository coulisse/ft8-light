#line 1 "/home/corrado/programming/arduino/portable_ft8/src/lib/Wav.hpp"
//reference: https://github.com/MhageGH/esp32_SoundRecorder

#include <Arduino.h>

// 16bit, monoral, 44100Hz,  linear PCM
void CreateWavHeader(byte* header, int waveDataSize);  // size of header is 44
