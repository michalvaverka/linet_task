#include "FS.h"

#include "HDD_aditional.h"

/* Single file entry within the header -> contains only the ID of the first sector, where it is stored. */
typedef UINT16 FS_FileHeaderData;

/* Since 0 is the index of header sector, it can be used as a termination pointer to the next sector, because no file will ever be stored in the 0 sector. */
#define FS_ID_UNASSIGNED 0

/* Whole Header; it contains only FS_FileHeaderData instances per every possible file, each 
 * This is the bottleneck of the possible amount of files, it is limited by the header being only one sector, 
 *      Therefore there can be only size(sector) / size(FileHeaderData) files => 512.
*/
typedef FS_FileHeaderData FS_Header[FS_FILES_MAX];

/* Sector metadata included in every sector of HDD with the data */
typedef struct {
    UINT16 nextSectorIdx;              /* ID of the next sector where the file continues */
    UINT16 storedDataSize;          /* Size of stored file-data within the sector in bytes */
} FS_FileSectorMetadata;

/* When this is reached as the nextSector, it means it is the last sector */
#define FS_FILE_FINAL_SECTOR 0

/* Metadata instance is written in every sector, which shortens the space for actual data by 4 bytes (size of metadata) */
#define FS_FILE_DATA_PER_CLUSTER (HDD_SectorSize - 4)

/* Whole Cluster of Sector size */
typedef struct {
    FS_FileSectorMetadata metadata;         /* Metadata of the file instance (needed for successful read) */
    UINT8 data[FS_FILE_DATA_PER_CLUSTER];               /* Actual data */
} FS_FileCluster;

// Inicialization of FS(called once on start)
void FS_Init(void)
{
    HDD_Init();     /* Init the HDD, required step */

    UINT32 temp = 0;

    /* Setting the whole header to zeroes, since there is no guarantee it will be zeroed. */
    for (UINT16 i = 0; i < HDD_SectorSize / sizeof(UINT32); i++)
    {
        HDD_WriteHeader(i * sizeof(UINT32), (UINT8 *) &temp, sizeof(UINT32));
    }
}

// Deletes file on HDD
//
// brief        - Cycles through sectors of a specified file and deallocates every single one, in the end updates the header.
//
// lintHandler	- file ID
// returns	    - if file could be deleted
BOOL FS_DeleteFile(UINT16 lintHandler)
{
    BOOL ret = TRUE;

    UINT16 currSectorIdx;

    HDD_ReadHeader(lintHandler * sizeof(FS_FileHeaderData), (UINT8 *) &currSectorIdx, sizeof(FS_FileHeaderData));   /* Reading the first sector ID from header */

    if (currSectorIdx == FS_FILE_FINAL_SECTOR)
    {
        ret = FALSE;
    } else
    {
        FS_FileSectorMetadata fileMetadata; 
        
        HDD_Read(currSectorIdx, 0, (UINT8 *) &fileMetadata, sizeof(FS_FileSectorMetadata)); /* Retrieving the first sector */

        /* Iterating through sectors and deallocating them until a terminating symbol is encountered. */
        while (currSectorIdx != FS_FILE_FINAL_SECTOR)
        {
            HDD_Read(currSectorIdx, 0, (UINT8 *) &fileMetadata, sizeof(FS_FileSectorMetadata));
            HDD_DeAlloc(currSectorIdx);
            currSectorIdx = fileMetadata.nextSectorIdx;
        }
        
        /* Updating the header data since the file will not be stored anymore. */
        currSectorIdx = FS_ID_UNASSIGNED;  

        /* currSectorIdx is the same size as FS_FileHeaderData, only saving local space by not creating another variable. */
        HDD_WriteHeader(lintHandler * sizeof(FS_FileHeaderData), (UINT8 *) &currSectorIdx, sizeof(FS_FileHeaderData)); 
    }

    return ret;
}

