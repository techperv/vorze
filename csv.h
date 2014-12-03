#ifndef CSV_H
#define CSV_H

typedef struct {
	int timestamp;
	int v1;
	int v2;
} CsvEntry;

CsvEntry *csvLoad(char *file);
CsvEntry *csvGetForTs(CsvEntry *list, int timestamp);
void csvFree(CsvEntry *ent);

#endif