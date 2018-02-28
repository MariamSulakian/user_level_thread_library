#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: deadlock. both parent and child threads blocking, never signaled. process exits successfully with both threads still in cvs. */

unsigned int lock = 1995;
unsigned int cv = 10;
unsigned int lock2 = 1000;
unsigned int cv2 = 55;

void threadFunc(void* b){
	int a = 0;
	std::cout << "child thread blocking" << std::endl;
	thread_wait(lock2, cv2);
	std::cout << "child thread completes" << std::endl;
}

void createThreads(void* a){
	thread_lock(lock);
	if(thread_create(threadFunc, (void*) "thread")){
			std::cout << "ERROR: thread create failed" << std::endl;
			exit(1);
		}
	else std::cout << "child thread created" << std::endl;
	std::cout << "parent thread blocking" << std::endl;
	thread_wait(lock, cv);

	std::cout << "Process complete." << std::endl;
	thread_unlock(lock);
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}