// Get file size
//
// brief        - Cycles through file clusters and adds their lengths into a local variable, which is then returned.
//
// lintHandler  - file ID
// returns	    - file len
UINT32 FS_GetFileSize( UINT16 lintHandler )
{
    UINT32 fileSize = 0;

    UINT16 currSectorIdx;

    FS_FileSectorMetadata fileData;

    HDD_ReadHeader(lintHandler * sizeof(FS_FileHeaderData), (UINT8 *) &currSectorIdx, sizeof(UINT16)); /* Reading the first sector ID from header */

    /* Iterate through sectors until the terminating symbol has been found. */
    while (currSectorIdx != FS_FILE_FINAL_SECTOR)
    {
        HDD_Read(currSectorIdx, 0, (UINT8 *) &fileData, sizeof(FS_FileSectorMetadata));

        fileSize += fileData.storedDataSize; /* Add data contained in this cluster */

        currSectorIdx = fileData.nextSectorIdx;
    }

    return fileSize;
}

// Copy file from HDD to alocated memory
//
// brief        - Fetches first sector of file from the HDD header, then cycles through them until it encounters a terminating symbol.
//                  When this ID has no starting sector, returns FALSE, because then the file doesn't exist.
//
// lintHandler	- file ID
// lintFilePtr	- ptr on buffer in which u can insert data from HDD
// returns	    - if file was correctly copied
BOOL FS_ReadFile( UINT16 lintHandler, UINT8* lintFilePtr)
{
    BOOL ret = TRUE;

    FS_FileHeaderData fileData;

    HDD_ReadHeader(lintHandler * sizeof(FS_FileHeaderData), (UINT8 *) &fileData, sizeof(FS_FileHeaderData));    /* Reading the first sector ID from header */

    /* In case such file has no first sector in header, return false. */
    if (fileData == FS_FILE_FINAL_SECTOR)
    {
        ret = FALSE;
    } else 
    {
        UINT16 currSectorIdx = fileData;
        UINT16 nextSectorIdx = 0;

        FS_FileSectorMetadata fileMetaData; 
        UINT16 sectorCount = 0;

        /* Iterating until terminating symbol is found. */
        while (currSectorIdx != FS_FILE_FINAL_SECTOR)
        {
            HDD_Read(currSectorIdx, 0, (UINT8 *) &fileMetaData, sizeof(FS_FileSectorMetadata));
            
            nextSectorIdx = fileMetaData.nextSectorIdx;

            /* Read into pointer from HDD. */
            if (fileMetaData.storedDataSize == FS_FILE_DATA_PER_CLUSTER)
            {
                HDD_Read(currSectorIdx, sizeof(FS_FileSectorMetadata), &lintFilePtr[sectorCount * FS_FILE_DATA_PER_CLUSTER], FS_FILE_DATA_PER_CLUSTER);
            } else
            {
                /* The last sector will be smaller -> read with a value of the size. */
                HDD_Read(currSectorIdx, sizeof(FS_FileSectorMetadata), &lintFilePtr[sectorCount * FS_FILE_DATA_PER_CLUSTER], fileMetaData.storedDataSize);    
            }

            currSectorIdx = nextSectorIdx;
            sectorCount++;
        } 
    }

    return ret;
}

// Find first free file ID in the HDD header.
//
//
// brief        Cycles through header, until it finds a new ID (index) for a file.
//                  Intended only for internal use by the FS module.        
//
// returns      new fileId on success, FS_ERR_FULL on failure
UINT16 FS_FindFreeFileId()
{
    UINT16 fileId = FS_ERR_FULL;

    FS_FileHeaderData pointedSectorIdx;            /* Placeholder for current file data from the header */

    /* Looking for first free ID in the header */
    for (UINT16 i = 0; i < FS_FILES_MAX; i++)
    {
        HDD_ReadHeader(i * sizeof(FS_FileHeaderData), (UINT8 *) &pointedSectorIdx, sizeof(FS_FileHeaderData));

        if (pointedSectorIdx == FS_ID_UNASSIGNED)
        {
            fileId = i;
            break;
        }
    }

    return fileId;
}

