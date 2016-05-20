#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include "queue.h"
#include "miniz.h"

#define MAX_PATH 1024

typedef struct job{
	char *path;
	char *filename;

}Job;
static Queue jobQueue;

int main(int argc, char *argv[]){
	jobQueue = queue_create();


	return 0;
}

void *readPath(const char *path){
	char resolved_path[MAX_PATH];
	DIR *dir = NULL;
	struct dirent *dptr = NULL;
	if(realpath(path, resolved_path)){
		if(!(dir = opendir(resolved_path))){
			while((dptr = readdir(dir))){
				dptr->d_name;

			}
		}
	}
	return NULL;

}
