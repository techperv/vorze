#include "csv.h"
#include <stdio.h>
#include <stdlib.h>

CsvEntry *csvLoad(char *file) {
	CsvEntry *ret;
	int n;
	int ts, v1, v2;
	FILE *f;
	char buff[1024];
	
	f=fopen(file, "r");
	if (f==NULL) {
		perror(file);
		return NULL;
	}
	//First, figure out amount of lines in file.
	n=0;
	while(!feof(f)) {
		fgets(buff, sizeof(buff), f);
		if (sscanf(buff, "%d,%d,%d", &ts, &v1, &v2)==3) n++;
	}
	//Allocate memory for everything
	ret=malloc(sizeof(CsvEntry)*(n+1));
	//Get values
	rewind(f);
	n=0;
	while(!feof(f)) {
		fgets(buff, sizeof(buff), f);
		if (sscanf(buff, "%d,%d,%d", &ret[n].timestamp, &ret[n].v1, &ret[n].v2)==3) n++;
	}
	ret[n].timestamp=-1;
	ret[n].v1=0;
	ret[n].v2=0;
	printf("Read %d entries.\n", n);
	return ret;
}

CsvEntry *csvGetForTs(CsvEntry *list, int timestamp) {
	int idx=-1;
	int n=0;
	while (list[n].timestamp>=0) {
		if (list[n].timestamp<=timestamp) {
			if (idx==-1 || list[n].timestamp>list[idx].timestamp) {
				idx=n;
			}
		}
		n++;
	}
	if (idx==-1) {
		if (n>1) {
			return &list[n-1];
		} else {
			return &list[0];
		}
	}
	return &list[idx];
}

void csvFree(CsvEntry *ent) {
	free(ent);
}

