/******************************************************************

    natv.c - native Soundcard functions


    Copyright (c) 2023, 
        leecher@dose.0wnz.at  All Rights Reserved.
        dj.tuBIG/MaliceX, hax my anus

*******************************************************************/

#include "driver.h"
#include "natv.h"
#include "esfmu_helper.h"
#include "esfmregs.h"

/* --- typedefs ----------------------------------------------- */

/* MIDI */
// hax my anus
#define gBankMem GetGlobalBankMemory()
#define fmwrite(a,b) ESFM_write_reg_buffered_fast(getESFMuObject(), a,b)

/* transformation of linear velocity value to
        logarithmic attenuation */
uint8_t gbVelocityAtten[32] = {
        40, 36, 32, 28, 23, 21, 19, 17,
        15, 14, 13, 12, 11, 10, 9, 8,
        7, 6, 5, 5, 4, 4, 3, 3,
        2, 2, 1, 1, 1, 0, 0, 0 };

uint8_t pmask_MidiPitchBend[4] = {
        0x10, 0x20, 0x40, 0x80 };

uint8_t    pan_mask[NUMCHANNELS];
uint8_t    gbVelLevel[NUMCHANNELS];

/* channel volumes */
uint8_t    gbChanAtten[NUMCHANNELS];       /* attenuation of each channel, in .75 db steps */
short   giBend[NUMCHANNELS];    /* bend for each channel */

uint8_t    hold_table[NUMCHANNELS];
uint8_t    gbChanVolume[NUMCHANNELS];
uint8_t    program_table[NUMCHANNELS];
uint8_t    gbChanExpr[NUMCHANNELS];
uint8_t    gbChanBendRange[NUMCHANNELS];
uint8_t    note_offs[NUMCHANNELS];
voiceStruct voice_table[NUM2VOICES];
uint16_t  gwTimer;
uint32_t   voice1, voice2;

uint16_t NATV_table1[64] = {
    1024, 1025, 1026, 1027, 1028, 1029, 1030, 1030, 1031, 1032,
    1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042,
    1043, 1044, 1045, 1045, 1046, 1047, 1048, 1049, 1050, 1051,
    1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061,
    1062, 1063, 1064, 1065, 1065, 1066, 1067, 1068, 1069, 1070,
    1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080,
    1081, 1082, 1083, 1084,
};
uint16_t NATV_table2[49] = {
    256,  271,  287,  304,  323,  342,  362,  384,  406,  431,
    456,  483,  512,  542,  575,  609,  645,  683,  724,  767,
    813,  861,  912,  967,  1024, 1085, 1149, 1218, 1290, 1367,
    1448, 1534, 1625, 1722, 1825, 1933, 2048, 2170, 2299, 2435,
    2580, 2734, 2896, 3069, 3251, 3444, 3649, 3866, 4096,
};

int td_adjust_setup_operator[12] = {
    256, 242, 228, 215, 203, 192,
    181, 171, 161, 152, 144, 136
};

uint16_t fnum[12] = {
    514, 544, 577, 611,  /* G , G#, A , A# */
    647, 686, 727, 770,  /* B , C , C#, D  */
    816, 864, 916, 970   /* D#, E , F,  F# */
};


/**************************************************************
MidiAllNotesOff - switch off all active voices.

inputs - none
returns - none
*/
void MidiAllNotesOff(void)
{
    uint8_t i;

    for (i = 0; i < NUM2VOICES; i++) {
        note_off (voice_table[i].bChannel, voice_table[i].bNote);
    }
}

/*****************************************************************
MidiCalcFAndB - Calculates the FNumber and Block given
        a frequency.

inputs
        uint32_t   dwPitch - pitch
returns
        uint32_t - High byte contains the 0xb0 section of the
                        block and fNum, and the low byte contains the
                        0xa0 section of the fNumber
*/
uint32_t  MidiCalcFAndB (uint32_t wPitch, uint8_t bBlock)
{
    // D1(("MidiCalcFAndB"));
    /* bBlock is like an exponential to dwPitch (or FNumber) */
    for (; wPitch >= 0x400; wPitch >>= 1, bBlock++)
        ;

    if (bBlock > 0x07)
        bBlock = 0x07;  /* we cant do anything about this */

    /* put in high two bits of F-num into bBlock */
    return ((uint32_t) bBlock << 10) | (uint32_t) wPitch;
}


