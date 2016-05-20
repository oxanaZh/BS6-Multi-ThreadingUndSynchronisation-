#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include "queue.h"
#include "miniz.h"

#define MAX_PATH 1024

typedef struct job{
	char *path;
	char *filename;

}Job;
static Queue jobQueue;
void *readPath(char *path);

int main(int argc, char *argv[]){
	jobQueue = queue_create();
	printf("argv[1]: %s\n", argv[1]);
	readPath(argv[1]);
	return 0;
}

void *readPath(char *path){
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
