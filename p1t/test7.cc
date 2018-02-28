#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: testing locks and yields. */

unsigned int lock = 2;

void threadFunc(void* b){	
	if(thread_lock(lock))
		std::cout << "child thread cannot acquire lock" << std::endl;
	else std::cout << "child thread successfully acquired lock" << std::endl;
	
	if(thread_yield())
		std::cout << "child thread cannot yield" << std::endl;
	else std::cout << "child thread yields" << std::endl;

	// makes sure that thread still has lock when it comes back from yield
	if(thread_unlock(lock))
		std::cout << "child thread did not release lock properly" << std::endl;
	else std::cout << "child thread completes" << std::endl;
}

void createThreads(void* a){
	if(thread_lock(lock))
		std::cout << "master thread cannot acquire lock" << std::endl;
	else std::cout << "master thread aquired lock"<< std::endl;
	
	if(thread_create(threadFunc, (void*) "thread1"))
		std::cout << "master thread cannot create child thread" << std::endl;
	else std::cout << "master thread created child thread" << std::endl;
	
	if(thread_yield())
		std::cout << "master thread cannot yield" << std::endl;
	else std::cout << "master thread yields" << std::endl;
	
	if(thread_unlock(lock))
		std::cout << "master thread did not release lock properly" << std::endl;
	else std::cout << "master thread completes" << std::endl;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}