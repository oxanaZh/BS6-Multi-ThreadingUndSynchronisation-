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
				j->filename = dptr->d_name;
				queue_insert(jobQueue, j);
				printf("file: %s\n", j->filename);
			}
			closedir(dir);
		}
	}
	return NULL;
}
void *compileFiles() {
	Job *j;

	pthread_mutex_lock(&lock);

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
	/*
	 //Result *r = compress_string(j->filename);
	 char * newfile = j->filename;
	 char * compressed = ".compr";
	 strcat(newfile,compressed);
	 j->filename = newfile;

	 compressed = strcat(j->path,newfile);
	 printf("%s",compressed);
	 FILE *fp;
	 fp = fopen(compressed, "w+");
	 fprintf(fp, "This is testing for fprintf...\n");
	 fclose(fp);
	 */
	//free(r->data);
	//free(r);
	//free(j);

	return NULL;

}
