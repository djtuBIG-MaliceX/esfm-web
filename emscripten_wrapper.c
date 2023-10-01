#include <emscripten.h>
#include "esfmu_helper.h"
#include "natv.h"


void Initialize() 
{
   InitWaveOut();
   StartWaveOut();

   LoadBank("banks/bnk_common.bin");
   fmreset();
}

void Reset()
{
   Initialize();
}

void WriteMidiData(uint32_t dwData)
{
   // if (instance != nullptr)
   // {
   //    //std::cout << "WriteMidiData(" << dwData << ")" << std::endl;
   //    instance->WriteMidiData(dwData);
   // }
   //printf("Debug MIDI Test: %x\n", dwData);
   MidiMessage(dwData);
}

void GetSample(short *samplem, int len)
{
   Render(samplem, len);
}

void GetSampleTwo(short *samplem, int len, float* fsamparrL, float* fsamparrR)
{
   GetSample(samplem, len);
   
   for (int i = 0, j = 0; i < len; )
   {
      fsamparrL[j]   = samplem[i++] / 32768.f;
      fsamparrR[j++] = samplem[i++] / 32768.f;
   }
   
}

void PlaySysex(uint8_t *bufpos, uint32_t len)
{
   // TBI
   printf("%s\n", "Sysex messages not implemented");
}

void CloseInstance()
{
   CloseWaveOut();
}

void ReloadBank(const char *bankName)
{
   LoadBank(bankName);
}