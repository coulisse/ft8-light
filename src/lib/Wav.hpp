//reference: https://github.com/MhageGH/esp32_SoundRecorder
//reference: https://forum.arduino.cc/t/creating-a-wav-file-header/314260/4

#include <Arduino.h>

void CreateWavHeader(byte* header, int waveDataSize);  // size of header is 44