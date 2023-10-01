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
    PATCHMEM m_patches;
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

        //memset(&helper.m_patches, 0, sizeof(PATCHMEM));
    }
    return &helper.esfmu;
}

void Render(short *bufpos, uint32_t totalFrames) {
    //printf("Rendering %u frames...\n", totalFrames);
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

void LoadBank(const char *bankName) {
    FILE *fp = fopen(bankName, "rb");
    fseek(fp, 0, SEEK_END);
    size_t dwSize = ftell(fp);
    uint8_t *lpMem = malloc(dwSize);
    uint16_t *pTbl = (uint16_t*)lpMem;
    PATCHSET *ps = NULL;

    fseek(fp, 0, SEEK_SET);
    fread(lpMem, 1, dwSize, fp);
    fclose(fp);
    fp = NULL;

    printf("Loaded %lu bytes of patch data\n", dwSize);
    
    for (int i=0; i<256; i++)
    {
        if (pTbl[i] && pTbl[i] + sizeof(PATCH)<= dwSize)
        {
            ps = (PATCHSET*)&lpMem[pTbl[i]];
            helper.m_patches.offs[i] = 512 + (i*(uint16_t)sizeof(helper.m_patches.patches[0]));
            helper.m_patches.patches[i].p[0] = ps->p[0];

            printf("Test load patch: %d - %p\n", i, ps);

            if (((HDR0*)&ps->p[0].h0)->OP > 0 && pTbl[i] + (sizeof(PATCH) * 2) <= dwSize)
                helper.m_patches.patches[i].p[1] = ps->p[1];
        } else 
            helper.m_patches.offs[i] = 0;
    }
    free(lpMem);
    printf("DEBUG: Bank load run\n");
}

uint8_t* GetGlobalBankMemory() {
    //printf("DEBUG: Bank obtainium\n");
    return (uint8_t*)&(helper.m_patches);
}