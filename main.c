#include "HDD.h"
#include "HDD_aditional.h"

#include "FS.h"

#include <stdbool.h>
#include <assert.h>

//Vytestuj z toho shit
void surface_test_hdd(void)
{
    UINT8 buffer[1024];

    /* Write header surfacetest */
    memcpy(buffer, "AHOJTEKAMARADI", 15);

    HDD_WriteHeader(0, buffer, 15);

    /* Read header surfacetest */
    UINT8 sec_buff[1024];

    HDD_ReadHeader(0, sec_buff, 15);

    //printf("%s\n", sec_buff);
        
    /* Alloc test */
    UINT16 sec_1 = HDD_Alloc();
    UINT16 sec_2 = HDD_Alloc();
    UINT16 sec_3 = HDD_Alloc();

    /* Write test */
    HDD_Write(sec_1, 0, sec_buff, 15);
    HDD_Write(sec_2, 0, sec_buff, 15);
    HDD_Write(sec_3, 0, sec_buff, 15);

    /* Read test */
    UINT8 thir_buff[15];

    memset(thir_buff, 0, 15);
    HDD_Read(sec_1, 0, thir_buff, 15);
    HDD_DeAlloc(sec_1);
    // printf("%s\n", thir_buff);
        
    memset(thir_buff, 0, 15);
    HDD_Read(sec_2, 0, thir_buff, 15);
    HDD_DeAlloc(sec_2);
    // printf("%s\n", thir_buff);

    memset(thir_buff, 0, 15);
    HDD_Read(sec_3, 0, thir_buff, 15);
    HDD_DeAlloc(sec_3);
    // printf("%s\n", thir_buff);

    /* DeAlloc test */
    HDD_DeAlloc(sec_1);
    HDD_DeAlloc(sec_2);
    HDD_DeAlloc(sec_3);
}

void test_hdd_randomization()
{
    HDD_randomize_header("alenene", 8);
    
    UINT8 buff[HDD_SectorSize + 1];

    HDD_ReadHeader(0, buff, HDD_SectorSize);

    buff[1024] = '\0';

    // printf("Randomization result: ");
    // printf("%s\n", buff);
}

void test_fs_init()
{
    HDD_deinit();

    FS_Init();

    // printf("notyet22\n");

    UINT8 buff[HDD_SectorSize + 1];
    HDD_ReadHeader(0, buff, HDD_SectorSize);
    buff[1024] = '\0';

    // printf("Header after init: ");
    // printf("%s\n", buff);
}

void test_fs_save_simple_file()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[10200];

    memset(data0, 65, 1020);
    memset(&data0[1020], 66, 1020);
    memset(&data0[2040], 67, 1020);
    memset(&data0[3060], 68, 1020);
    memset(&data0[4080], 69, 1020);
    memset(&data0[5100], 70, 1020);
    memset(&data0[6120], 71, 1020);
    memset(&data0[7140], 72, 1020);
    memset(&data0[8160], 73, 1020);
    memset(&data0[9180], 74, 1020);

    // UINT16 Handler0 = FS_SaveFile(data0, 10200);
    (void) FS_SaveFile(data0, 10200);

    // printf("File nr %d and %d\n", Handler0, Handler0 == FS_ERR_FULL || Handler0 == FS_ERR_ERROR);

    //HDD_ListUsedSectors();

    // printf("%d\n", Handler0);
}

void test_fs_save_uneven_file()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[2461];

    memset(data0, 65, 1020);
    memset(&data0[1020], 66, 1020);
    memset(&data0[2040], 67, 421);

    // UINT16 Handler0 = FS_SaveFile(data0, 2461);
    (void) FS_SaveFile(data0, 2461);

    // printf("File nr %d and %d\n", Handler0, Handler0 == FS_ERR_FULL || Handler0 == FS_ERR_ERROR);
    //printf("%d\n", Handler0);
}

void test_fs_alloc_fault()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[102000];

    memset(data0, 69, 102000);

    (void) FS_SaveFile(data0, 102000);
}

void test_fs_small_file()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[15];

    memset(data0, 69, 15);

    (void) FS_SaveFile(data0, 15);
}

void test_fs_first_sect_fault()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[1020];

    memset(data0, 69, 1020);

    (void) FS_SaveFile(data0, 1020);
    
}

