#ifndef NATV_H
#define NATV_H

#define LOBYTE(w)           ((uint8_t)(((uint32_t)(w)) & 0xff))
#define HIBYTE(w)           ((uint8_t)((((uint32_t)(w)) >> 8) & 0xff))

void  fmreset();

uint16_t NATV_CalcBend(uint16_t detune, uint16_t iBend, uint16_t iBendRange);
uint8_t NATV_CalcVolume(uint8_t reg1, uint8_t bVelocity, uint8_t bChannel);
void NATV_CalcNewVolume(uint8_t bChannel);

void note_on(uint8_t bChannel, uint8_t bNote, uint8_t bVelocity);
void note_off(uint8_t bChannel, uint8_t bNote);

void voice_on(int voiceNr);
void voice_off(int voiceNr);

void hold_controller(uint8_t bChannel, uint8_t bVelocity);
void find_voice(bool patch1617_allowed_voice1, bool patch1617_allowed_voice2, uint8_t bChannel, uint8_t bNote);
void setup_voice(int voicenr, int offset, int bChannel, int bNote, int bVelocity);
int  steal_voice(int patch1617_allowed);

void MidiAllNotesOff(void);
uint32_t  MidiCalcFAndB (uint32_t wPitch, uint8_t bBlock);
void  MidiPitchBend (uint8_t  bChannel, uint16_t iBend);
void  MidiMessage (uint32_t dwData);

void  fmreset();

#endif