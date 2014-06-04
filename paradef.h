#ifndef PARADEF_H
#define PARADEF_H

#include "CSParaDef.h"

#define OPTIONS ":afqhbeipvl:t:m:n:d:"
#define WHITE_SPACE " \n\r\t\v\f"
#define MAXLINE 4096
#define CHLEN 256 

#if defined(THREAD_THREADS_MAX) 
 #define THMAX THREAD_THREADS_MAX
#else 
#define THMAX -1
#endif  



#define PROCESS_DOTNAME 0x01
#define STARTS_BEG 0x02
#define ENDS_END 0x04
#define NOFOLLOW_SYMLINK 0x08
#define CASE_INSENSITIVE 0x10
#define PRINT_REALPATH 0x20
#define QUIET 0x40
#define INVERT_OUTPUT 0x80

#define DEBUG 1


typedef struct {

  int a;
  int f;
  int q;
  int i;
  int h;
  int b;
  int e;
  int p;
  int v;

} FLAG_t; //flags for switches


typedef struct {

  int d;
  int l;
  int m;
  int n;
  int t;

} OPTNUM_t; // argument following switches


typedef struct {
  
  int l;
  int m;
  int n;
  int d;
  int t;
  int final;
  
} STATUS_t; // err status



typedef struct{

  usint flags;
  int d;
  int l;
  int m;
  int n;
  int t;

}MESSAGE_ONE_t;


typedef struct {

  FLAG_t flag;
  OPTNUM_t num;
  STATUS_t err;
  char *pattern;

} DATA_t; //store all the data




#endif