//------------------------------------------------------------------------
//  void MidiPitchBend
//
//  Description:
//     This pitch bends a channel.
//
//  Parameters:
//     uint8_t bChannel
//        channel
//
//     short iBend
//        values from -32768 to 32767, being -2 to +2 half steps
//
//  Return Value:
//     Nothing.
//
//
//------------------------------------------------------------------------

void  MidiPitchBend
(
    uint8_t            bChannel,
    uint16_t          iBend
)
{
   uint16_t i, j;
   uint16_t bnd;

   // D1( "\nMidiPitchBend" ) ;

   // Remember the current bend..

   giBend[ bChannel ] = iBend ;

   // Loop through all the notes looking for the right
   // channel.  Anything with the right channel gets its
   // pitch bent...

   for (i = 0; i < NUM2VOICES; i++)
      if (voice_table[ i ].bChannel == bChannel && (voice_table[ i ].flags1 & 1))
      {
         for (j = 0 ; j < OPS_PER_CHAN; j++ )
         {
             if ((pmask_MidiPitchBend[j] & voice_table[ i ].bPatch)) continue;
             bnd = NATV_CalcBend( voice_table[ i ].detune[ j ], iBend, gbChanBendRange[bChannel] ) ;
             bnd = MidiCalcFAndB( bnd, (uint8_t)((voice_table[ i ].reg5[ j ] >> 2) & 7)) ;
             fmwrite( (uint32_t)(32 * i + 8 * j + 5), (uint8_t)(HIBYTE(bnd) | (voice_table[ i ].reg5[ j ] & 0xE0)) );
             fmwrite( (uint32_t)(32 * i + 8 * j + 4), (uint8_t)(bnd & 0xFF));
         }
      }

} // end of MidiPitchBend()



/* --- midi interpretation -------------------------------------*/


/***************************************************************
MidiMessage - This handles a MIDI message. This
        does not do running status.

inputs
        uint32_t dwData - up to 4 bytes of MIDI data
                depending upon the message.
returns
        none
*/
void  MidiMessage (uint32_t dwData)
{
    uint8_t    bChannel, data2, data1;
    int     i;

    // D1("\nMidiMessage");
    bChannel = (uint8_t) dwData & (uint8_t)0x0f;
    data2 = (uint8_t) (dwData >> 16) & (uint8_t)0x7f;
    data1 = (uint8_t) ((uint32_t) dwData >> 8) & (uint8_t)0x7f;

    //printf("Debug MIDI: ch %d, data2 %d, data1 %d\n", bChannel, data2, data1);

    switch ((uint8_t)dwData & 0xf0) {
        case 0x90:
#ifdef DEBUG
            {
                    uint8_t szTemp[4], szDebug[ 80 ];
                    uint8_t ;
                    szTemp[0] = "0123456789abcdef"[data1 >> 4];
                    szTemp[1] = "0123456789abcdef"[data1 & 0x0f];
                    szTemp[2] = ' ';
                    szTemp[3] = 0;
                    if ((bChannel == 9) && data2) D1(szTemp);
                    
                    wsprintf( szDebug, "bChannel = %d, data1 = %d",
                            bChannel, data1 ) ;
                    D1( szDebug ) ;
            }
#endif

                /* turn key on, or key off if volume == 0 */
                if (data2)
                {
                    note_on(bChannel, data1, data2);
                    break;
                }

                /* else, continue through and turn key off */
        case 0x80:
                /* turn key off */
                note_off( bChannel, data1 );
                break;

        case 0xb0:
                // D1("\nChangeControl");
                /* change control */
                switch (data1) {
                        case 6:
                                if ( (hold_table[bChannel] & 6) == 6 )
                                    gbChanBendRange[bChannel] = data2;
                                break;
                        case 7:
                                gbChanAtten[bChannel] = gbVelocityAtten[data2 >> 1];
                                gbChanVolume[bChannel] = data2;
                                NATV_CalcNewVolume(bChannel);
                                break;
                        case 8:
                        case 10:
                                /* change the pan level */
                                if ( data2 <= 80 )
                                {
                                    if ( data2 >= 48 )
                                        pan_mask[bChannel] = 0x30;
                                    else
                                        pan_mask[bChannel] = 0x10;
                                }
                                else
                                {
                                    pan_mask[bChannel] = 0x20;
                                }
                                break;
                        case 11:
                                /* change expression */
                                gbChanExpr[bChannel] = data2;
                                NATV_CalcNewVolume(bChannel);
                                break;
                        case 64:
                                /* sustain */
                                hold_controller(bChannel, data2);
                                break;
                        case 100:
                                if ( data2 == 0 )
                                {
                                    hold_table[bChannel] |= 2;
                                    break;
                                }
                        case 98:
                                hold_table[bChannel] &= ~2;
                                break;
                        case 101:
                                if ( data2 == 0 )
                                {
                                    hold_table[bChannel] |= 4;
                                    break;
                                }
                        case 99:
                                hold_table[bChannel] &= ~4;
                                break;
                        case 120:
                        case 124:
                        case 125:
                                for (i = 0; i < NUM2VOICES; i++)
                                {
                                    if ((voice_table[i].flags1 & 1) && voice_table[i].bChannel == bChannel)
                                        voice_off(i);
                                }
                                break;
                        case 121:
                                /* reset all controllers */
                                if (hold_table[bChannel] & 1)
                                {
                                    for (i = 0; i < NUM2VOICES; i++)
                                    {
                                        if ((voice_table[i].flags1 & 1) && voice_table[i].bChannel == bChannel && (voice_table[i].flags1 & 4))
                                            voice_off(i);
                                    }
                                }
                                hold_table[bChannel] &= ~1u;
                                gbChanVolume[bChannel] = 100;
                                gbChanExpr[bChannel] = 127;
                                giBend[bChannel] = 0x2000;
                                pan_mask[bChannel] = 0x30;
                                gbChanBendRange[bChannel] = 2;
                                break;
                        case 123:
                        case 126:
                        case 127:
                                /* All notes off */
                                for (i = 0; i < NUM2VOICES; i++)
                                {
                                    if ((voice_table[i].flags1 & 1) && voice_table[i].bChannel == bChannel && (voice_table[i].flags1 & 4) == 0)
                                        voice_off(i);
                                }
                                break;
                        };
                break;

        case 0xc0:
                program_table[bChannel] = data1;
                break;

        case 0xe0:
                // D1("\nBend");
                /* pitch bend */
                MidiPitchBend (bChannel, (uint16_t)data1 | ((uint16_t)data2 << 7));

                break;
    };

    return;
}



