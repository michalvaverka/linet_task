#ifndef HDD_H_INCLUDED
#define HDD_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// size of single sector in bytes, maximum count of sectors is unknown
#define HDD_SectorSize      1024

// value which indicates full hdd during HDD_Alloc
#define HDD_FULL            65535		// some number will be inserted by testing framework


typedef uint8_t     BOOL;
#define             TRUE        1
#define             FALSE       0
typedef uint8_t     UINT8;
typedef int8_t      INT8;
typedef uint16_t    UINT16;
typedef int16_t     INT16;
typedef uint32_t    UINT32;
typedef int32_t     INT32;

// inits HDD
void HDD_Init(void);

// Write header, Copy header data to hdd
//
// lintHddOffset    - on which byte of sector start to write
// lintData	        - ptr on data
// lintDataLen	    - data len(1-1024)
void HDD_WriteHeader(UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen);

// Read header, Copy header data from hdd
//
// lintHddOffset    - on which byte of sector start to read
// lintData	        - ptr on data
// lintDataLen	    - data len(1-1024)
void HDD_ReadHeader(UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen);

// Alocates 1 sector for data
//
// returns 	        - sectorID (1-x), if HDD is full returns HDD_FULL
UINT16 HDD_Alloc( void );

// Dealocates 1 sector for data
//
// lintSector 	    - sectorID
void HDD_DeAlloc(UINT16 lintSector);

// Copy data from pointer to HDD
//
// lintSector 	    - sectorID
// lintHddOffset	- on which byte of sector start to write
// lintData	        - ptr on data to write
// lintDataLen	    - data len(1-1024)
void HDD_Write(UINT16 lintSector,UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen);

// Copy data form HDD to pointer
//
// lintSector 	    - sectorID
// lintHddOffset	- on which byte of sector start to read
// lintData	        - ptr on data to read
// lintDataLen	    - data len(1-1024)
void HDD_Read(UINT16 lintSector,UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen);

#endif // HDD_H_INCLUDED
