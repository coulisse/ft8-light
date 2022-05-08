#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/Wav.cpp"
//reference: https://github.com/MhageGH/esp32_SoundRecorder

#include "Wav.hpp"

void CreateWavHeader(byte* header, int waveDataSize){


    //5 - 8: Size of the overall file - 8 bytes, in bytes (32-bit integer). Typically, you’d fill this in after creation.
  unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);


  //41-44: Size of the data section.
  header[40] = (byte)(waveDataSize & 0xFF);
  header[41] = (byte)((waveDataSize >> 8) & 0xFF);
  header[42] = (byte)((waveDataSize >> 16) & 0xFF);
  header[43] = (byte)((waveDataSize >> 24) & 0xFF);


  //Setting wav header standard fixed values
  //1 - 4: 	Marks the file as a riff file. Characters are each 1 byte long.
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';

  //9 -12: File Type Header. For our purposes, it always equals “WAVE”.
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';

  //13-16: 	Format chunk marker. Includes trailing null
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';

  //17-20: Length of format data as listed above
  header[16] = 0x10;  
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;

  //21-22: 	Type of format (1 is PCM) - 2 byte integer
  header[20] = 0x01;  // linear PCM
  header[21] = 0x00;

  //23-24: Number of Channels - 2 byte integer
  header[22] = 0x01;  
  //header[22] = 0x02;  
  header[23] = 0x00;

  //25-28: Sample Rate - 32 byte integer. Common values are 44100 (CD), 48000 (DAT). Sample Rate = Number of Samples per second, or Hertz
  header[24] = 0x11;  // sampling rate 11025
  header[25] = 0x2B;
  header[26] = 0x00;
  header[27] = 0x00;

  //29-32: (Sample Rate * BitsPerSample * Channels) / 8.
  header[28] = 0x22;  
  header[29] = 0x56;
  header[30] = 0x00;
  header[31] = 0x00;
  
  //33-34: (BitsPerSample * Channels) / 8.1 - 8 bit mono2 - 8 bit stereo/16 bit mono4 - 16 bit stereo
  header[32] = 0x02;  // mono
  header[33] = 0x00;

  //35-36: 	Bits per sample
  header[34] = 0x10;  // 16bit
  header[35] = 0x00;

  //37-40: “data” chunk header. Marks the beginning of the data section.
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';  


  
}
