#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include "queue.h"
#include "miniz.h"


typedef struct job{
	char *path;
	char *filename;

}Job;
static Queue jobQueue;

int main(int argc, char *argv[]){
	jobQueue = queue_create();


	return 0;
}

int readPath(const char *path){
	DIR *dir = NULL;
	struct dirent *dptr = NULL;
	if(!(dir = opendir(path))){
		return 1;
	}
	while((dptr = readdir(dir))){


	}

	return 0;
}
