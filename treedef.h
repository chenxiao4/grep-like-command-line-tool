#ifndef TREEDEF_H
#define TREEDEF_H

#include "LibHeader.h"
#include "paradef.h"


typedef struct stat     STAT_t;
typedef struct dirent   DIR_t;


typedef struct {

  usint symlink;
  usint dir;
  usint loop;
  usint dir_pruned;//pruned
  usint maxdirdepth;
  usint dotname;
  usint thread;
  usint thread_pruned;//pruned
  usint thread_maxact;
  usint errs;
  usint linematch;
  usint lineread;
  usint file;
  usint bytesread;

  //usint thread_fail;

} INFO_t; //statistics for each node



struct node {
 
  char *realname;
  char *fullname;
  int level;
  DIR *dp;
  char dirname[BUFSIZE];
  struct node *parent;

  INFO_t statcs; //dir info

  pthread_t tid;
  pthread_mutex_t node_mutex;
  pthread_cond_t cid;
  int client_fd;
  int Shift[CHLEN];
  DATA_t data;
  double searchtime;
  double prate;
  int nref;//number of child

};//node structure

typedef struct node NODE_t;



typedef struct{

  NODE_t *node;
  char *fname;

} PARAMS_t;



typedef struct {

  usint concur;//concurrent
  usint max_act;//max
  
  pthread_mutex_t all_mutex;

} THREADS_t;


typedef struct{

  usint errs;
  pthread_mutex_t err_mutex;

} ERR_t;



typedef struct{

  MESSAGE_ONE_t type1;
  char type2[BUFSIZE];
  char type3[BUFSIZE];

} MESSAGE_t;


typedef struct{

  char server_port[TEXT_SIZE];
  char server_name[TEXT_SIZE];
  NODE_t *node;
  MESSAGE_t msg;

} CLIENT_t;



typedef struct{

  long line;
  pthread_rwlock_t wr_mutex;

} AGENT_WR_t;


#endif
