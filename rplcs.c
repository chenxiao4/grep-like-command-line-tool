#include "commonincludes.h"


THREADS_t       threads_info;
AGENT_WR_t      wr;
AGENT_INFO_t    ainfo;

int main (int argc, char *argv[]){

  NODE_t *root;
  time_type start_time, end_time;
  char node[TEXT_SIZE], serv[TEXT_SIZE], rfname[TEXT_SIZE];
  MESSAGE_t msg;
  int Shift[CHLEN];
  DATA_t data;

  memset(&msg,0,sizeof(msg));
  ainfo.nodetype = 1;
  //init all parameters in DATA_t, this function is in functions.c
  ClientParaInit(&data,&msg.type1);
  PInit();

  //Get the options, this is in functions.c
  setopt(argc,argv, &data, &msg.type1);
  PrintOpt(stdout,msg.type1);
  //if -h is set, exit immediatly with status 0 
  if (data.flag.h == 1 && data.err.final != -1)
    exit(EXIT_SUCCESS);

  //if this is any error in setting the options, the final err status is 1
  if (data.err.final == -1)
    return EXIT_FAILURE;

  data.pattern = argv[optind];
  memcpy(msg.type2,argv[optind],strlen(argv[optind])+1);

  //init shift table
  ShiftTable(&data,Shift);

  // PrintOpt();
  root = NewNode();//create the root node
  root->level = -1;
  memcpy(root->Shift,Shift,sizeof(Shift));
  memcpy(&root->data,&data,sizeof(DATA_t));



  get_time(&start_time);


  if (argc - optind == 1){
    //if nothing behind pattern, default is to input pattern
    // strings from stdin  
   
    DecentFile("-",root,0);

  } else {
    
    int ind;
    for (ind = optind + 1; ind < argc; ind++) {
 
      if (IS_LOCAL(argv[ind])){
	DecentDir(argv[ind],root);

      } else {

	if (remotefile(argv[ind],node,serv,rfname)){

	  memcpy(msg.type3,rfname,strlen(rfname)+1);
	  client(serv,node,root,msg);
	}

      }
    }//for

    /*
     * deal with main thread
     */
    CleanHd(root);
    WaitChild(root);
  }//endif
    
  get_time(&end_time);
  root->searchtime = TimeInterval(&start_time, &end_time);
  root->prate = 1.E-6 * root->statcs.bytesread / root->searchtime;
  root->statcs.thread_maxact += threads_info.max_act;
    
  PrintStats(stdout,root);
  DeNode(root);
  pthread_rwlock_destroy(&(wr.wr_mutex));
  return EXIT_SUCCESS;
}//end plcs
