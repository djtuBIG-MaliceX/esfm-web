#ifndef ESFMU_HELPER
#define ESFMU_HELPER

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esfm.h"

// Fucking disgusting hax to make the same ESFMu object visible across all files
esfm_chip* getESFMuObject();

// wAV PLAY
void Render(short *bufpos, uint32_t totalFrames);
void RenderingThread(void *arg);
int InitWaveOut();
int StartWaveOut();
int CloseWaveOut();

#endif