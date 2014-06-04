#ifndef SEARCH_H
#define SEARCH_H

#include "paradef.h"


void ShiftTable(DATA_t *data, int *Shift);
void strslc(char *str, char *p, int position, int length);
int BM_Search(char *buf, DATA_t *data, int *Shift);
int stringcmp(char *s1, char *s2, int m, int i);
int search(char *buf, DATA_t *data, int *Shift);

#endif
