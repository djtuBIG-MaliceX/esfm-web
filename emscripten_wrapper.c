#include <emscripten.h>
#include "esfmu_helper.h"
#include "natv.h"


void Initialize() 
{
   InitWaveOut();
   StartWaveOut();

   LoadBank();
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
   /*
   for (int i = 0, j=0; i < len; i+=2, j++)
   {
      fsamparrL[j] = samplem[i] / 32768.f;
      //fsamparrL[j] = (fsamparrL[j]<-1)?-1:(fsamparrL[j]>1)?1:fsamparrL[j];
   }
      
   
   for (int i = 1, j=0; i < len; i+=2, j++)
   {
      fsamparrR[j] = samplem[i] / 32768.f;
      //fsamparrR[j] = (fsamparrR[j]<-1)?-1:(fsamparrR[j]>1)?1:fsamparrR[j];
      
      //EM_ASM_({
      //   var xxx = 0;
      //   if ($2 % 1000 == 0)
      //   document.getElementById('bufNum').innerHTML += $0 + ' => ' + $1 + '<br />';
      //}, samplem[i], fsamparrR[j], j);
      
   }
   */ 
   
   
   for (int i = 0, j = 0; i < len; )
   {
      fsamparrL[j]   = samplem[i++] / 32768.f;
      fsamparrR[j++] = samplem[i++] / 32768.f;
      //EM_ASM_({
      //   document.getElementById('bufNum').innerHTML = $0 + '<br />';
      //}, fsamparr[i]);
   }
   
}

void PlaySysex(uint8_t *bufpos, uint32_t len)
{
   //std::cout << "PlaySysex( len = " << len << ")" << std::endl;
   //instance->PlaySysex(bufpos, len);

   // TBI
}

void CloseInstance()
{
   //std::cout << "CloseInstance()" << std::endl;
   //instance->close();
   CloseWaveOut();
}