void test_fs_read_file_whole()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[2040];

    for (int i = 0; i < 20; i++)
    {
        memset(&data0[i * 102], 65 + i, 102);
    }

    UINT16 Handle0 = FS_SaveFile(data0, 2040);
    
    UINT8 data1[2041];

    printf("%d\n", FS_ReadFile(Handle0, data1));
    printf("%d\n", FS_ReadFile(8, NULL));

    data1[2040] = '\0';

    printf("%s\n", data1);
}

void test_fs_read_file_uneven()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[2045];

    for (int i = 0; i < 20; i++)
    {
        memset(&data0[i * 102], 65 + i, 102);
    }

    memset(&data0[2040], 88, 5);

    UINT16 Handle0 = FS_SaveFile(data0, 2045);
    
    UINT8 data1[2046];

    printf("%d\n", FS_ReadFile(Handle0, data1));
    printf("%d\n", FS_ReadFile(8, NULL));

    data1[2045] = '\0';

    printf("%s\n", data1);
}

void test_fs_read_file_small()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[15];
    
    memset(data0, 88, 15);

    UINT16 Handle0 = FS_SaveFile(data0, 15);
    
    UINT8 data1[16];

    printf("%d\n", FS_ReadFile(Handle0, data1));
    printf("%d\n", FS_ReadFile(8, NULL));

    data1[15] = '\0';

    printf("%s\n", data1);
}

void test_fs_size_small()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[69];
    UINT8 data1[420];

    memset(data0, 88, 69);
    memset(data1, 88, 420);

    UINT16 Handle0 = FS_SaveFile(data0, 69);
    UINT16 Handle1 = FS_SaveFile(data1, 420);
    
    UINT32 size0 = FS_GetFileSize(Handle0);
    UINT32 size1 = FS_GetFileSize(Handle1);
    UINT32 size2 = FS_GetFileSize(15);

    printf("HAHAAA %d %d and even %d\n", size0, size1, size2);
}

void test_fs_size_normal()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[1020];
    UINT8 data1[4080];

    memset(data0, 88, 1020);
    memset(data1, 88, 4080);

    UINT16 Handle0 = FS_SaveFile(data0, 1020);
    UINT16 Handle1 = FS_SaveFile(data1, 4080);
    
    UINT32 size0 = FS_GetFileSize(Handle0);
    UINT32 size1 = FS_GetFileSize(Handle1);

    printf("Sizes %d and %d\n", size0, size1);
}

void test_fs_size_big()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[102000];
    UINT8 data1[408000];

    memset(data0, 88, 102000);
    memset(data1, 88, 408000);

    UINT16 Handle0 = FS_SaveFile(data0, 102000);
    UINT16 Handle1 = FS_SaveFile(data1, 408000);
    
    UINT32 size0 = FS_GetFileSize(Handle0);
    UINT32 size1 = FS_GetFileSize(Handle1);

    printf("Sizes %d and %d\n", size0, size1);
}

void test_fs_size_uneven()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[5115];
    UINT8 data1[4083];

    memset(data0, 88, 5115);
    memset(data1, 88, 4083);

    UINT16 Handle0 = FS_SaveFile(data0, 5115);
    UINT16 Handle1 = FS_SaveFile(data1, 4083);
    
    UINT32 size0 = FS_GetFileSize(Handle0);
    UINT32 size1 = FS_GetFileSize(Handle1);

    printf("Sizes %d and %d\n", size0, size1);
}

void test_fs_delete_small()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[420];
    UINT8 data1[69];

    memset(data0, 88, 420);
    memset(data1, 88, 69);

    UINT16 Handle0 = FS_SaveFile(data0, 420);
    UINT16 Handle1 = FS_SaveFile(data1, 69);
    
    BOOL del0 = FS_DeleteFile(Handle0);
    BOOL del1 = FS_DeleteFile(Handle1);
    BOOL del3 = FS_DeleteFile(14);

    printf("Deletes %d and %d AND EVEN %d\n", del0, del1, del3);
}

void test_fs_delete_normal()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[1020];
    UINT8 data1[4080];

    memset(data0, 88, 1020);
    memset(data1, 88, 4080);

    UINT16 Handle0 = FS_SaveFile(data0, 1020);
    UINT16 Handle1 = FS_SaveFile(data1, 4080);
    
    BOOL del0 = FS_DeleteFile(Handle0);
    BOOL del1 = FS_DeleteFile(Handle1);
    BOOL del3 = FS_DeleteFile(14);

    printf("Deletes %d and %d AND EVEN %d\n", del0, del1, del3);
}

