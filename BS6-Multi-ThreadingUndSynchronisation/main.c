#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include "queue.h"
#include "miniz.h"

#define MAX_PATH 1024
#define COMPILETHREADS 10

pthread_mutex_t lock;

typedef struct job{
	char *path;
	char *filename;

}Job;
static Queue jobQueue;
void *readPath(void *path);
void *compressFiles();

int main(int argc, char *argv[]){
	jobQueue = queue_create();
	printf("argv[1]: %s\n",argv[1]);
	int error;

	pthread_t reader;
	error = pthread_create(&reader,NULL,readPath(argv[1]),NULL);
	pthread_t threads[COMPILETHREADS];
	int threads_ids[COMPILETHREADS];
	error = pthread_mutexattr_init(&lock);
	if(error!=0)
		exit(EXIT_FAILURE);
	int i;
	for(i=0;i<COMPILETHREADS;i++){
		error = pthread_create(&threads[i],NULL,compressFiles,NULL);
		if(error!=0)
			exit(EXIT_FAILURE);
		threads_ids[i] = i;
	}

	for(i=0;i<COMPILETHREADS;i++){
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&lock);
	return 0;
}

void *readPath(void *path){
	char resolved_path[MAX_PATH];
	DIR *dir = NULL;
	struct dirent *dptr = NULL;
	char *dot = ".";
	char *dotdot = "..";
	if(realpath(path, resolved_path)){
		printf("resolved_path: %s\n", resolved_path);
		if((dir = opendir(resolved_path))){
			while((dptr = readdir(dir))){
				if(strcmp(dptr->d_name, dot) == 0 || strcmp(dptr->d_name, dotdot)==0){
					continue;
				}
				Job *j = (Job *) malloc(sizeof(Job));
				j->path = resolved_path;
				j->filename = dptr->d_name;
				queue_insert(jobQueue, j);
				printf("file: %s\n", j->filename);
			}
			closedir(dir);
		}
	}
	return NULL;

}
void *compressFiles(void *args){
	pthread_mutex_lock(&lock);
	printf("test\n");
	pthread_mutex_unlock(&lock);

	return NULL;
}
