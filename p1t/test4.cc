#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: out of memory, cannot handle num thread requests */

unsigned int first = 59;
unsigned int second = 105;
int threadnum = 0;

void threadFunc(void* b){
	thread_lock(105);
	threadnum++;
	std::cout << "thread created: " << threadnum << std::endl;	
	thread_unlock(105);
}

void createThreads(void* a){
	thread_lock(first);
	int num = 0;
	while(num < 500){
		if(thread_create(threadFunc, (void*) "thread")){
			std::cout << "ERROR: Process takes too much memory" << std::endl;
		}
		num++;
	}
	std::cout << "COMPLETE: All threads created." << std::endl;
	thread_unlock(first);
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}