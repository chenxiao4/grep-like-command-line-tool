#ifndef DOSEARCH_H
#define DOSEARCH_H

#include "paradef.h"
#include "treedef.h"


int boolread(FILE *fp, int mnum, int linelim);
void doserach(FILE *fp, char *buf,char *fname,NODE_t *node);
void process_search(int source,char *buf	\
		    ,char *fname,NODE_t *node);
char *MessageMake(char *fmt, ...);


#endif
