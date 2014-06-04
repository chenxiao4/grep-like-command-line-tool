#include "commonincludes.h"

extern THREADS_t threads_info;
extern AGENT_INFO_t  ainfo;
extern AGENT_WR_t     wr;


void DecentDir(char *fname,NODE_t *node){

	STAT_t statbuf;
	char *fullname;
	NODE_t *newnode;

	fullname = fullpath(fname,NULL);


	if (ainfo.nodetype == 1 && strcmp(fname,"-") == 0){
		/* it is the client
		 * and read from the stdin
		 */
		DecentFile(fname,node,0);
		return;
	}


	if (lstat(fname,&statbuf) < 0){
		RepFileErr(fname,node);
		return;
	}


	/*
	 * if it is smylink
	 */

	if (S_ISLNK(statbuf.st_mode)){
		pthread_mutex_lock(&(node->node_mutex));
		node->statcs.symlink++;
		pthread_mutex_unlock(&(node->node_mutex));
	}

	/*
	 *  level 0 always follow symlink
	 */
	if (stat(fname,&statbuf) < 0){
		RepFileErr(fname,node);
		return;
	}

	/*
	 *  if it is a DIR
	 */
	if (S_ISDIR(statbuf.st_mode)){
		if(node->data.num.d == 0){
			if (!node->data.flag.q){
				fprintf(stderr,"Level: 0 : %s exceeds the max limit of decent depth %d\n",\
						fullname,node->data.num.d);
				WriteErr(node->client_fd,"Level: 0 : %s is exceeds the max limit of decent depth %d\n",\
						fullname,node->data.num.d);
			} else {//err msg
				StatErr(node);
				pthread_mutex_lock(&(node->node_mutex));
				node->statcs.dir_pruned++;
				pthread_mutex_unlock(&(node->node_mutex));
			}
		} else { //does not exceeds the limit
			pthread_mutex_lock(&(node->node_mutex));
			node->statcs.dir++;
			pthread_mutex_unlock(&(node->node_mutex));
			newnode = MakeNode(node,fname);
			MoveOn(newnode);
		}
		return;
	}// is dir

	/*
	 * if it is file
	 */
	DecentFile(fullname,node,1);

	pthread_mutex_lock(&(node->node_mutex));
	node->statcs.file++;
	pthread_mutex_unlock(&(node->node_mutex));

}//DecentFile




void MoveOn(NODE_t *node){

	pthread_mutex_lock(&threads_info.all_mutex);
	if ((int)threads_info.concur >= node->data.num.t){
		pthread_mutex_unlock(&threads_info.all_mutex);
		pthread_mutex_lock(&(node->node_mutex));
		node->statcs.thread_pruned++;
		pthread_mutex_unlock(&(node->node_mutex));
		Recursion(node);
	} else {
		if (pthread_create(&node->tid,NULL,IntoDir,(void *)node) != 0){
			pthread_mutex_unlock(&(threads_info.all_mutex));
			if (!node->data.flag.q){
				fprintf(stderr,"Cannot create thread!!\n");
				WriteErr(node->client_fd,"Cannot create thread!!\n");
			} else
				StatErr(node);
			Recursion(node);
		} else {
			pthread_mutex_unlock(&(threads_info.all_mutex));
			pthread_mutex_lock(&(node->node_mutex));
			node->statcs.thread++;
			pthread_mutex_unlock(&(node->node_mutex));

		}
	}

	return;
}//MoveOn




void *IntoDir(void *arg){

	NODE_t *node = (NODE_t *)arg;

	pthread_detach(node->tid);
	pthread_mutex_lock(&(threads_info.all_mutex));
//	pthread_rwlock_wrlock(&wr.wr_mutex);
	threads_info.concur++;
	if(threads_info.concur > threads_info.max_act)
		threads_info.max_act = threads_info.concur;
//	pthread_rwlock_unlock(&wr.wr_mutex);
	pthread_mutex_unlock(&(threads_info.all_mutex));

	Recursion(node);

	/*
	 * thread finish its work
	 */
	pthread_mutex_lock(&(threads_info.all_mutex));
//	pthread_rwlock_wrlock(&wr.wr_mutex);
	threads_info.concur--;
//	pthread_rwlock_unlock(&wr.wr_mutex);
	pthread_mutex_unlock(&(threads_info.all_mutex));

	pthread_exit(NULL);
	return NULL;
}//IntoDir




