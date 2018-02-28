#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: error if thread tries to acquire lock it already has. */

unsigned int lock = 2;

void threadFunc(void* b){
	std::cout << "child thread running" << std::endl;
	thread_lock(lock);
	std::cout << "child thread successfully acquires lock" << std::endl;
	//thread_unlock(lock);
	//thread_lock(lock);
	if(thread_lock(lock)) {
		std::cout << "SUCCESSFUL ERROR: child thread cannot acquire lock it already has" << std::endl;
	} else std::cout << "child thread incorrectly acquired lock it already has" << std::endl;
}

void createThreads(void* a){
	thread_lock(lock);
	std::cout << "master thread creates child threads 1"<< std::endl;
	thread_create(threadFunc, (void*) "thread1");
	//std::cout << "master thread creates child threads 2"<< std::endl;
	//thread_create(threadFunc, (void*) "thread2");
	std::cout << "master thread yields" << std::endl;
	thread_yield();
	std::cout << "master thread completes and releases lock" << std::endl;
	thread_unlock(lock);
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}
