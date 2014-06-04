#include "commonincludes.h"

extern AGENT_WR_t      wr;

/* 
 * chech if it is local file
 */
int IS_LOCAL(char *fname){

	int ret;
	char *ptr;

	if ((ptr = strchr(fname,':')) == NULL)
		ret = 1;
	else
		ret = 0;

	return ret;
}


/*
 *if it's not local file, get node serv,and file name
 */

int remotefile(char *string, char *node, char *serv, char *fname){

	/*
	 *  0 on error
	 */
	char *str;
	int ret = 1;
	char *ptr, *temp, *pend = NULL;
	int pa, pb;
	char *scp1, *scp2, *scp3;

	str = (char *)malloc((strlen(string) + 1)*sizeof(char));

	strcpy(str,string);
	/*
	 * get the node name
	 */
	ptr = strchr(str,':');
	scp1 = str;
	pa = ptr - str;
	*(scp1 + pa) = '\0';
	ptr++;

	if ((int) strlen(scp1) == 0){
		ret = 0;
		fprintf(stderr,"<node:serv/filename>: node is empty\n");
		return ret;
	}

	strcpy(node,scp1);

	/*
	 * get the serv name
	 */
	scp2 = ptr;
	temp = strchr(ptr,'/');
	pb = temp - ptr;
	*(scp2 + pb) = '\0';
	temp++;

	if ((int) strlen(scp2) == 0){
		ret = 0;
		fprintf(stderr,"<node:serv/filename>: node is empty\n");
		return ret;
	}

	if (strtol(scp2,&pend,10) == 0L || strlen(pend) != 0){
		ret = 0;
		fprintf(stderr,"<node:serv/filename>: serv must be valid decimal number\n");
		return ret;

	}

	strcpy(serv,scp2);

	/*
	 * get the remote file name
	 */
	scp3 = temp;

	if ((int) strlen(scp3) == 0){
		ret = 0;
		fprintf(stderr,"<node:serv/filename>: node is empty\n");
		return ret;
	}

	strcpy(fname,scp3);

	free(str);
	return ret;
}





void *client_agent(void *params){

	int			fd, result, one = 1, n, errcode;
	unsigned int		len;
	struct sockaddr	server, client;
	struct sockaddr_in	*iptr;
	char			text_buf[TEXT_SIZE];
	socklen_t		size;
	HTYPE_t               type;
	MESSAGE_ONE_t         type1;
	char                  *type2, *type3;
	char                  type4[BUFSIZE];
	INFO_t                *type6;
	NODE_t                *node;
	char                  *server_port;
	char                  *server_name;
	CLIENT_t              *arg = (CLIENT_t *)params;



	/* ignore sigpipe signals -- handle the error synchronously */
	no_sigpipe();

	errcode = pthread_detach(pthread_self());
	if (errcode != 0)
		fprintf(stderr,"pthread detach client: %s\n",strerror(errcode));

	server_port = arg->server_port;
	server_name = arg->server_name;
	node = arg->node;
	memcpy(&type1,&(arg->msg.type1),sizeof(MESSAGE_ONE_t));
	type2 = arg->msg.type2;
	type3 = arg->msg.type3;
	/* establish a connection to indicated server */

	// printf("%s: %s\n",server_port,server_name);

	fd = openclient(server_port, server_name, &server, &client);

	if (fd < 0){

		pthread_mutex_lock(&node->node_mutex);
		node->nref--;
		pthread_mutex_unlock(&node->node_mutex);
		pthread_cond_signal(&node->cid);
		return NULL;
		//exit(EXIT_FAILURE);
	}
	/* we are now successfully connected to a remote server */
	iptr = (struct sockaddr_in *)&client;
	if (inet_ntop(iptr->sin_family, &iptr->sin_addr, text_buf, TEXT_SIZE)
			== NULL) {
		perror("inet_ntop client");
		exit(EXIT_FAILURE);
	}
	printf("client2tcp at IP address %s port %d\n",
			text_buf, ntohs(iptr->sin_port));
	iptr = (struct sockaddr_in *)&server;
	if (inet_ntop(iptr->sin_family, &iptr->sin_addr, text_buf, TEXT_SIZE)
			== NULL) {
		perror("inet_ntop server");
		exit(EXIT_FAILURE);
	}
	printf("client2tcp connected to server at IP address %s port %d\n",
			text_buf, ntohs(iptr->sin_port));


	/* first demonstrate socket-level options */
	size = sizeof(result);
	if( getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &result, &size) < 0 )
		perror("getsockopt REUSEADDR");
	//  else
	//  fprintf(stderr, "REUSEADDR value is %d\n", result);

	if( setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0 )
		perror("setsockopt REUSEADDR");
	//else
	//  fprintf(stderr, "REUSEADDR value set to %d\n", one);

	size = sizeof(result);
	if( getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &result, &size) < 0 )
		perror("getsockopt REUSEADDR");
	// else
	//  fprintf(stderr, "REUSEADDR value is now %d\n", result);

	size = sizeof(result);
	if( getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &result, &size) < 0 )
		perror("getsockopt KEEPALIVE");
	//else
	//  fprintf(stderr, "KEEPALIVE value is %d\n", result);

	if( setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)) < 0 )
		perror("setsockopt KEEPALIVE");

	size = sizeof(result);
	if( getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &result, &size) < 0 )
		perror("getsockopt KEEPALIVE");

	/* now demonstrate tcp-level options */
	size = sizeof(result);
	if( getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &result, &size) < 0 )
		perror("getsockopt TCP_NODELAY");

	if( setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)) < 0 )
		perror("setsockopt TCP_NODELAY");

	size = sizeof(result);
	if( getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &result, &size) < 0 )
		perror("getsockopt TCP_NODELAY");

	/*
	 * starting to send message type form 1 to 3
	 */


	len = sizeof(type1);
	OptToNet(&type1);
	if (our_send_message(fd,CLIENT_OPT,len,&type1) == -1){
		fprintf(stderr,"client send message type 1 fail\n");
		goto quit;
	}

	len = strlen(type2);
	if (our_send_message(fd,CLIENT_STRING,len,type2) == -1){
		fprintf(stderr,"client send message type 2 fail\n");
		goto quit;
	}

	len = strlen(type3);
	if (our_send_message(fd,CLIENT_FILENAME,len,type3) == -1){
		fprintf(stderr,"client send message type 3 fail\n");
		goto quit;
	}



	/*
	 * recv message type4 5 6 from server
	 */

