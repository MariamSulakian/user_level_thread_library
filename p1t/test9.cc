#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: testing thread library functions
ensuring libinit works correctly
ensuring lock gets reaquired after returning from wait
ensuring signal and broadcast work as intended
ensuring yield works without releasing lock */

unsigned int lock = 201;
unsigned int lock2 = 826;
unsigned int cv = 193;

void threadFunc(void* b){
	std::cout << "child thread acquiring lock" << std::endl;
	
	if(thread_lock(lock))
		std::cout << "child thread failed to aquire lock" << std::endl;	
	std::cout << "child thread running" << std::endl;
	
	std::cout << "child thread signaling" << std::endl;
	if(thread_signal(lock, cv))
		std::cout << "signal failed" << std::endl;

	std::cout << "child thread blocking" << std::endl;	
	if(thread_wait(lock, cv))
		std::cout << "wait failed" << std::endl;

	std::cout << "child thread broadcasting" << std::endl;
	if(thread_broadcast(lock, cv))
		std::cout << "broadcast failed" << std::endl;
	
	std::cout << "child thread yielding" << std::endl;
	if(thread_yield())
		std::cout << "thread yield failed" << std::endl;

	std::cout << "child thread running" << std::endl;
	std::cout << "child thread releasing lock" << std::endl;
	if(thread_unlock(lock)){
		std::cout << "child thread failed to release lock" << std::endl;
	}
}

void createThreads(void* a){
	thread_lock(lock);
	if(thread_create(threadFunc, (void*) "thread1"))
			std::cout << "thread create failed" << std::endl;
	else std::cout << "child thread created" << std::endl;
	
	std::cout << "parent blocking" << std::endl;
	if (thread_wait(lock, cv))
		std::cout << "parent failed to yield" << std::endl;
	
	if (thread_signal(lock, cv))
		std::cout << "parent failed to signal" << std::endl;
	else std::cout << "parent signals successfully" << std::endl;

	std::cout << "parent thread completed" << std::endl;
	thread_unlock(lock);
}

// for testing
int main(int argc, char *argv[]){
		if(thread_libinit(createThreads, (void *) "lib_created"))
				std::cout << "failed to create thread lib" << std::endl;
		return 0;
}