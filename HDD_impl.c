#include "HDD.h"
#include "HDD_aditional.h"

#include <stdbool.h>

//#define TEST_ALLOC_FAULT
//#define TEST_FIRST_SECT_FAULT

/* this times 1024B / 1KiB */
#define HDD_SectorDefaultCount HDD_FULL  /* now 1 GiB -> that is a pretty testing size */ 

/* A single HDD sector representation */
typedef struct {
    bool is_alloced;        /* if is currently used by some file */
    char * data;            /* actual data written in the sector */
} HDD_sector;

HDD_sector * HDD = NULL;    /* global instance of HDD -> simpler for testing */
int sector_count = 0;       /* perhaps redundant variable */

// inits HDD
void HDD_Init(void)
{
    HDD = (HDD_sector *) calloc(sizeof(HDD_sector), HDD_SectorDefaultCount);

    if (HDD == NULL)
    {
        return;
    }

    for (int i = 0; i < HDD_SectorDefaultCount; i++)
    {
        if (i == 0)
        {
            HDD[i].is_alloced = true;
        } else
        {
            HDD[i].is_alloced = false;
        }

        HDD[i].data = (char *) calloc(HDD_SectorSize, 1);

        if (HDD[i].data == NULL)
        {
            sector_count = i;
            return;
        }
    }

    sector_count = HDD_SectorDefaultCount;
    
    printf("Success.\n");
    return;
}


void HDD_deinit(void)
{
    for (int i = 0; i < sector_count; i++)
    {
        free(HDD[i].data);
    }

    free(HDD);
}
/* Sector metadata included in every sector of HDD with the data */
typedef struct {
    UINT16 nextSector;              /* ID of the next sector where the file continues */
                                        // 0 means termination -> define as a macro
    UINT16 storedDataSize;          /* Size of stored file-data within the sector in bytes */
} FS_FileSectorMetadata;

/* When this is reached as the nextSector, it means it is the last sector */
#define FS_FILE_FINAL_SECTOR 0

/* Metadata instance is written in every sector, which shortens the space for actual data by 4 bytes (size of metadata) */
#define FS_FILE_DATA_PER_CLUSTER (HDD_SectorSize - 4)

typedef struct {
    FS_FileSectorMetadata metadata; /* Metadata of the file instance (needed for successful read) */
    UINT8 data[FS_FILE_DATA_PER_CLUSTER];               /* Actual data */
} FS_FileCluster;

void HDD_ListUsedSectors(void)
{
    char buffer[1021];
    buffer[1020] = '\0';

    FS_FileCluster cluster;

    printf("HDD - START LIST SECTORS\n");
    for (int i = 1; i < sector_count; i++)
    {
        if (HDD[i].is_alloced)
        {
            memcpy(&cluster, HDD[i].data, 1024);
            printf("%d with data:\n", i);
            printf("\t\tNext sector:\t%d\n", cluster.metadata.nextSector);
            printf("\t\tSize:\t%d\n", cluster.metadata.storedDataSize);
            memcpy(buffer, cluster.data, cluster.metadata.storedDataSize);
            buffer[cluster.metadata.storedDataSize] = '\0';
            printf("\t\tData:\t%s\n", buffer);
        }
    }
    printf("HDD - END LIST SECTORS\n");
}


void HDD_randomize_header(char * source, int len)
{
    int a = 0;
    for (int i = 0; i < HDD_SectorSize; i++)
    {
        if (a == len - 1)
            a = 0;
        
        HDD[0].data[i] = source[a];
        
        a++;
    }
}

