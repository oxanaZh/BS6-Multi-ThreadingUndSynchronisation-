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

void *readPath(char *path);
void *compileFiles();

int main(int argc, char *argv[]) {
	jobQueue = queue_create();
	printf("argv[1]: %s\n", argv[1]);

	pthread_t reader;
	int error = pthread_create(&reader, NULL, readPath, (void*)argv[1]);
	pthread_join(reader,NULL);

	if (error != 0)
		exit(EXIT_FAILURE);

	pthread_t threads[COMPILETHREADS];

	int threads_ids[COMPILETHREADS];

	pthread_mutex_init(&lock,NULL);


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

void *readPath(char *path) {
	char resolved_path[MAX_PATH];
	DIR *dir = NULL;
	struct dirent *dptr = NULL;
	char *dot = ".";
	char *dotdot = "..";
	char * compressed = ".compr";
	if (realpath(path, resolved_path)) {
		printf("resolved_path: %s\n", resolved_path);
		if ((dir = opendir(resolved_path))) {
			while ((dptr = readdir(dir))) {
				if (strcmp(dptr->d_name, dot) == 0
						|| strcmp(dptr->d_name, dotdot) == 0) {
					continue;
				}
				Job *j = (Job *) malloc(sizeof(Job));

				if (!j)
					exit(EXIT_FAILURE);
				j->path = resolved_path;
				strcat(j->path,"/");

				j->filename = j->path;
				strcat(j->path,dptr->d_name);
				j->newfilename = j->filename;
				strcat(j->newfilename,compressed);
				queue_insert(jobQueue, j);
				printf("file: %s\n", j->filename);
			}
			closedir(dir);
		}
	}
	return NULL;
}
void *compileFiles() {
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
	printf("newfile %s\n",newfile);

	read = fopen (absoluteFilename, "r");
	write =fopen (newfile, "w+");

	if(!read || !write){
		return NULL;
	}
	const int lineLength = 255;
	char buff[lineLength];

	while(fgets(buff, lineLength, (FILE*)read)){
		Result *r = compress_string(buff);
		fputs(r->data,(FILE*)write );
		free(r->data);
		free(r);
	}
	fclose(read);
	fclose(write);

	return NULL;

}
