#ifndef FS_H_INCLUDED
#define FS_H_INCLUDED

#include "HDD.h"

// Maximum number of files that can be saved
//  -> 2B per file needed on the header, which is one sector
#define FS_FILES_MAX        HDD_SectorSize / 2	//chose yourself

// Return value if FS is FULL
#define FS_ERR_FULL         FS_FILES_MAX

// Return value if FS detects some bad inputs, file not found etc...
#define FS_ERR_ERROR        FS_FILES_MAX+1

// Inicialization of FS(called once on start)
void FS_Init(void);

// Deletes file on HDD
//
// lintHandler	- file ID
// returns	- if file could be deleted
BOOL FS_DeleteFile(UINT16 lintHandler);

// Get file size
//
// lintHandler	- file ID
// returns	- file len
UINT32 FS_GetFileSize( UINT16 lintHandler );

// Copy file from HDD to alocated memory
//
// lintHandler	- file ID
// lintFilePtr	- ptr on buffer in which u can insert data from HDD
// returns	- if file was correctly copied
BOOL FS_ReadFile( UINT16 lintHandler, UINT8* lintFilePtr);

// Save file from alocated memory to HDD
//
// lintFilePtr	- ptr on data that can be saved on HDD
// lintFileSize	- data len
// returns	- file ID(0-FS_FILES_MAX-1), FS_ERR_FULL when FS is full, FS_ERR_ERROR when some other problem occures
UINT16 FS_SaveFile( UINT8 * lintFilePtr, UINT32 lintFileSize);

#endif // FS_H_INCLUDED