/*
 * fmreset - silence the board and set all voices off.
 */
void  fmreset() 
{
	uint32_t i;

    for (i=0; i<16; i++) 
    {
        giBend[i]           = 0x2000;
        gbChanBendRange[i]  = 0x02;
        hold_table[i]       = 0x00;
        gbChanExpr[i]       = 0x7F;
        gbChanVolume[i]     = 0x64;
        gbChanAtten[i]      = 0x04;
        pan_mask[i]         = 0x30;
    }
    
    for (i=0; i < 18; i++) 
    {
        voice_table[i].wTime = 0;
        voice_table[i].flags1 = 0;
    }
    
    gwTimer = 0;
}

uint16_t NATV_CalcBend(uint16_t detune, uint16_t iBend, uint16_t iBendRange)
{
    //!WARN iBend is int16 in OPL midi driver sample
    if ( iBend == 0x2000 ) 
        return detune;
    else 
	{
        int v5;
        if ( iBend >= 0x3F80 ) iBend = 0x4000;
        v5 = ((iBendRange * (((int)iBend - 0x2000))) >> 5) + 0x1800;
        return (detune * (uint16_t)((NATV_table1[(v5>>2)&0x3F] * NATV_table2[v5>>8]) >> 10) + 512) >> 10;
    }
}

uint8_t NATV_CalcVolume(uint8_t reg1, uint8_t bVelocity, uint8_t bChannel)
{
    uint8_t vol;

    if ( !gbChanVolume[bChannel] ) return 63;

    switch ( bVelocity )
    {
    case 0:
    default:
        vol = 0;
        break;
    case 1:
        vol = ((127 - gbChanExpr[bChannel]) >> 4 ) + ((127 - gbChanVolume[bChannel]) >> 4);
        break;
    case 2:
        vol = ((127 - gbChanExpr[bChannel]) >> 3) + ((127 - gbChanVolume[bChannel]) >> 3);
        break;
    case 3:
        vol = gbChanVolume[bChannel];
        if ( vol < 64 )
            vol = ((63 - vol) >> 1) + 16;
        else
            vol = (127 - vol) >> 2;
        if ( gbChanExpr[bChannel] < 64 )
        {
            vol += ((63 - gbChanExpr[bChannel]) >> 1) + 16;
        }
        else
        {
            vol += ((127 - gbChanExpr[bChannel]) >> 2);
        }
        break;
    }
    vol += (reg1 & 0x3F);          // ATTENUATION
    if ( vol > 63 ) vol = 63;
    return vol | reg1 & 0xC0;      // KSL
}