void Recursion(NODE_t *node){

	DIR *dp;
	DIR_t *entry, *buf;
	NODE_t *child;
	STAT_t statbuf;
	int depth,err;
	char *fullname;



	if ((dp = opendir(node->realname)) == NULL){

		RepFileErr(node->realname,node);
		CleanHd(node);
		return;
	}

	if ((buf = dirent_alloc(node)) == NULL){
		RepFileErr("dir entry table malloc",node);
		free(buf);
		CleanHd(node);
		return;
	}


	pthread_mutex_lock(&(node->parent->node_mutex));
	node->statcs.dir++;
	pthread_mutex_unlock(&(node->parent->node_mutex));
	/*
    printf("%s: %u: %s( %d - L%d )<<------%s( %d - L%d ): %lu\n",__func__,__LINE__,\
    node->parent->fullname,node->parent->nref,node->parent->level,\
    node->fullname,node->nref,node->level,(long unsigned int)pthread_self());
	 */
	node->dp = dp;
	depth = node->level;

	pthread_mutex_lock(&(node->node_mutex));
	if (depth > (int)node->statcs.maxdirdepth )
		node->statcs.maxdirdepth = depth;
	pthread_mutex_unlock(&(node->node_mutex));

	while ((err = readdir_r(dp,buf,&entry)) == 0 && (entry != NULL)){

		/*
		 * decide if ignore filename starting with .
		 */


		if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
			continue;

		if((!node->data.flag.a && entry->d_name[0] == '.')){
			pthread_mutex_lock(&(node->node_mutex));
			node->statcs.dotname++;
			pthread_mutex_unlock(&(node->node_mutex));
			continue;
		}


		fullname = fullpath(entry->d_name,node->realname);

		if(lstat(fullname,&statbuf) < 0){
			RepFileErr(fullname,node);
			free(fullname);
			continue;
		}//lstat

		/*
		 * for symlink
		 */

		if(S_ISLNK(statbuf.st_mode)){
			if (node->data.flag.f){//do not follow
				//	fprintf(stderr,"SymLink: %s\n",fullname);
				//	WriteErr(node->client_fd, "%s : %s\n", fullname, "SymLink");

				pthread_mutex_lock(&(node->node_mutex));
				node->statcs.symlink++;
				pthread_mutex_unlock(&(node->node_mutex));

				free(fullname);
				continue;
			} else {
				if (stat(fullname,&statbuf) < 0){
					RepFileErr(fullname,node);
					free(fullname);
					continue;
				}
			}//follow symlink
		}// is symlink


		if(S_ISDIR(statbuf.st_mode)){//is dir

			if (depth >= node->data.num.d){//do not decent
				if (!node->data.flag.q){//print error
					fprintf(stderr,"Level: %d : %s is exceeds the max limit of decent depth %d\n",\
							depth,fullname,node->data.num.d);
					WriteErr(node->client_fd,"Level: %d : %s is exceeds the max limit of decent depth %d\n",\
							depth,fullname,node->data.num.d);
					free(fullname);
				} else {
					StatErr(node);
					pthread_mutex_lock(&(node->node_mutex));
					node->statcs.dir_pruned++;
					pthread_mutex_unlock(&(node->node_mutex));
				}//count
				continue;
			} else {

				child = MakeNode(node,entry->d_name);

				if(LoopHd(child)){
					CleanHd(child);
					//printf("---> here\n");
					continue;
				}
/*
				pthread_mutex_lock(&(node->node_mutex));
				node->statcs.dir++;
				pthread_mutex_unlock(&(node->node_mutex));
*/
				MoveOn(child);
				free(fullname);
				continue;
			}//decent
		}//is dir

		DecentFile(fullname,node,1);
		pthread_mutex_lock(&(node->node_mutex));
		node->statcs.file++;
		pthread_mutex_unlock(&(node->node_mutex));
		free(fullname);
	}//end while

	free(buf);
	CleanHd(node);
}//Recursion



void DecentFile(const char *filename, NODE_t *parent, int io){

	char *buf;

	buf = (char *)tralloc(MAXLINE*sizeof(char));

	//io : 0->stdin; 1->file
	if (!io)
		process_search(io,buf,NULL,parent);
	else
		process_search(io,buf,(char *)filename,parent);

	free(buf);

	return;

}//end DecentFile




void DirClean(NODE_t *node){

	//int err;
	//pthread_rwlock_t lock;

	//pthread_rwlock_init(&lock,NULL);

	pthread_mutex_lock(&node->parent->node_mutex);
	/*
	if ((err = pthread_rwlock_wrlock(&lock)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}
	*/
	node->parent->statcs.file += node->statcs.file;
	node->parent->statcs.dir += node->statcs.dir;
	node->parent->statcs.loop += node->statcs.loop;
	node->parent->statcs.symlink += node->statcs.symlink;
	node->parent->statcs.dir_pruned += node->statcs.dir_pruned;
	node->parent->statcs.errs += node->statcs.errs;
	node->parent->statcs.lineread += node->statcs.lineread;
	node->parent->statcs.linematch += node->statcs.linematch;
	node->parent->statcs.bytesread += node->statcs.bytesread;

	if ( node->statcs.maxdirdepth > node->parent->statcs.maxdirdepth )
		node->parent->statcs.maxdirdepth = node->statcs.maxdirdepth;

	node->parent->statcs.thread += node->statcs.thread;
	node->parent->statcs.thread_pruned += node->statcs.thread_pruned;
	node->parent->statcs.dotname += node->statcs.dotname;

/*
	if ((err = pthread_rwlock_unlock(&lock)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}
*/
    pthread_mutex_unlock(&node->parent->node_mutex);

}



