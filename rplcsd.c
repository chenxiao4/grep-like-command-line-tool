#include "commonincludes.h"





int main (int argc, char *argv[]){

  char *server_port;
  char *interface_name;

  argv++;
  argc--;

  if (argc == 0){
    server_port = NULL;
  } else {
    server_port = *argv++;
    argc--;
  }


  if(argc == 0){
    interface_name = NULL;
  } else {
    interface_name = *argv++;
    argc--;
  }


  listener(server_port,interface_name,argc,argv);
 
  //init all parameters in DATA_t, this function is in functions.c

  return EXIT_SUCCESS;

  
}//end plcs
