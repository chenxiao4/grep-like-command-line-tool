#include "commonincludes.h"


extern THREADS_t threads_info;
extern AGENT_INFO_t  ainfo;
extern AGENT_WR_t wr;

int boolread(FILE *fp, int mnum, int linelim) {
  int i;
  

  //to check the argument of -m, and setup the max ouput lines

  if (mnum == 0){ 
    i = !feof(fp);
  } else {
    i = !feof(fp) && linelim <= mnum;
  }
  return i;

}



void doserach(FILE *fp,char *buf,char *fname, NODE_t *node){

  int index,lineno;
  int max_len;
  int mnum;
  int nnum;
  int line;
  long lineread, bytesread, linematch;
  usint len;
  int client_fd;
  char *memptr;
  char *realname;
  size_t size;
  char *type4;
  int err;

  max_len = node->data.num.l;
  mnum = node->data.num.m;
  nnum = node->data.num.n;
  line = 1;

  client_fd = node->client_fd;

  lineread = 0;
  bytesread = 0;
  linematch = 0;


  for (lineno = 1; boolread(fp,mnum,line); lineno++)
    {

      if (fgets(buf,max_len,fp) == NULL)
	break;


      bytesread += (long)strlen(buf);
      lineread++;

      //the special case : pattern string is null
      if (strlen(node->data.pattern) == 0 ){

	trim_line(buf);
	index = search(buf,&node->data,node->Shift);

      } else {

	trim_line(buf) ;
	index = search(buf,&node->data,node->Shift);

      }

      
      if (index != -1){
	 
	linematch++;	  

	if (node->data.flag.p == 1 && fp != stdin){

	  realname = path_alloc(&size);
	  memptr = realpath(fname,realname);

	  if (nnum > 0){

	    fprintf(stdout,"%s: %*d: %s\n",memptr,nnum,lineno,buf);
	    
	  }else {
	    //no lineno if -n is not set
	    fprintf(stdout,"%s: %s\n",memptr,buf);
	  }
	  
	  free(realname);
	} else {

	  if (nnum > 0){

	    fprintf(stdout,"%*d: %s\n",nnum,lineno,buf);
	    
	  }else {
	    //no lineno if -n is not set
	    fprintf(stdout,"%s\n",buf);
	  }

	}
	/*
	 * create message type 4
	 */


	if (ainfo.nodetype != 1){


	if ((err = pthread_rwlock_wrlock(&wr.wr_mutex)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}
	if (node->data.flag.p){

	    char *realptr;
	    char *memptr;
	    size_t size;
	    memptr = path_alloc(&size);
	    realptr = realpath(fname,memptr);

	    if (nnum > 0)
	      type4 = MessageMake("%s: %*d: %s\n",realptr,nnum,lineno,buf);
	    else
	      type4 = MessageMake("%s: %s\n",realptr,buf);	      

	    free (memptr);
	  } else {

	    if (nnum > 0)
	      type4 = MessageMake("%*d: %s\n",nnum,lineno,buf);	      
	    else
	      type4 = MessageMake("%s\n",buf);	      

	  }

	
	 // pthread_rwlock_wrlock(&wr.wr_mutex);
	  len = strlen(type4);
	  if (our_send_message(client_fd,AGENT_OUTPUT,len+1,type4) == -1){
	    fprintf(stderr,"%d: agent send message type 4 fail\n",client_fd);
	  }
	  free(type4);

		if ((err = pthread_rwlock_unlock(&wr.wr_mutex)) != 0){

			fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
			return;
		}

	}
	/*
	 * done with type4
	 */
	   
	line++;
      }
    }//end for


  if (fp == stdin)
    rewind(stdin);

  
  pthread_mutex_lock(&node->node_mutex);
  node->statcs.lineread += lineread;
  node->statcs.linematch += linematch;
  node->statcs.bytesread += bytesread;
  pthread_mutex_unlock(&node->node_mutex);

}




void process_search(int source,char *buf,char *fname, NODE_t *node){

  //source = 0 for stdin, and fname = NULL
  //source = 1 for file
  FILE *fp;

  if (source == 0)  
    doserach(stdin,buf,NULL,node);

  else if (source == 1){  
    if ((fp = fopen(fname,"r")) == NULL || ferror(fp)){
      //perror(fname);
      RepFileErr((char *)fname,node);
      fclose(fp);
      return;
    } else {
      doserach(fp, buf,fname,node);
      fclose(fp);
    }	
  }  


}



char *MessageMake(char *fmt, ...){

  char *ptr;
  va_list ap;
  
  ptr = (char *)tralloc(BUFSIZE);
  
  va_start(ap,fmt);
  vsnprintf(ptr,BUFSIZE-1,fmt,ap);
  fflush(stdout);
  fflush(stderr);
  va_end(ap);

  return ptr;
}