BOOL anysectorused(void)
{
    BOOL ret = FALSE;

    for (int i = 0; i < sector_count; i++)
    {
        if (HDD[i].is_alloced) {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

// Write header, Copy header data to hdd
//
// lintHddOffset    - on which byte of sector start to write
// lintData	        - ptr on data
// lintDataLen	    - data len(1-1024)
void HDD_WriteHeader(UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen)
{
    if (lintDataLen > HDD_SectorSize)
    {
        printf("WriteHeader Fault - Incorrect data size %d\n", lintDataLen);
        return;
    } else if (lintData == NULL)
    {
        printf("WriteHeader Fault - NULL data\n");    
        return;
    } else if (lintHddOffset > HDD_SectorSize)
    {
        printf("WriteHeader Fault - Incorrect offset %d\n", lintHddOffset);
        return;
    }

    memcpy(&HDD[0].data[lintHddOffset], lintData, lintDataLen);
}

// Read header, Copy header data from hdd
//
// lintHddOffset    - on which byte of sector start to read
// lintData	        - ptr on data
// lintDataLen	    - data len(1-1024)
void HDD_ReadHeader(UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen)
{
    if (lintDataLen > HDD_SectorSize)
    {
        printf("ReadHeader Fault - Incorrect data size %d\n", lintDataLen);
        return;
    } else if (lintData == NULL)
    {
        printf("ReadHeader Fault - NULL data\n");    
        return;
    } else if (lintHddOffset > HDD_SectorSize)
    {
        printf("ReadHeader Fault - Incorrect offset %d\n", lintHddOffset);
        return;
    }

    memcpy(lintData, &HDD[0].data[lintHddOffset], lintDataLen);
}

// Alocates 1 sector for data
//
// returns 	        - sectorID (1-x), if HDD is full returns HDD_FULL
UINT16 HDD_Alloc( void )
{
    UINT16 ret = HDD_FULL;

    for (UINT16 i = 1; i < sector_count; i++)
    {
        if (HDD[i].is_alloced == false)
        {
            HDD[i].is_alloced = true;
            ret = i;
            break;
        }
    }

    #ifdef TEST_ALLOC_FAULT

    if (ret == 99)
    {
        HDD[ret].is_alloced = false;
        ret = HDD_FULL;
    }

    #endif

    #ifdef TEST_FIRST_SECT_FAULT

    if (ret == 1)
    {
        HDD[1].is_alloced = false;
        ret = HDD_FULL;
    }

    #endif

    printf("Alloc of sector %d successful.\n", ret);

    return ret;
}

// Dealocates 1 sector for data
//
// lintSector 	    - sectorID
void HDD_DeAlloc(UINT16 lintSector)
{
    if (lintSector >= HDD_FULL || lintSector == 0)
    {
        printf("DeAlloc Fault - Invalid sector ID.\n");
        return;
    }

    HDD[lintSector].is_alloced = false;
    memset(HDD[lintSector].data, 0, HDD_SectorSize);

    printf("Dealloced %d\n", lintSector);
}

// Copy data from pointer to HDD
//
// lintSector 	    - sectorID
// lintHddOffset	- on which byte of sector start to write
// lintData	        - ptr on data to write
// lintDataLen	    - data len(1-1024)
void HDD_Write(UINT16 lintSector,UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen)
{
    if (lintDataLen > HDD_SectorSize)
    {
        printf("Write Fault - Incorrect data size %d\n", lintDataLen);
        return;
    } else if (lintData == NULL)
    {
        printf("Write Fault - NULL data\n");    
        return;
    } else if (lintHddOffset > HDD_SectorSize)
    {
        printf("Write Fault - Incorrect offset %d\n", lintHddOffset);
        return;
    } else if (lintSector == 0 || lintSector >= HDD_FULL)
    {
        printf("Write Fault - Incorrect sector id %d\n", lintSector);
        return;    
    }

    printf("Write successful.\n");

    memcpy(&HDD[lintSector].data[lintHddOffset], lintData, lintDataLen);
}

// Copy data form HDD to pointer
//
// lintSector 	    - sectorID
// lintHddOffset	- on which byte of sector start to read
// lintData	        - ptr on data to read
// lintDataLen	    - data len(1-1024)
void HDD_Read(UINT16 lintSector,UINT32 lintHddOffset, UINT8 * lintData, UINT32 lintDataLen)
{
    if (lintDataLen > HDD_SectorSize)
    {
        printf("Read Fault - Incorrect data size %d\n", lintDataLen);
        return;
    } else if (lintData == NULL)
    {
        printf("Read Fault - NULL data\n");    
        return;
    } else if (lintHddOffset > HDD_SectorSize)
    {
        printf("Read Fault - Incorrect offset %d\n", lintHddOffset);
        return;
    } else if (lintSector == 0 || lintSector >= HDD_FULL)
    {
        printf("Read Fault - Incorrect sector id %d\n", lintSector);
        return;    
    }

    memcpy(lintData, &HDD[lintSector].data[lintHddOffset], lintDataLen);
}
