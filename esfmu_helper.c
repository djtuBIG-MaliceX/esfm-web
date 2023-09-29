#include "esfmu_helper.h"

typedef struct eht {
    uint32_t	chunks;
    uint32_t framesRendered;
    unsigned int sampleRate;
    unsigned int bufferSize;
    unsigned int chunkSize;
    bool	stopProcessing;
    esfm_chip esfmu;
    bool esfm_initialized;
    int16_t buffer[16384];
} esfmu_helper_t;

static esfmu_helper_t helper;

esfm_chip* getESFMuObject() {
    // static esfm_chip *esfmu = NULL;
    // if (esfmu == NULL) {
    //     esfmu = (esfm_chip*)malloc(sizeof(esfm_chip));
    //     ESFM_init(esfmu);
    // }
    // return esfmu;
    if (!helper.esfm_initialized) {
        ESFM_init(&helper.esfmu);

        // Init ESFM native mode
        ESFM_write_port(&helper.esfmu, 2, 0x05);
        ESFM_write_port(&helper.esfmu, 3, 0x80);
        helper.esfm_initialized = true;
    }
    return &helper.esfmu;
}

void Render(short *bufpos, uint32_t totalFrames) {
	ESFM_generate_stream(getESFMuObject(), bufpos, totalFrames);
}

int16_t* GetWaveBuffer() {
    // static int16_t buffer[8820];
    return helper.buffer;
}

int InitWaveOut()
{
    memset(&helper, 0, sizeof(esfmu_helper_t));
    helper.framesRendered = 0;
    helper.sampleRate = 49716; //44100;
    helper.bufferSize = (49716 * 100 / 1000.f);
    helper.chunkSize = (49716 * 10 / 1000.f);
	helper.stopProcessing = false;
	return 0;
}

int StartWaveOut()
{
	//Render(GetWaveBuffer(), bufferSize);
	helper.framesRendered = 0;
    return 0;
}

int CloseWaveOut() {
	helper.stopProcessing = true;
	return 0;
}