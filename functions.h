#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "paradef.h"
void PrintFlag(FILE *fp,usint flags, usint this_one, char *name);
void PrintVal(FILE *fp,char *name, int value);
void PrintOpt(FILE *fp,MESSAGE_ONE_t type1);
int trim_line(char *buf);
int scan_switch_num(int switch_char, int *result);
void print_realpath(char *fname);
void setopt(int argc, char *argv[], DATA_t *data,MESSAGE_ONE_t *type1);
void ClientParaInit(DATA_t *data,MESSAGE_ONE_t *type1);
void ServerParaInit(DATA_t *data, MESSAGE_ONE_t type1);
#endif
