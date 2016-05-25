#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include "queue.h"
#include "miniz.h"
#include <stdio.h>

#define MAX_PATH 1024
#define COMPILETHREADS 10

pthread_mutex_t lock;

typedef struct job {
	char *path;
	char *filename;
	char *newfilename;

} Job;
static Queue jobQueue;

void* readPath(void* path) {
	char resolved_path[MAX_PATH];
	DIR *dir = NULL;
	struct dirent *dptr = NULL;
	char* dot = ".";
	char* dotdot = "..";
	char * compressed = ".compr";
	if (realpath((char*) path, resolved_path)) {
		printf("resolved_path: %s\n", resolved_path);
		if ((dir = opendir(resolved_path))) {
			while ((dptr = readdir(dir))) {
				if (strcmp(dptr->d_name, dot) == 0 || strcmp(dptr->d_name, dotdot) == 0 || strstr(dptr->d_name, compressed) != NULL){
					continue;
				}
				Job *j = (Job *) malloc(sizeof(Job));

				if (!j) {
					exit(EXIT_FAILURE);
				}
				setJob(resolved_path, dptr->d_name, j);
				queue_insert(jobQueue, j);
				printf("file: %s\n", j->filename);
			}
			closedir(dir);
		}
	}
	return NULL;
}
void *compileFiles(void *arg);
void setJob(char *path, char *file, Job* job);

int main(int argc, char *argv[]) {
	jobQueue = queue_create();
	printf("argv[1]: %s\n", argv[1]);

	pthread_t reader;
	//int error = pthread_create(&reader, NULL, readPath, (void*) argv[1]);
	//pthread_join(reader, NULL);
	int error = 0;
	readPath(argv[1]);
	if (error != 0)
		exit(EXIT_FAILURE);

	pthread_t threads[COMPILETHREADS];

	int threads_ids[COMPILETHREADS];

	pthread_mutex_init(&lock, NULL);

	int i;

	for (i = 0; i < COMPILETHREADS; i++) {
		error = pthread_create(&threads[i], NULL, compileFiles, NULL);
		if (error != 0)
			exit(EXIT_FAILURE);
		threads_ids[i] = i;
	}
	//pthread_join(reader, NULL);
	for (i = 0; i < COMPILETHREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	printf("finished");
	pthread_mutex_destroy(&lock);

	return 0;
}

void *compileFiles(void * arg) {
	pthread_mutex_lock(&lock);
	Job *j;



	if (queue_empty(jobQueue) == 0) {
		j = queue_head(jobQueue);
		queue_delete(jobQueue);
		printf(">> %s\n", j->filename);
	} else {
		printf("finished");
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	pthread_mutex_unlock(&lock);
	FILE * read;
	FILE * write;

	char * absoluteFilename = j->filename;
	printf("absoluteFilename %s\n",absoluteFilename);
	char * newfile = j->newfilename;
	printf("compress newfile %s\n",newfile);

	read = fopen (absoluteFilename, "r");
	write =fopen (newfile, "w+");

	if(!read || !write){
		return NULL;
	}
	const int lineLength = 255;
	char buff[lineLength];
	Result *r = compress_string(buff);
	while(fgets(buff, lineLength, (FILE*)read)){

		fputs(r->data,(FILE*)write );

	}
	free(r->data);
			free(r);
	fclose(read);
	fclose(write);

	return NULL;

}

void setJob(char *path, char *file, Job* job){
	printf("setJob----- \n\n");
	char * compressed = ".compr";
	printf("path: %s\n",path);
	printf("filename: %s\n",file);
	printf("job path: %s\n",job->path);

	job->path = (char *) malloc(MAX_PATH);
	strcpy(job->path, path);

	job->filename = (char *) malloc(MAX_PATH);
	strcpy(job->filename,job->path);

	strcat(job->filename,"/");
	strcat(job->filename,file);

	job->newfilename = (char *) malloc(MAX_PATH);
	strcpy(job->newfilename,job->filename);
	strcat(job->newfilename,compressed);

	printf("job path: %s\n",job->path);
	printf("job filename: %s\n",job->filename);
	printf("job newfilename: %s\n\n",job->newfilename);


}