//	pthread_rwlock_rdlock(&wr.wr_mutex);
	while (1){

		len = BUFSIZE;

		if ((n = our_recv_message(fd, &type, &len, type4)) < 0)
			break;

	//	printf("------>recv: type %d: len %d\n",type,len);

		if (type != AGENT_OUTPUT && type != AGENT_ERROR){
			if (type == AGENT_STAT){

				type6 = (INFO_t *)type4;
				StatToh(type6);
				UpdateClientStat(node,type6);
			}
			break;
		}
		if (type == AGENT_OUTPUT)
			fprintf(stdout,"%s:%d%s%s",text_buf, ntohs(iptr->sin_port),"/",type4);
		if (type == AGENT_ERROR)
			fprintf(stderr,"%s:%d%s%s",text_buf, ntohs(iptr->sin_port),"/",type4);
	}

//	pthread_rwlock_unlock(&wr.wr_mutex);

	quit:
	if (close(fd) < 0){
		perror("client close");
	}



	pthread_mutex_lock(&node->node_mutex);
	node->nref--;
	pthread_mutex_unlock(&node->node_mutex);
	pthread_cond_signal(&node->cid);
	free(params);
	return NULL;
}



void OptToNet(MESSAGE_ONE_t *type1){

	type1->flags =htonl(type1->flags);
	type1->d = htonl(type1->d);
	type1->l = htonl(type1->l);
	type1->m = htonl(type1->m);
	type1->n = htonl(type1->n);
	type1->t = htonl(type1->t);
}




void StatToh(INFO_t *statcs){

	statcs->symlink = ntohl(statcs->symlink);
	statcs->dir = ntohl(statcs->dir);
	statcs->loop = ntohl(statcs->loop);
	statcs->dir_pruned = ntohl(statcs->dir_pruned);
	statcs->maxdirdepth = ntohl(statcs->maxdirdepth);
	statcs->dotname = ntohl(statcs->dotname);
	statcs->thread = ntohl(statcs->thread);
	statcs->thread_pruned = ntohl(statcs->thread_pruned);
	statcs->thread_maxact = ntohl(statcs->thread_maxact);
	statcs->errs = ntohl(statcs->errs);
	statcs->linematch = ntohl(statcs->linematch);
	statcs->lineread = ntohl(statcs->lineread);
	statcs->file = ntohl(statcs->file);
	statcs->bytesread = ntohl(statcs->bytesread);
}





void UpdateClientStat(NODE_t *node, INFO_t *statcs){


	pthread_mutex_lock(&node->node_mutex);

	node->statcs.file += statcs->file;
	node->statcs.dir +=  statcs->dir;
	node->statcs.loop += statcs->loop;
	node->statcs.symlink += statcs->symlink;
	node->statcs.dir_pruned += statcs->dir_pruned;

	node->statcs.lineread += statcs->lineread;
	node->statcs.linematch += statcs->linematch;
	node->statcs.bytesread += statcs->bytesread;

	if ( statcs->maxdirdepth > node->statcs.maxdirdepth )
		node->statcs.maxdirdepth = statcs->maxdirdepth;

	node->statcs.thread += statcs->thread;
	node->statcs.thread_maxact += statcs->thread_maxact;
	node->statcs.errs += statcs->errs;
	node->statcs.thread_pruned += statcs->thread_pruned;
	node->statcs.dotname += statcs->dotname;

	pthread_mutex_unlock(&node->node_mutex);

}



void client(char *serv, char *node, NODE_t *root, MESSAGE_t msg){

	CLIENT_t *carg;
	pthread_t tid;
	int errcode;


	pthread_mutex_lock(&root->node_mutex);
	root->nref++;
	pthread_mutex_unlock(&root->node_mutex);


	carg = tralloc(sizeof(CLIENT_t));
	memcpy(&carg->msg,&msg,sizeof(MESSAGE_t));
	memcpy(carg->server_port,serv,TEXT_SIZE);
	memcpy(carg->server_name,node,TEXT_SIZE);
	carg->node = root;

	//printf("client: %s: %s\n",serv,node);

	errcode = pthread_create(&tid,NULL,client_agent,carg);
	if (errcode != 0){
		fprintf(stderr,"pthread creat client: %s\n",strerror(errcode));
	}

}
