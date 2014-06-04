#include "commonincludes.h"

extern THREADS_t threads_info;
extern AGENT_WR_t wr;
extern AGENT_INFO_t ainfo;

void PInit(){

	threads_info.concur = 0;
	threads_info.max_act = 0;
	//if (ainfo.nodetype == 1)
	pthread_mutex_init(&(threads_info.all_mutex),NULL);
	wr.line = 0;//actually not used
	if (ainfo.nodetype == 1)
		pthread_rwlock_init(&(wr.wr_mutex),NULL);

}//end PInit


//creating new node for the tree
NODE_t *NewNode(){

	NODE_t *node = (NODE_t *)tralloc(sizeof(NODE_t));

	node->realname = NULL;
	node->fullname = NULL;
	node->parent = NULL;
	memset(&node->statcs,0,sizeof(node->statcs));
	pthread_mutex_init(&(node->node_mutex),NULL);
	pthread_cond_init(&(node->cid),NULL);

	node->level = 0;
	node->tid = 0;
	node->nref = 1;
	node->dp = NULL;
	return node;
}//end NewNode


NODE_t *MakeNode(NODE_t *parent, char *fname){

	char *nameptr, *realname, *ptr;
	size_t size;
	NODE_t *newnode;

	realname = path_alloc(&size);

	if (parent->parent == NULL){
		nameptr = fullpath(fname,NULL);
	} else {
		nameptr = fullpath(fname,parent->realname);
	}

	if ((ptr = realpath(nameptr,realname)) == NULL){
		RepFileErr(nameptr,parent);
		exit(EXIT_FAILURE);
	}


	newnode = NewNode();  //creating new node
	newnode->parent = parent;  //link to parent node
	newnode->realname = realname;
	newnode->fullname = nameptr;
	newnode->level = parent->level+1;
	newnode->client_fd = parent->client_fd;
	memcpy(newnode->Shift,parent->Shift,sizeof(parent->Shift));
	memcpy(&newnode->data,&parent->data,sizeof(DATA_t));
	pthread_mutex_lock(&(parent->node_mutex));
	parent->nref++;
	pthread_mutex_unlock(&(parent->node_mutex));
	return newnode;
}



//delete a node from the tree
void DeNode(NODE_t *node){

	if (node->parent ==NULL ){
		pthread_mutex_destroy(&(threads_info.all_mutex));
	}
	pthread_mutex_destroy(&(node->node_mutex));
	pthread_cond_destroy(&(node->cid));
	free(node->realname);
	free(node->fullname);
	if (node->dp)
		if (closedir(node->dp)){
			fprintf(stderr,"Could not close %s\n",node->realname);
			WriteErr(node->client_fd,"Could not close %s\n",node->realname);
			exit(EXIT_FAILURE);
		}
	if (node->nref !=0)
		printf("Reference is : %d\n",node->nref);
	assert(node->nref == 0);
	free(node);
}//end DeNode