void NATV_CalcNewVolume(uint8_t bChannel)
{
    uint32_t i, j;

    for (i=0; i < NUM2VOICES; i++)
    {
        voiceStruct *voice = &voice_table[i];
        if ((voice->flags1 & 1) && (voice->bChannel == bChannel || bChannel == (uint8_t)0xFF)) 
        {
            for (j=0; j < OPS_PER_CHAN; j++)
				fmwrite(i * 32 + j * 8 + 1, NATV_CalcVolume(voice->reg1[j], (voice->bVelocity >> (j*2)) & 3, voice->bChannel));
        }
    }   
}


void note_on(uint8_t bChannel, uint8_t bNote, uint8_t bVelocity)
{
    int patch;
    int offset;
    uint8_t flags_voice1;
    int fixed_pitch;

    if ( bChannel == 9 )
        patch = bNote + 128;
    else
        patch = program_table[bChannel];
    offset = gBankMem[2 * patch] + ((int)gBankMem[2 * patch + 1] << 8);
    //printf("Debug: note on patch offset %x\n", offset);
    if ( offset )
    {
        flags_voice1 = gBankMem[offset];
        fixed_pitch = (flags_voice1 >> 1) & 3;
        switch (fixed_pitch)
        {
        case 0:
            find_voice(flags_voice1 & 1, 0, bChannel, bNote);
            if ( voice1 == 255 ) voice1 = steal_voice(gBankMem[offset] & 1);
            setup_voice(voice1, offset, bChannel, bNote, bVelocity);
            voice_on(voice1);
            break;
        case 1:
            find_voice(flags_voice1 & 1, gBankMem[offset + 36] & 1, bChannel, bNote);
            if (voice1 == 255) voice1 = steal_voice(gBankMem[offset] & 1);
            setup_voice(voice1, offset, bChannel, bNote, bVelocity);
            if (voice2 != 255)
            {
                setup_voice(voice2, offset + 36, bChannel, bNote, bVelocity);
                voice_table[voice2].flags1 |= 8u;
                voice_on(voice2);
            }
            voice_on(voice1);
            break;
        case 2:
            find_voice(flags_voice1 & 1, gBankMem[offset + 36] & 1, bChannel, bNote);
            if ( voice1 == 255 )
            voice1 = steal_voice(gBankMem[offset] & 1);
            if ( voice2 == 255 )
            voice2 = steal_voice(gBankMem[offset + 36] & 1);
            setup_voice(voice1, offset, bChannel, bNote, bVelocity);
            setup_voice(voice2, offset + 36, bChannel, bNote, bVelocity);
            voice_on(voice1);
            voice_on(voice2);
            break;
        }
        gbVelLevel[bChannel] = bVelocity;
        if (note_offs[bChannel] == (uint8_t)255) note_offs[bChannel]=0; else note_offs[bChannel]++;
    }
}

void note_off(uint8_t bChannel, uint8_t bNote)
{
    int i;
    
    for (i=0; i<18; i++)
    {
		voiceStruct *voice = &voice_table[i];
        if ((voice->flags1 & 1) && voice->bChannel == bChannel && voice->bNote == bNote)
        {
            if ( hold_table[bChannel] & 1 ) 
            {
                voice->flags1 |= 4;
            }
            else
            {
                voice_off(i);
            }
        }
    }
}

void hold_controller(uint8_t bChannel, uint8_t bVelocity)
{
    if ( bVelocity < 64 ) 
    {
        int i;
        
        hold_table[bChannel] &= ~1;
        
        for (i = 0; i< NUM2VOICES; i++)
        {
            if ((voice_table[i].flags1 & 4) && voice_table[i].bChannel == bChannel)
                voice_off(i);
        }
    } else {
        hold_table[bChannel] |= 1;
    }
}

void voice_on(int voiceNr)
{
    if ( voiceNr >= 16 )
    {
        if ( voiceNr == 16 )
        {
            fmwrite(0x250, 1);
            fmwrite(0x251, 1);
        }
        else
        {
            fmwrite(0x252, 1);
            fmwrite(0x253, 1);
        }
    }
    else 
    {
        fmwrite((uint16_t)voiceNr + 0x240, 1);
    }
}

