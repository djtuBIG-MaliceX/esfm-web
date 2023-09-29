#ifndef DRIVER_H
#define DRIVER_H
/****************************************************************************
 *
 *   driver.h
 *
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "synth.h"




#define SR_ALERT_NORESOURCE     11
#define DATA_FMPATCHES          1234

#ifndef RC_INVOKED
#define RT_BINARY               MAKEINTRESOURCE( 256 )
#else
#define RT_BINARY               256
#endif



//
// Porting stuff
//

#define BCODE

#define fEnabled true

#define AsULMUL(a, b) ((uint32_t)((uint32_t)(a) * (uint32_t)(b)))
#define AsLSHL(a, b) ((uint32_t)((uint32_t)(a) << (uint32_t)(b)))
#define AsULSHR(a, b) ((uint32_t)((uint32_t)(a) >> (uint32_t)(b)))

#define AsMemCopy CopyMemory

extern void* MidiDeviceHandle;
extern SYNTH_DATA DeviceData[];
extern int MidiPosition;
extern void MidiFlush(void);
extern void MidiCloseDevice(void* DeviceHandle);
extern uint32_t MidiOpenDevice(void* lpHandle, bool Write);
extern uint32_t MidiSetVolume(uint32_t Left, uint32_t Right);
extern void MidiCheckVolume(void);
extern uint32_t MidiGetVolume(uint32_t lpVolume);

#define SYNTH_DATA_SIZE 80

#define NUM2VOICES              (18)            /* # 2operator voices */
#define OPS_PER_CHAN			(4)				/* Operators per channel */

extern void  MidiSendFM (uint32_t wAddress, uint8_t bValue);
extern void  MidiNewVolume (uint32_t wLeft, uint32_t wRight);
extern uint32_t  MidiInit (void);

extern uint8_t gbVelocityAtten[32];

//
// End of porting stuff
//

/*
 * midi device type - determined by kernel driver
 */
extern uint32_t gMidiType;
/*
 * values for gMidiType - set in MidiOpenDevice
 */
#define TYPE_ADLIB	1
#define TYPE_OPL3	2


/*
 *  String IDs
 *  NOTE - these are shared with the drivers and should be made COMMON
 *  definitions
 */

#define SR_ALERT                1
#define SR_ALERT_NOPATCH        10

#define SYSEX_ERROR     0xFF    // internal error code for sysexes on input

#define STRINGLEN               (100)

/* volume defines */
#define VOL_MIDI                (0)
#define VOL_NUMVOL              (1)

#define VOL_LEFT                (0)
#define VOL_RIGHT               (1)

/* MIDI defines */

#define NUMCHANNELS                     (16)
#define NUMPATCHES                      (256)
#define DRUMCHANNEL                     (9)     /* midi channel 10 */

/****************************************************************************

       typedefs

 ***************************************************************************/


// per allocation structure for midi
typedef struct portalloc_tag {
    uint32_t               dwCallback;     // client's callback
    uint32_t               dwInstance;     // client's instance data
    void*            hMidi;          // handle for stream
    uint32_t               dwFlags;        // allocation flags
}PORTALLOC;

typedef struct _voiceStruct {
        uint8_t    flags1;
        uint16_t  wTime;                  /* time that was turned on/off;
                                           0 time indicates that its not in use */
        uint8_t    bChannel;               /* channel played on */
        uint8_t    bNote;                  /* note played */
        uint8_t    bVelocity;              /* velocity */
        uint16_t  detune[OPS_PER_CHAN];   /* original pitch, for pitch bend */
        uint8_t    bPatch;                 /* what patch is the note,
                                           drums patch = drum note + 128 */
        uint8_t    reg5[OPS_PER_CHAN];		/* Delay, block */
        uint8_t    reg1[OPS_PER_CHAN];		/* KSL and Attenuation*/
} voiceStruct;



/****************************************************************************

       strings

 ***************************************************************************/

#define INI_STR_PATCHLIB TEXT("Patches")
#define INI_SOUND        TEXT("synth.ini")
#define INI_DRIVER       TEXT("Driver")


/****************************************************************************

       globals

 ***************************************************************************/

/* midi.c */
extern uint8_t	gbMidiInUse;		/* if MIDI is in use */

extern void*  hDriverModule1;           // our module handle

/***************************************************************************

    prototypes

***************************************************************************/

/* midi.c */
void  MidiMessage (uint32_t dwData);
uint32_t modSynthMessage(uint32_t id, uint32_t msg, uint32_t dwUser, uint32_t dwParam1, uint32_t dwParam2);
uint32_t MidiOpen (void);
void MidiClose (void);
void MidiReset(void);





#endif