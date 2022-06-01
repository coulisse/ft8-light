/**
 * @file Wav.hpp
 * @author reference: https://github.com/MhageGH/esp32_SoundRecorder
 * @brief used to generate a wav header
 */

#include <Arduino.h>

void CreateWavHeader(byte* header, int waveDataSize);  // size of header is 44