void voice_off(int voiceNr)
{
    if ( voiceNr >= 16 )
    {
        if ( voiceNr == 16 )
        {
          fmwrite(0x250, 0);
          fmwrite(0x251, 0);
        }
        else
        {
          fmwrite(0x252, 0);
          fmwrite(0x253, 0);
        }
    }
    else
    {
        fmwrite((uint16_t)voiceNr + 0x240, 0);
    }
    voice_table[voiceNr].flags1 = 2;
    voice_table[voiceNr].wTime = (uint16_t)gwTimer;
    gwTimer++;
}

void find_voice(bool patch1617_allowed_voice1, bool patch1617_allowed_voice2, uint8_t bChannel, uint8_t bNote)
{
    int i;
    uint16_t td, timediff1=0, timediff2=0;
    
    voice1 = voice2 = 255;

    // Patch 0-15
    for (i=0; i<16; i++)
    {
        voiceStruct *voice = &voice_table[i];
        if (voice->flags1 & 1)
        {
            if (voice->bChannel == bChannel && voice->bNote == bNote)
                voice_off(i);
        }
        else
        {
            td = gwTimer - voice->wTime;
            if (td < timediff1)
            {
                if (td >= timediff2)
                {
                    timediff2 = td;
                    voice2 = i;
                }
            }
            else
            {
                timediff2 = timediff1;
                voice2 = voice1;
                timediff1 = td;
                voice1 = i;
            }
        }
    }
    
    // Patch 16
    if (voice_table[16].flags1 & 1)
    {
        if (voice_table[16].bChannel == bChannel && voice_table[16].bNote == bNote)
            voice_off(16);
    }
    else
    {
        td = gwTimer - voice_table[16].wTime;
        if (patch1617_allowed_voice1 || td < timediff1)
        {
            if (!patch1617_allowed_voice2 && td >= timediff2)
            {
                timediff2 = gwTimer - voice_table[16].wTime;
                voice2 = 16;
            }
        }
        else
        {
            timediff2 = timediff1;
            voice2 = voice1;
            timediff1 = td;
            voice1 = 16;
        }
    }

    // Patch 17
    if (voice_table[17].flags1 & 1)
    {
        if (voice_table[17].bChannel == bChannel && voice_table[17].bNote == bNote)
            voice_off(17);
    }
    else
    {
        td = gwTimer - voice_table[17].wTime;
        if (patch1617_allowed_voice1 || td < timediff1)
        {
            if (!patch1617_allowed_voice2 && td >= timediff2)
                voice2 = 17;
        }
        else
        {
            if (voice1 != 16 || !patch1617_allowed_voice2)
                voice2 = voice1;
            voice1 = 17;
        }
    }
}

int steal_voice(int patch1617_allowed)
{
    uint32_t i, last_voice=0, max_voices = (patch1617_allowed?18:16);
    uint8_t chn, chncmp = 0, bit3 = 0;
	uint16_t timediff = 0;
    
    for (i=0; i<max_voices; i++)
    {
        chn = voice_table[i].bChannel == 9?1:voice_table[i].bChannel+2;
        if (bit3 == (voice_table[i].flags1 & 8))
        {
            if (chn <= chncmp)
            {
                if (chn != chncmp || (gwTimer - voice_table[i].wTime) <= timediff)
                    continue;
            }
            else
            {
                chncmp = chn;
            }
        }
        else if (!bit3)
        {
            bit3 = 8;
            chncmp = chn;
        }
        else continue;
        
        timediff = gwTimer - voice_table[i].wTime;
        last_voice = i;
    }
    voice_off(last_voice);
    return last_voice;
}

