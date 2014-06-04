#include "commonincludes.h"

THREADS_t      threads_info;
AGENT_INFO_t   ainfo;
AGENT_WR_t     wr;



static pthread_key_t key;
#ifdef __sun__
static pthread_once_t init = {PTHREAD_ONCE_INIT};
#else
static pthread_once_t init = PTHREAD_ONCE_INIT;
#endif

void thread_init(){

  pthread_key_create(&key,free);

}


void *server_agent(void *params){

	int     client_fd, n, errcode;
	HTYPE_t type;
	usint   len;

	NODE_t *root;
	time_type start_time, end_time;

	THREADS_t *allval;
	MESSAGE_ONE_t type1;
	char type2[BUFSIZE];
	char type3[BUFSIZE];
	int Shift[CHLEN];
	DATA_t data;

	errcode = pthread_detach(pthread_self());
	if (errcode != 0)
		fprintf(stderr,"pthread_detach server agent: %s\n",strerror(errcode));

	client_fd = ((THP_t *)params)->client_fd;
	printf("%s: servicing fd %d, thread id %lu\n","starting agent",client_fd,(unsigned long)pthread_self());

	get_time(&start_time);


	len = sizeof(type1);
	type1.flags = 0;

	if ((n = our_recv_message(client_fd, &type, &len,&type1)) < 0)
		goto clean;
	if (type != CLIENT_OPT)
		goto clean;

	OptToh(&type1);

	/*
	 * reproting connection status
	 */
	printf("%s: agent thread's fd %d, thread id %lu\n","client options",client_fd,(unsigned long)pthread_self());

	//pthread_rwlock_wrlock(&wr.wr_mutex);
		PrintOpt(stderr,type1);
	//pthread_rwlock_wrlock(&wr.wr_mutex);

	//printf("\n server receiving client's options: %0x\n",type1.flags);

	len = BUFSIZE;
	if ((n = our_recv_message(client_fd, &type, &len, type2)) < 0)
		goto clean;
	if (type != CLIENT_STRING)
		goto clean;
	//printf("\n server receiving client's search string: %s\n",type2);

	len =BUFSIZE;
	if ((n = our_recv_message(client_fd, &type, &len, type3)) < 0)
		goto clean;
	if (type != CLIENT_FILENAME)
		goto clean;
	// printf("\n server receiving client's target filename: %s\n",type3);



  pthread_once(&init,thread_init);


  if ((allval = (THREADS_t *)pthread_getspecific(key)) == NULL){

    allval = malloc(sizeof(THREADS_t));
    pthread_setspecific(key,allval);

  }

  memcpy(&threads_info,&allval,sizeof(THREADS_t));


	/*
	 * starting searching
	 */
	ServerParaInit(&data,type1);
	data.pattern = type2;

	PInit();

	//init shift table
	ShiftTable(&data,Shift);

	root = NewNode();//create the root node
	root->level = -1;
	memcpy(root->Shift,Shift,sizeof(int)*CHLEN);
	memcpy(&root->data,&data,sizeof(DATA_t));
	root->client_fd = client_fd;


	DecentDir(type3,root);

	CleanHd(root);

	WaitChild(root);

	get_time(&end_time);
	root->searchtime = TimeInterval(&start_time, &end_time);
	root->prate = 1.E-6 * root->statcs.bytesread / root->searchtime;
	root->statcs.thread_maxact = threads_info.max_act;

	pthread_rwlock_wrlock(&wr.wr_mutex);
		fprintf(stdout,"\n%2s****** Search Statistics: agent thread's fd %d, id %lu ******\n","*",\
				client_fd,(unsigned long)pthread_self());
		PrintStats(stderr,root);
    pthread_rwlock_unlock(&wr.wr_mutex);

	StatToNet(&root->statcs);

	/*
	 * writte message type6
	 */
	pthread_rwlock_wrlock(&wr.wr_mutex);
	len = sizeof(root->statcs);
	if (our_send_message(client_fd,AGENT_STAT,len,&root->statcs) == -1)
		fprintf(stderr,"client send message type 6 fail\n");
	pthread_rwlock_unlock(&wr.wr_mutex);

	DeNode(root);


	clean:
	if (close(client_fd) < 0)
		perror("server close connection to client");

	free(params);
	printf("terminating agent: servicing fd %d, thread id %lu\n",client_fd, \
			(unsigned long)pthread_self());
	return NULL;

}



