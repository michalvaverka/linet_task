#ifndef HDDADITIONAL
#define HDDADITIONAL

void HDD_randomize_header(char * source, int len);

void HDD_deinit(void);

void HDD_ListUsedSectors(void);

BOOL anysectorused(void);

#endif /* HDDADITIONAL */