void PrintStats(FILE *fptr, NODE_t *root){


	if (fptr == NULL) return;
	if (ainfo.nodetype == 1)
		fprintf(fptr,"\n%5s***************** Search  Statistics ********************\n","*");
	fprintf(fptr, "%50s: %u\n","Total soft links ignored due to -f",\
			root->statcs.symlink);
	fprintf(fptr, "%50s: %u\n", "Total directories opened successfully",\
			root->statcs.dir);
	fprintf(fptr, "%50s: %u\n", "Total directory loops avoided",\
			root->statcs.loop);
	fprintf(fptr, "%50s: %u\n", "Total directory descents pruned by -d", \
			root->statcs.dir_pruned);
	fprintf(fptr, "%50s: %u\n", "Maximum directory descent depth", \
			root->statcs.maxdirdepth);
	fprintf(fptr, "%50s: %u\n", "Total dot names not ignored due to -a", \
			root->statcs.dotname);
	fprintf(fptr, "%50s: %u\n", "Total descent threads created", \
			root->statcs.thread);
	fprintf(fptr, "%50s: %u\n", "Total descent threads pruned by -t",\
			root->statcs.thread_pruned);
	fprintf(fptr, "%50s: %u\n", "Maximum simultaneously active descent threads", \
			root->statcs.thread_maxact);
	fprintf(fptr, "%50s: %u\n", "Total errors not printed due to -q",\
			root->statcs.errs);
	fprintf(fptr,"%50s: %u\n","Total lines matched",root->statcs.linematch);
	fprintf(fptr,"%50s: %u\n","Total lines read",root->statcs.lineread);
	fprintf(fptr,"%50s: %u\n", "Total files read",root->statcs.file);
	fprintf(fptr,"%50s: %u\n","Total bytes read",root->statcs.bytesread);
	fprintf(fptr,"%50s: %f\n","Total search time in seconds",\
			root->searchtime);
	fprintf(fptr,"%50s: %f\n","Total rate in megabytes per second",\
			root->prate);


}




void WriteErr(int fd,char *fmt,...){

	char *ptr;
	va_list ap;
	usint len;
	int err;

	if (ainfo.nodetype == 1)
		return;

	ptr = (char *)tralloc(BUFSIZE);

	va_start(ap,fmt);
	vsnprintf(ptr,BUFSIZE-1,fmt,ap);
	fflush(stdout);
	fflush(stderr);
	va_end(ap);


	if ((err = pthread_rwlock_wrlock(&wr.wr_mutex)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}

	len = strlen(ptr);
	if (our_send_message(fd,AGENT_ERROR,len+1,ptr) == -1){
		fprintf(stderr,"%d: agent send message type 5 fail\n",fd);
	}

	free(ptr);

	if ((err = pthread_rwlock_unlock(&wr.wr_mutex)) != 0){

		fprintf(stderr,"rw_lock: %s,%s,%u\n",strerror(err),__func__,__LINE__);
		return;
	}

}



void WriteLoop(int fd,char *fmt,...){


	char *ptr;
	va_list ap;
	usint len;

	if (ainfo.nodetype == 1)
		return;

	ptr = (char *)tralloc(BUFSIZE);

	va_start(ap,fmt);
	vsnprintf(ptr,BUFSIZE-1,fmt,ap);
	fflush(NULL);
	va_end(ap);


	len = strlen(ptr);
	if (our_send_message(fd,AGENT_ERROR,len+1,ptr) == -1){
		fprintf(stderr,"%d: agent send message type 5 fail\n",fd);
	}

	free(ptr);



}



void WritePerror(int fd, char *fname){

	char *buf;
	if (ainfo.nodetype == 1) // do not send if it is client
		return;
#ifndef __sun__
	buf = malloc(BUFSIZE*sizeof(char));
	strerror_r(errno,buf,BUFSIZE);
#else
	buf = strerror(errno);
#endif
	WriteErr(fd,"%s: %s\n",fname,buf);
#ifndef __sun__
	free(buf);
#endif

}

