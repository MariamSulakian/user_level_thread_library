#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: signal and broadcast without holding lock */

unsigned int lock = 201;
unsigned int cv = 193;

void threadFunc(void* b){
	std::cout << "child thread running" << std::endl;
	if(thread_signal(lock, cv))
			std::cout << "signal failed" << std::endl;
	else std::cout << "child thread signals without holding lock" << std::endl;
	thread_yield();
	if(thread_broadcast(lock, cv))
		std::cout << "broadcast failed" << std::endl;
	else std::cout << "child thread broadcasts without holding lock" << std::endl;
}

void createThreads(void* a){
	thread_lock(lock);
	std::cout << "parent thread creates child thread" << std::endl;
	if(thread_create(threadFunc, (void*) "thread1"))
			std::cout << "thread create failed" << std::endl;
	std::cout << "parent thread yields" << std::endl;
	thread_yield();
	// std::cout << "parent thread yields" << std::endl;
	// thread_yield();
	std::cout << "parent thread completed without releasing lock" << std::endl;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}