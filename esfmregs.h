#ifndef ESFMREGS_H
#define ESFMREGS_H
/******************************************************************

    esfmregs.h - Structures for ESFM registers and patches file

    ESFM Bank editor

    Copyright (c) 2023, leecher@dose.0wnz.at  All Rights Reserved.

*******************************************************************/
#pragma pack(1)

typedef struct
{
	uint8_t MULT:4;
	uint8_t KSR:1;
	uint8_t EGT:1;
	uint8_t VIB:1;
	uint8_t TRM:1;
} OPREG0;

typedef struct
{
	uint8_t ATTENUATION:6;
	uint8_t KSL:2;
} OPREG1;

typedef struct
{
	uint8_t DECAY:4;
	uint8_t ATTACK:4;
} OPREG2;

typedef struct
{
	uint8_t RELEASE:4;
	uint8_t SUSTAIN:4;
} OPREG3;

typedef struct	/* Fixed pitch */
{
	uint8_t FNUMlo;
} OPREG4_FP;

typedef struct	/* No fixed pitch */
{
	uint8_t CTlo:2; /* Coarse tune */
	uint8_t FINETUNE:6;
} OPREG4_NFP;

typedef union
{
	OPREG4_FP FP;
	OPREG4_NFP NFP;
} OPREG4;

typedef struct	/* Fixed pitch */
{
	uint8_t FNUMhi:2;
	uint8_t BLOCK:3;
	uint8_t DELAY:3;
} OPREG5_FP;

typedef struct	/* Fixed pitch */
{
	uint8_t CThi:5;	/* Coarse tune */
	uint8_t DELAY:3;
} OPREG5_NFP;

typedef union
{
	OPREG5_FP FP;
	OPREG5_NFP NFP;
} OPREG5;

typedef struct
{
	uint8_t unk:1;
	uint8_t MOD:3;
	uint8_t L:1;
	uint8_t R:1;
	uint8_t VIBD:1;
	uint8_t TRMD:1;
} OPREG6;

typedef struct
{
	uint8_t WAVE:3;
	uint8_t NOISE:2;
	uint8_t OU:3;
} OPREG7;

typedef struct
{
	uint8_t PAT16:1;
	uint8_t OP:2;
	uint8_t unk1:1;
	uint8_t FP1:1;
	uint8_t FP2:1;
	uint8_t FP3:1;
	uint8_t FP4:1;
} HDR0;

typedef struct {
	uint8_t RV1:2;
	uint8_t RV2:2;
	uint8_t RV3:2;
	uint8_t RV4:2;
} HDR3;

typedef struct
{
	OPREG0 r0;
	OPREG1 r1;
	OPREG2 r2;
	OPREG3 r3;
	OPREG4 r4;
	OPREG5 r5;
	OPREG6 r6;
	OPREG7 r7;
} OPREG; 

typedef struct
{
	uint8_t h0;
	uint8_t h1;
	uint8_t h2;
	uint8_t h3;

	OPREG o[4];
} PATCH;

typedef struct
{
	PATCH p[2];
} PATCHSET;

typedef struct
{
	uint16_t offs[256];
	PATCHSET patches[256];
} PATCHMEM;

#pragma pack()

#endif