void OptToh(MESSAGE_ONE_t *type1){

	type1->flags = ntohl(type1->flags);
	type1->d = ntohl(type1->d);
	type1->l = ntohl(type1->l);
	type1->m = ntohl(type1->m);
	type1->n = ntohl(type1->n);
	type1->t = ntohl(type1->t);
}



void StatToNet(INFO_t *statcs){

	statcs->symlink = htonl(statcs->symlink);
	statcs->dir = htonl(statcs->dir);
	statcs->loop = htonl(statcs->loop);
	statcs->dir_pruned = htonl(statcs->dir_pruned);
	statcs->maxdirdepth = htonl(statcs->maxdirdepth);
	statcs->dotname = htonl(statcs->dotname);
	statcs->thread = htonl(statcs->thread);
	statcs->thread_pruned = htonl(statcs->thread_pruned);
	statcs->thread_maxact = htonl(statcs->thread_maxact);
	statcs->errs = htonl(statcs->errs);
	statcs->linematch = htonl(statcs->linematch);
	statcs->lineread = htonl(statcs->lineread);
	statcs->file = htonl(statcs->file);
	statcs->bytesread = htonl(statcs->bytesread);
}



void listener(char *server_port, char *interface_name, int argc, char *argv[]){

	socklen_t len;
	int listening_fd, errcode;
	SA_t server;
	SAIN_t *iptr;
	char text_buf[TEXT_SIZE];
	THP_t *params;
	pthread_t agent_id;
	int i;

	no_sigpipe();

	pthread_rwlock_init(&(wr.wr_mutex),NULL);
	//pthread_mutex_init(&(threads_info.all_mutex),NULL);

	for (i = 1; i < argc; i++)
		fprintf(stderr,"unsued command-line arguments %s\n",argv[i]);


	listening_fd = openlistener(server_port,interface_name, &server);
	if (listening_fd < 0)
		exit(EXIT_FAILURE);


	ainfo.nodetype = 0; //for server, nodetype is 1 for client


	for(; ;){

		iptr = (SAIN_t *)&server;
		if (inet_ntop(iptr->sin_family, &iptr->sin_addr,text_buf,TEXT_SIZE) == NULL){
			perror("inet_ntop server");
			continue;
		}

		printf("\nserver listening at IP address %s port %d\n",text_buf,\
				ntohs(iptr->sin_port));


		params = malloc(sizeof(THP_t));
		if (params == NULL){
			perror("listener malloc");
			continue;
		}

		len = sizeof(params->client);
		if ((params->client_fd = accept(listening_fd,&params->client,&len)) < 0){
			perror("server accept");
			continue;
		}


		iptr = (SAIN_t *)(&((THP_t *)params)->client);
		if (inet_ntop(iptr->sin_family,&iptr->sin_addr,text_buf,TEXT_SIZE) ==  NULL){

			perror("inet_ntop client");
			free(params);
			continue;

		}
		printf("server connected to client at IP address %s client port %d socket fd %d\n"\
				,text_buf,ntohs(iptr->sin_port),params->client_fd);

		errcode = pthread_create(&agent_id, NULL, server_agent, params);

		if (errcode != 0){
			fprintf(stderr,"pthread_create server agent: %s\n",strerror(errcode));
			continue;
		}
	}

	if (close(listening_fd) < 0)
		perror("server close");
	pthread_rwlock_destroy(&(wr.wr_mutex));
	pthread_exit(NULL);

}
