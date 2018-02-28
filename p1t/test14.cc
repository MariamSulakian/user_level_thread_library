#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: checks locks, signal, broadcast */

unsigned int lock = 2;
unsigned int lock2 = 62;
unsigned int lock3 = 291;
unsigned int cv = 594;

void threadFunc(void* b){
	thread_lock(lock2);
	std::cout << "child thread yields" << std::endl;
	if(thread_yield())
		std::cout << "ERROR: yield failed"<< std::endl;

	if(thread_signal(6, cv)){
		std::cout << "ERROR: attempting to signal unused lock" << std::endl;
	} else std::cout << "signaled" << std::endl;

	if(thread_broadcast(lock3, 81)){
		std::cout << "ERROR: attempting broadcast" << std::endl;
	} else std::cout << "broadcasted" << std::endl;
	
	std::cout << "child thread completes" << std::endl;
	thread_unlock(lock2);
}

void createThreads(void* a){
	thread_lock(lock);

	thread_create(threadFunc, (void*) "thread1");
	if(thread_lock(lock2))
		std::cout << "ERROR: master thread cannot aquire lock"<< std::endl;
	else std::cout << "master thread completes, unlocks" << std::endl;
	
	thread_unlock(lock2);
	if(thread_unlock(lock2)){
		std::cout << "Correct Error: cannot unlock twice"<< std::endl;	
	} else std::cout << "ERROR: master thread cannot aquire lock"<< std::endl;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}