void  setup_operator(
        int offset,
        int bNote,
        int bVelocity,
        uint16_t reg,
        int fixed_pitch,
        int rel_velocity,
        int bChannel,
        int oper,
        int voicenr)
{
    int note, transpose, block, notemod12, reg1, detune;
    uint16_t fnum_block;
    uint8_t reg4, reg5, reg6, panmask;
    
    panmask = pan_mask[bChannel];
    fmwrite(reg + 7, 0);
    
	note = bNote;
    if (!fixed_pitch)
    {
        transpose = ((((gBankMem[offset + 5]) << 2) & 0x7F) | (gBankMem[offset + 4] & 3));
        if (gBankMem[offset + 5] & 0x10) // signed?
            transpose |= ~0x7F;
        note += transpose;
    }
    
    if ( note < 19 )
        note += 12 * (((18 - note) / 12u) + 1);
    if ( note > 114 )
        note -= 12 * (((note - 115) / 12u) + 1);
    block = (note - 19) / 12;
    notemod12 = (note - 19) % 12;
    
    fmwrite(reg + 0, gBankMem[offset]);
    
    switch ( rel_velocity )
    {
    case 0:
    default:
        reg1 = 0;
        break;
    case 1:
        reg1 = (127 - bVelocity) >> 4;
        break;
    case 2:
        reg1 = (127 - bVelocity) >> 3;
        break;
    case 3:
        if ( bVelocity < 64 )
            reg1 = ((63 - bVelocity) >> 1) + 16;
        else
            reg1 = (127 - bVelocity) >> 2;
        break;
    }
    reg1 += (gBankMem[offset + 1] & 0x3F); // Attenuation
    if (reg1 > 63) reg1 = 63;
    reg1 += (gBankMem[offset + 1] & 0xC0); // KSL
    voice_table[voicenr].reg1[oper] = (uint8_t)reg1;
    
    fmwrite(reg + 1, NATV_CalcVolume((uint8_t)reg1, (uint8_t)rel_velocity, (uint8_t)bChannel));
    fmwrite(reg + 2, gBankMem[offset + 2]);
    fmwrite(reg + 3, gBankMem[offset + 3]);
    
    if ( fixed_pitch )
    {
        reg4 = gBankMem[offset + 4];
        reg5 = gBankMem[offset + 5];
    }
    else
    {
        detune = ((int)*((uint8_t*)&gBankMem[offset + 4])) & (~3);
        if (detune)
        {
            detune = ((detune >> 2) * td_adjust_setup_operator[notemod12]) >> 8;
            if (block > 1)
                detune >>= block - 1;
        }
        detune += fnum[notemod12];
        voice_table[voicenr].reg5[oper] = (uint8_t)((HIBYTE(detune) & 3) | (gBankMem[offset + 5] & 0xE0) | (block << 2)); // detune | delay | block
        fnum_block = MidiCalcFAndB(NATV_CalcBend((uint16_t)detune, giBend[bChannel], (uint16_t)gbChanBendRange[bChannel]), (uint8_t)block);
        reg4 = LOBYTE(fnum_block);
        reg5 = HIBYTE(fnum_block) | (voice_table[voicenr].reg5[oper] & 0xE0);
        voice_table[voicenr].detune[oper] = (uint16_t)detune;
    }
    reg6 = gBankMem[offset + 6];
    if ((reg6 & 0x30) && panmask != 0x30) reg6 = panmask | (reg6 & 0xCF);
    fmwrite(reg + 4, reg4);
    fmwrite(reg + 5, reg5);
    fmwrite(reg + 6, reg6);
    fmwrite(reg + 7, gBankMem[offset + 7]);
}

void  setup_voice(int voicenr, int offset, int bChannel, int bNote, int bVelocity)
{
    uint8_t rel_vel, bPatch;
    
    bPatch = gBankMem[offset];
    rel_vel = gBankMem[offset + 3];
    offset += 4;
    setup_operator(offset     , bNote, bVelocity, 32 * (uint16_t)voicenr + 0 , bPatch & 0x10, (rel_vel >> 0) & 3, bChannel, 0, voicenr);
    setup_operator(offset + 8 , bNote, bVelocity, 32 * (uint16_t)voicenr + 8 , bPatch & 0x20, (rel_vel >> 2) & 3, bChannel, 1, voicenr);
    setup_operator(offset + 16, bNote, bVelocity, 32 * (uint16_t)voicenr + 16, bPatch & 0x40, (rel_vel >> 4) & 3, bChannel, 2, voicenr);
    setup_operator(offset + 24, bNote, bVelocity, 32 * (uint16_t)voicenr + 24, bPatch & 0x80, (rel_vel >> 6) & 3, bChannel, 3, voicenr);

    voice_table[voicenr].bPatch = bPatch;
    voice_table[voicenr].bVelocity = rel_vel;
    voice_table[voicenr].wTime = gwTimer;
    voice_table[voicenr].bNote = (uint8_t)bNote;
    voice_table[voicenr].flags1 = 1;
    voice_table[voicenr].bChannel = (uint8_t)bChannel;
    
    gwTimer++;
}

