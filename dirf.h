#ifndef DIRF_H
#define DIRF_H

#include "treedef.h"

char *path_alloc(size_t *sizep);
char *fullpath(const char *fname, char *realname);
void PInit();
void *tralloc(size_t size);
NODE_t *NewNode();
NODE_t *MakeNode(NODE_t *parent, char *fname);
void CleanHd(NODE_t *node);
void DeNode(NODE_t *node);
void WaitChild(NODE_t *parent);
void RepFileErr(const char *fname, NODE_t *node);
int Depth(NODE_t *parent);
int LoopHd(NODE_t *node);
int DetectLoop(NODE_t *parent);
void StatErr(NODE_t *node);
DIR_t *dirent_alloc(NODE_t *node);


#endif
