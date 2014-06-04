#ifndef CSPARADEF_H
#define CSPARADEF_H



#define LISTENING_DEPTH 3
#define BUFSIZE 4096
#define TEXT_SIZE 256


typedef struct sockaddr       SA_t;
typedef struct sockaddr_in    SAIN_t;
typedef struct addrinfo       AI_t;
typedef unsigned int          usint;
/*
 * format of header
 */
typedef struct {

  usint type;
  usint length;

} HD_t;



typedef struct{

  char node[TEXT_SIZE];
  char port[TEXT_SIZE];
  int nodetype;//1 for client
}AGENT_INFO_t;



typedef struct {

  int client_fd;
  SA_t client;
  AGENT_INFO_t ainfo;

} THP_t;

/*
 * header type
 */
typedef enum {

  CLIENT_OPT = 1,
  CLIENT_STRING,
  CLIENT_FILENAME,
  AGENT_OUTPUT,
  AGENT_ERROR,
  AGENT_STAT,

} HTYPE_t;



#endif
