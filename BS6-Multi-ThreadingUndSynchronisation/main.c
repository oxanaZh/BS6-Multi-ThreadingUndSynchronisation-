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
pthread_mutex_t starter_mutex;
pthread_cond_t starter_threshold_cv;

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
				if (strcmp(dptr->d_name, dot)
						== 0|| strcmp(dptr->d_name, dotdot) == 0 || strstr(dptr->d_name, compressed) != NULL) {
					continue;
				}
				Job *j = (Job *) malloc(sizeof(Job));

				if (!j) {
					exit(EXIT_FAILURE);
				}
				setJob(resolved_path, dptr->d_name, j);
				queue_insert(jobQueue, j);
				printf("file: %s\n", j->filename);
				sleep(1);
			}
			closedir(dir);
		}
	}
	printf("READER DONE\n");
	return NULL;
}
void *compressFiles(void *arg);
void *startComprThreads(void * arg);
void setJob(char *path, char *file, Job* job);

int main(int argc, char *argv[]) {
	jobQueue = queue_create();

	printf("argv[1]: %s\n", argv[1]);
	pthread_t reader;

	int error = pthread_create(&reader, NULL, readPath, (void*) argv[1]);
	if (error != 0)
		exit(EXIT_FAILURE);


	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&starter_mutex, NULL);
	pthread_cond_init(&starter_threshold_cv, NULL);

	pthread_t starter;
	error = pthread_create(&starter, NULL, startComprThreads, NULL);

	pthread_join(reader, NULL);

	pthread_mutex_lock(&starter_mutex);
	pthread_cond_wait(&starter_threshold_cv, &starter_mutex);
	pthread_mutex_unlock(&starter_mutex);

	printf("\n\n\nFinishedCompressing!!!");


	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&starter_mutex);
	pthread_cond_destroy(&starter_threshold_cv);
	return 0;
}

void *startComprThreads(void * arg) {
	pthread_t threads[COMPILETHREADS];
	int threads_ids[COMPILETHREADS];
	int i;
	int error;
	while (1) {
		for (i = 0; i < COMPILETHREADS; i++) {
			error = pthread_create(&threads[i], NULL, compressFiles, (void*)i);
			threads_ids[i]=i;
			if (error != 0)
				exit(EXIT_FAILURE);
		}
		for (i = 0; i < COMPILETHREADS; i++) {
			pthread_join(threads[i], NULL);
		}
		if (queue_empty(jobQueue) != 0) {
			pthread_mutex_lock(&starter_mutex);
			pthread_cond_signal(&starter_threshold_cv);
			pthread_mutex_unlock(&starter_mutex);
		}

	}
	return NULL;
}

void *compressFiles(void * arg) {
	int id = (int*) arg;

	pthread_mutex_lock(&lock);
	Job *j;

	if (queue_empty(jobQueue) == 0) {
		j = queue_head(jobQueue);
		queue_delete(jobQueue);
		printf(">> %s\n", j->filename);
	} else {
		pthread_mutex_unlock(&lock);
		printf("Thread %d hat keinen Auftrag\n",id);
		return NULL;
	}
	pthread_mutex_unlock(&lock);
	printf("Thread %d komprimiert\n",id);
	FILE * read;
	FILE * write;

	char * absoluteFilename = j->filename;
	printf("absoluteFilename %s\n", absoluteFilename);
	char * newfile = j->newfilename;
	printf("compress newfile %s\n", newfile);

	read = fopen(absoluteFilename, "r");
	write = fopen(newfile, "w+");

	if (!read || !write) {
		return NULL;
	}
	const int lineLength = 255;
	char buff[lineLength];
	Result *r = compress_string(buff);
	while (fgets(buff, lineLength, (FILE*) read)) {

		fputs(r->data, (FILE*) write);

	}
	free(r->data);
	free(r);
	fclose(read);
	fclose(write);
	free(j->filename);
	free(j->newfilename);
	free(j->path);
	free(j);
	sleep(3);
	return NULL;

}

void setJob(char *path, char *file, Job* job) {
	printf("setJob----- \n\n");
	char * compressed = ".compr";
	printf("path: %s\n", path);
	printf("filename: %s\n", file);


	job->path = (char *) malloc(MAX_PATH);
	strcpy(job->path, path);

	job->filename = (char *) malloc(MAX_PATH);
	strcpy(job->filename, job->path);

	strcat(job->filename, "/");
	strcat(job->filename, file);

	job->newfilename = (char *) malloc(MAX_PATH);
	strcpy(job->newfilename, job->filename);
	strcat(job->newfilename, compressed);

	printf("job path: %s\n", job->path);
	printf("job filename: %s\n", job->filename);
	printf("job newfilename: %s\n\n", job->newfilename);

}