void test_fs_delete_big()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[10200];
    UINT8 data1[40800];

    memset(data0, 88, 10200);
    memset(data1, 88, 40800);

    UINT16 Handle0 = FS_SaveFile(data0, 10200);
    UINT16 Handle1 = FS_SaveFile(data1, 40800);
    
    BOOL del0 = FS_DeleteFile(Handle0);
    BOOL del1 = FS_DeleteFile(Handle1);
    BOOL del3 = FS_DeleteFile(14);

    printf("Deletes %d and %d AND EVEN %d\n", del0, del1, del3);
}

void test_fs_delete_uneven()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[2045];
    UINT8 data1[1021];

    memset(data0, 88, 2045);
    memset(data1, 88, 1021);

    UINT16 Handle0 = FS_SaveFile(data0, 2045);
    UINT16 Handle1 = FS_SaveFile(data1, 1021);
    
    BOOL del0 = FS_DeleteFile(Handle0);
    BOOL del1 = FS_DeleteFile(Handle1);
    BOOL del3 = FS_DeleteFile(14);

    printf("Deletes %d and %d AND EVEN %d\n", del0, del1, del3);
}

void test_assignment()
{
    HDD_deinit();

    FS_Init();

    UINT8 data0[5000];
    memset(data0, 88, 5000);

    UINT8 data1[5000];
    memset(data1, 89, 5000);

    UINT8 dataRead[5000];

    UINT16 FileHandler0 = FS_SaveFile(&data0[0], 5000);

    assert(FileHandler0 != FS_ERR_FULL && FileHandler0 != FS_ERR_ERROR);
    
    UINT16 FileHandler1 = FS_SaveFile(&data1[0], 5000);
    
    assert(FileHandler1 != FS_ERR_FULL && FileHandler1 != FS_ERR_ERROR);

    UINT8 printBuff[5001];
    
    FS_ReadFile(FileHandler0, &dataRead[0]);
    
    assert(dataRead[0] == 88 && dataRead[4999] == 88);
    
    memcpy(printBuff, dataRead, 5000);
    printBuff[5000] = '\0';

    printf("First: %s\n", printBuff);
    
    FS_ReadFile(FileHandler1, &dataRead[0]);

    assert(dataRead[0] == 89 && dataRead[4999] == 89);

    memcpy(printBuff, dataRead, 5000);
    printBuff[5000] = '\0';

    printf("Second: %s\n", printBuff);

    BOOL ret = FS_DeleteFile(FileHandler0);

    assert(ret == TRUE);

    FS_ReadFile(FileHandler1, &dataRead[0]);

    assert(dataRead[0] == 89 && dataRead[4999] == 89);

    memcpy(printBuff, dataRead, 5000);
    printBuff[5000] = '\0';

    printf("Third: %s\n", printBuff);

    UINT32 len = FS_GetFileSize(FileHandler1);

    assert(len == 5000);

    ret = FS_DeleteFile(FileHandler1);

    assert(ret == TRUE);
}

int main(void)
{
    HDD_Init();
    
    /** HDD TESTS */

    //surface_test_hdd();

    //test_hdd_randomization();

    /* FS INIT TESTS */
    //test_fs_init();

    /* FS FILE SAVE TESTS */
    // test_fs_save_simple_file();
    
    // test_fs_save_uneven_file();
    
    // test_fs_alloc_fault();           //!!! NEEDS ACTIVE TEST_ALLOC_FAULT in HDD_impl.c
    
    // test_fs_small_file();

    // test_fs_first_sect_fault();      //!!! NEEDS ACTIVE TEST_FIRST_SECT_FAULT in HDD_impl.c

    /* FS READ TESTS */

    // test_fs_read_file_whole();

    // test_fs_read_file_uneven();

    // test_fs_read_file_small();

    /* FS SIZE TESTS */

    // test_fs_size_small();

    // test_fs_size_normal();

    // test_fs_size_big();

    // test_fs_size_uneven();

    /* FS DELETE TESTS */

    // test_fs_delete_small();

    // test_fs_delete_normal();

    // test_fs_delete_big();

    // test_fs_delete_uneven();

    test_assignment();

    HDD_ListUsedSectors();

    HDD_deinit();
 
    return 0;
}