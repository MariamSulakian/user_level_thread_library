#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: check if locks can be reaquired */

unsigned int lock = 2;
unsigned int lock2 = 62;
unsigned int lock3 = 291;
unsigned int cv = 594;

void threadFunc(void* b){
	thread_lock(lock2);
	std::cout << "child thread yields" << std::endl;
	thread_yield();
	std::cout << "child thread completes" << std::endl;
	thread_unlock(lock2);
}

void createThreads(void* a){
	thread_lock(lock);

	std::cout << "master thread creates child threads 1"<< std::endl;
	thread_create(threadFunc, (void*) "thread1");

	std::cout << "master thread unlocks and yields"<< std::endl;
	thread_unlock(lock);
	thread_yield();

	std::cout << "master thread tries to aquires child's lock"<< std::endl;
	if(thread_lock(lock2))
		std::cout << "ERROR: master thread cannot aquire lock"<< std::endl;
	else std::cout << "master thread completes, unlocks" << std::endl;
	thread_unlock(lock2);

	if(thread_unlock(lock2)){
		std::cout << "Correct Error: cannot unlock twice" << std::endl;
	} else std::cout << "ERROR: thread unlocked twice" << std::endl;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}
