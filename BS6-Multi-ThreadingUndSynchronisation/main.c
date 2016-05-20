#include "queue.h"
#include "miniz.h"
#include <pthread.h>

typedef struct job{
	char *dir;


}Job;
static Queue jobQueue;

int main(int argc, char *argv[]){
	jobQueue = queue_create();


	return 0;
}
