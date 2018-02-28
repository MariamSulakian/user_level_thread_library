#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: error if thread tries to release lock it doesn't have. */

unsigned int lock = 2;

void threadFunc(void* b){
	std::cout << "child thread running: " << b << std::endl;
	if(thread_lock(lock))
		std::cout << "child thread cannot acquire lock" << std::endl;
	std::cout << "child thread successfully acquires lock" << std::endl;
	if (thread_unlock(3)) {
		std::cout << "SUCCESSFUL ERROR: child thread cannot release lock it doesn't have" << std::endl;
		//thread_unlock(lock);
	}
	else std::cout << "child thread incorrectly releases lock it doesn't have" << std::endl;
}

void createThreads(void* a){
	thread_lock(lock);
	std::cout << "master thread creates child threads 1"<< std::endl;
	thread_create(threadFunc, (void*) "thread1");
	//std::cout << "master thread creates child threads 2"<< std::endl;
	//thread_create(threadFunc, (void*) "thread2");
	//std::cout << "master thread yields" << std::endl;
	//thread_yield();
	std::cout << "master thread completes" << std::endl;
	thread_unlock(lock);
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}
