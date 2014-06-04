#ifndef DODIR_H
#define DODIR_H

#include "treedef.h"
void DecentFile(const char *filename,NODE_t *parent, int io);
void DecentDir(char *fname,NODE_t *node);
void MoveOn(NODE_t *node);
void *IntoDir(void *arg);
void Recursion(NODE_t *node);
void DirClean(NODE_t *node);
void PrintStats(FILE *fptr, NODE_t *root);
void WriteErr(int fd,char *fmt,...);
void WriteLoop(int fd,char *fmt,...);
void WritePerror(int fd, char *fname);


#endif