void CleanHd(NODE_t *node){

	int err;
	NODE_t *temp,*freenode;
	pthread_rwlock_t lock;

	pthread_rwlock_init(&lock,NULL);

	temp = node;


	/*
	 * if it is the main thread (root)
	 */
	if (temp->realname == NULL){
		pthread_mutex_lock(&temp->node_mutex);
		temp->nref--;
		pthread_mutex_unlock(&temp->node_mutex);
		pthread_cond_signal(&temp->cid);
		return;
	}

	/*
	 * it is not the root
	 */
	if ((err = pthread_rwlock_wrlock(&lock)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}

	while (1){

		temp->nref--;

		if (temp->parent == NULL)
			break;

		if (temp->nref == 0){
			DirClean(temp);
			freenode = temp;
			temp = temp->parent;
			DeNode(freenode);
		} else
			break;
	}


	if ((err = pthread_rwlock_unlock(&lock)) != 0){
		fprintf(stderr,"rw_unlock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}

	pthread_rwlock_destroy(&lock);
	if (temp->realname == NULL){
		pthread_cond_signal(&temp->cid);
	}
}




//waiting for all child of the parent node
void WaitChild(NODE_t *parent){

	pthread_mutex_t search_lock;

	pthread_mutex_init(&search_lock,NULL);
	//pthread_mutex_t search_lock = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&search_lock);
	while (parent->nref > 0){
		//printf("main thread waiting %d\n",parent->nref);
		pthread_cond_wait(&(parent->cid),&search_lock);
	}
	pthread_mutex_unlock(&search_lock);

	pthread_mutex_destroy(&search_lock);

}//endWait


void StatErr(NODE_t *node){

	pthread_mutex_lock(&(node->node_mutex));
	node->statcs.errs++;//errors due to -q
	pthread_mutex_unlock(&(node->node_mutex));

}


void RepFileErr(const char *fname, NODE_t *node){

	if (!node->data.flag.q || node->realname == NULL){
		perror(fname);
		WritePerror(node->client_fd,(char *)fname);
	}else
		StatErr(node);

}//end Rep



int Depth(NODE_t *parent){

	int res = 0;
	NODE_t *node;

	node = parent;
	while (node->realname != NULL){

		res++;
		node = node->parent;

	}

	return res;

}//end Depth


int DetectLoop(NODE_t *parent){

	int res = 0, err;
	NODE_t *node;
	node = parent->parent;

	if ((err = pthread_rwlock_rdlock(&wr.wr_mutex)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return -1;
	}

	while (node->realname != NULL){

		if (strcmp(node->realname,parent->realname) == 0){
			res = 1;
			break;
		}
		node = node->parent;
	}

	if ((err = pthread_rwlock_unlock(&wr.wr_mutex)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return -1;
	}
	return res;
}//end DetectLopp




int LoopHd(NODE_t *node){

	int ret = 0;
	NODE_t *current;

	current = node->parent;

	ret = DetectLoop(node);

	if (ret){

		pthread_mutex_lock(&(node->parent->node_mutex));
		node->parent->statcs.loop++;
		pthread_mutex_unlock(&(node->parent->node_mutex));

		if (node->data.flag.q){
			StatErr(node); //record err statistics
		} else {
			fprintf(stderr,"LOOP Detected: Level: %d: %s\n",node->level,node->fullname);
			WriteErr(node->client_fd,"LOOP Detected: Level: %d: %s\n",node->level,node->fullname);

			fprintf(stderr,"%*sLevel: %d: %s\n",3*node->level," ",node->level,node->realname);
			WriteErr(node->client_fd,"%*sLevel: %d: %s\n",3*node->level," ",node->level,node->realname);
		}// if print error msg

		//current = node->parent;
	   // pthread_rwlock_wrlock(&wr.wr_mutex);
		//if (err != 0)
		//	printf("%s",strerror(err));
		while (current->realname !=NULL){

			if (!node->data.flag.q){
				fprintf(stderr,"%*sLevel: %d: %s\n",3*node->level," ",\
						current->level,current->realname);
				WriteErr(node->client_fd,"%*sLevel: %d: %s\n",3*node->level," ",\
						current->level,current->realname);
			} else
				StatErr(node);

			if(strcmp(current->realname, node->realname) == 0)
				break;

			current = current->parent;

		}//endwhile
	 //pthread_rwlock_unlock(&wr.wr_mutex);

	}

	return ret;
}//LoopHd





void *tralloc(size_t size){


	void *ptr = malloc(size);
	if (ptr == NULL && size != 0){
		fprintf(stderr,"malloc of %ld error\n",(long)size);
		exit(EXIT_FAILURE);
	}
	memset(ptr,0,sizeof(*ptr));
	return ptr;
}



DIR_t *dirent_alloc(NODE_t *node){

	long name_max;
	DIR_t *buf;

	name_max = pathconf(node->realname, _PC_NAME_MAX);
	if (name_max < 0) {
		name_max = MAXLINE;
	}

	if ((buf = malloc(sizeof(DIR_t) + name_max + 1)) == NULL){
		//if (!data.flag.q)
		fprintf(stderr,"Cannot malloc for Dir entry \n");
	}
	return buf;
}



char *path_alloc(size_t *sizep){

	char *ptr;
	size_t pathmax;


#if defined(PATH_MAX)
	pathmax = PATH_MAX;
#else 
	if ((pathmax = pathconf(".", _PC_PATH_MAX)) == -1) {
		pathmax = 8192;
	}
#endif

	ptr = (char *)malloc(pathmax);

	if (sizep != NULL)
		*sizep = pathmax;


	return ptr;

}





char *fullpath(const char *fname, char *realname){

	char *memptr;
	size_t size, len, size1;
	char *ptr, *tmp, str[BUFSIZE],s[BUFSIZE],*rname;
	char *home;

	len = strlen(fname);


	memptr = path_alloc(&size);

	if (fname[0] == '/'){
		//check if we have enough mem for memptr if its absolute path;
		if (size <= len){
			size *= 2;
			if ((memptr = realloc(memptr,size)) == NULL)
				fprintf(stderr,"realloc error of fullpath\n");
		}
		strncpy(memptr,fname,strlen(fname)+1);

	} else if(fname[0] != '/' && (ptr = strrchr(fname,'/')) != NULL){

		strncpy(str,fname,strlen(fname)+1);
		str[ptr-fname] = '\0';
		ptr++;
		rname = path_alloc(&size1);

		if (str[0] == '~' && (home = getenv("HOME")))
			tmp = realpath(strcat(strcpy(s,home),str+1),rname);
		else
			tmp = realpath(str,rname);

		if (tmp == NULL){
			fprintf(stderr,"%s: %u: get realpath fail\n",__func__,__LINE__);
			exit(EXIT_FAILURE);
		}

		strncpy(memptr,tmp,strlen(tmp) + 1);
		strncat(memptr, "/", 1);
		strncat(memptr, ptr, strlen(ptr));
		free(rname);
	} else {

		if (realname == NULL){
			if ((memptr = getcwd(memptr,size)) == NULL){
				size *= 2;
				if ((memptr = realloc(memptr,size)) == NULL)
					fprintf(stderr,"realloc error of fullpath\n");
				getcwd(memptr,size);
			}
		} else {

			strncpy(memptr,realname,strlen(realname) + 1);

		}
		strncat(memptr, "/", 1);
		strncat(memptr, fname, strlen(fname));

	}

	return memptr;
}



