#include <iostream>
#include "thread.h"


/* TEST: libinit called multiple times */

int n = 1;

void threadFunc(void* vp);

int createThreads(void* vp){

	thread_create(threadFunc, (void *) "creating_thread_1");
	if(thread_libinit(threadFunc, (void *) "libinit_2")){
		std::cout << "ERROR: thread_libinit called twice" << std::endl;
		return -1;
	}
	thread_create(threadFunc, (void *) "creating_thread_2");
}

void threadFunc(void* vp){
	if (n!=1)
		std::cout << "ERROR: thread 2 created after thread_libinit called twice" << std::endl;
	n++;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit((thread_startfunc_t) createThreads, (void *) "libinit_1");
		return 0;
}