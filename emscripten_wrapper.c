#include <emscripten.h>
#include <emscripten/webaudio.h>
#include <emscripten/em_math.h>
#include "esfmu_helper.h"
#include "natv.h"


static uint8_t audioThreadStack[4096];
static EMSCRIPTEN_WEBAUDIO_T context;
static bool isAudioStarted = false;

EM_BOOL OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  EMSCRIPTEN_WEBAUDIO_T audioContext = (EMSCRIPTEN_WEBAUDIO_T)userData;
  if (emscripten_audio_context_state(audioContext) != AUDIO_CONTEXT_STATE_RUNNING) {
    emscripten_resume_audio_context_sync(audioContext);
  }
  return EM_FALSE;
}

void GetSample(short *samplem, int len)
{
   Render(samplem, len);
}

EM_BOOL GenerateRender(int numInputs, const AudioSampleFrame *inputs,
                       int numOutputs, AudioSampleFrame *outputs,
                       int numParams, const AudioParamFrame *params,
                       void *userData)
{
   static short buf[256];   // hardcoded 128 samples x numChannels (2)

   Render(buf, 128); // AudioWorklet only renders 128 samples at a time
   float *chLData = &(outputs[0].data[0]);
   float *chRData = &(outputs[0].data[128]);

   for(int i = 0, j = 0; i < 256; ++i)
   {
      float n = (float)buf[i] / 32768.0f;
      if ((i&1) != 0) {
         chLData[j] = n;
      } else {
         chRData[j++] = n;
      }
   }

  return EM_TRUE; // Keep the graph output going
}

void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
{
  if (!success) return; // Check browser console in a debug build for detailed errors

  int outputChannelCounts[1] = { 2 };
  EmscriptenAudioWorkletNodeCreateOptions options = {
    .numberOfInputs = 0,
    .numberOfOutputs = 1,
    .outputChannelCounts = outputChannelCounts
  };

  // Create node
  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext,
                                                            "esfm-generator", &options, &GenerateRender, 0);

  // Connect it to audio context destination
  EM_ASM({emscriptenGetAudioObject($0).connect(emscriptenGetAudioObject($1).destination)},
    wasmAudioWorklet, audioContext);

  // Resume context on mouse click
  emscripten_set_click_callback("body", (void*)audioContext, 0, OnCanvasClick);
}

void AudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
{
   printf("AudioThread Debug init\n");
  if (!success) return; // Check browser console in a debug build for detailed errors
  WebAudioWorkletProcessorCreateOptions opts = {
    .name = "esfm-generator",
  };
  emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, &AudioWorkletProcessorCreated, 0);
}


void Initialize() 
{
   if (!isAudioStarted) {
      InitWaveOut();
      StartWaveOut();
      //static EmscriptenWebAudioCreateAttributes attrs = {"interactive", 49716};
      static EmscriptenWebAudioCreateAttributes attrs = {"playback", 49716};
      context = emscripten_create_audio_context(&attrs);
      emscripten_start_wasm_audio_worklet_thread_async(context, audioThreadStack, sizeof(audioThreadStack),
                                                   &AudioThreadInitialized, 0);
      isAudioStarted = true;
   }
   
   LoadBank("banks/bnk_common.bin");
   fmreset();
}

void Reset()
{
   Initialize();
}

void WriteMidiData(uint32_t dwData)
{
   MidiMessage(dwData);
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