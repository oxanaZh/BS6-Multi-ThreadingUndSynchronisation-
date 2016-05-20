#include "queue.h"
#include "miniz.h"
#include <pthread.h>
#include <dirent.h>

typedef struct job{
	char *path;
	char *filename;

}Job;
static Queue jobQueue;

int main(int argc, char *argv[]){
	jobQueue = queue_create();


	return 0;
}

int read(const char *path){
	DIR *dir = opendir (path);
	if(!dir){
		return -1;
	}
}