// Save file from alocated memory to HDD
//
// brief        - After a free ID has been found, the input file is split into clusters consisting of 1020B of file data and 4B of metadata and written on the HDD
//                  If any sector cannot be allocated, the operation fails and FS_DeleteFile is called to clean the already allocated sectors.
//                  After successful run, returns a new file ID.
//
// lintFilePtr	- ptr on data that can be saved on HDD
// lintFileSize	- data len
// returns	- file ID(0-FS_FILES_MAX-1), FS_ERR_FULL when FS is full, FS_ERR_ERROR when some other problem occures
UINT16 FS_SaveFile( UINT8 * lintFilePtr, UINT32 lintFileSize)
{
    UINT16 fileId = FS_FindFreeFileId();

    /* In case no free fileId can be found, the FS is full, therefore no sense to continue. */
    if (fileId != FS_ERR_FULL && fileId != FS_ERR_ERROR)
    {
        UINT16 currSectorIdx = HDD_Alloc();   /* Allocating the first sector, whose ID will be written in the header */

        /* In case first sector cannot be allocated, the HDD is full, therefore return. */
        if (currSectorIdx == HDD_FULL)
        {
            fileId = FS_ERR_FULL;
        } else
        {
            HDD_WriteHeader(fileId * sizeof(FS_FileHeaderData), (UINT8 *) &currSectorIdx, sizeof(FS_FileHeaderData));  /* Update the header with first file sector pointer */

            UINT32 wholeClusterCount = lintFileSize / FS_FILE_DATA_PER_CLUSTER;     /* Number of clusters, which will be completely filled by the data */
            UINT32 lastClusterSize = lintFileSize - (wholeClusterCount * FS_FILE_DATA_PER_CLUSTER);    /* Size of the last cluster, which in general won't be completely filled (even though it can occur). */

            UINT16 nextSectorIdx = 0;     /* Sector, in which the next cluster will be located, has to be saved in the previous cluster. */
            BOOL writeFailed = FALSE;        /* Is needed to recognize if some sector couldn't be allocated and therefore start the dealocation sequence. */

            FS_FileSectorMetadata currentFileData;

            /* Iterating through all needed clusters, allocating sectors and filling them with respective data.*/
            for (UINT32 i = 0; i < wholeClusterCount; i++)
            {
                nextSectorIdx = HDD_Alloc();

                /* In case sector cannot be allocated, the HDD is full and deallocation sequence must be started. */
                if (nextSectorIdx == HDD_FULL)
                {
                    writeFailed = TRUE;

                    currentFileData.storedDataSize = 0;
                    currentFileData.nextSectorIdx = FS_FILE_FINAL_SECTOR;

                    HDD_Write(currSectorIdx, 0, (UINT8 *) &currentFileData, sizeof(FS_FileSectorMetadata));   /* Update the data, that current sector is the last. */
                
                    break;
                }

                /* In case the file has all clusters full (its size is n*1020B ), the next sector pointer must be set to terminating symbol. */
                if (i == wholeClusterCount - 1 && lastClusterSize == 0)
                {
                    currentFileData.nextSectorIdx = FS_FILE_FINAL_SECTOR;
                    HDD_DeAlloc(nextSectorIdx);
                } else
                {
                    currentFileData.nextSectorIdx = nextSectorIdx;
                }

                currentFileData.storedDataSize = FS_FILE_DATA_PER_CLUSTER;

                /* Passing the data to the HDD. */
                HDD_Write(currSectorIdx, 0, (UINT8 *) &currentFileData, sizeof(FS_FileSectorMetadata));   /* Writing current file metadata onto the HDD */
                HDD_Write(currSectorIdx, sizeof(FS_FileSectorMetadata), &lintFilePtr[i * FS_FILE_DATA_PER_CLUSTER], HDD_SectorSize - sizeof(FS_FileSectorMetadata));     /* Writing file contents onto the HDD */

                currSectorIdx = nextSectorIdx;  /* Moving onto the next sector */
            }

            if (writeFailed)
            {
                FS_DeleteFile(fileId); /* Deallocation sequence in case the file couldn't be written completely. */
            } else if (lastClusterSize > 0) /* Most files will have a last cluster, which is not completely full, those are resolved here. */
            {
                currentFileData.nextSectorIdx = FS_FILE_FINAL_SECTOR;    /* Terminating symbol, since this truly is the last cluster */
                currentFileData.storedDataSize = lastClusterSize;

                HDD_Write(currSectorIdx, 0, (UINT8 *) &currentFileData, sizeof(FS_FileSectorMetadata));
                HDD_Write(currSectorIdx, sizeof(FS_FileSectorMetadata), &lintFilePtr[wholeClusterCount * FS_FILE_DATA_PER_CLUSTER], lastClusterSize);
            }
        }
    }

    return fileId;
}
