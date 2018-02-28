#include <iostream>
#include <cstdlib>		// for general purpose utilities	
#include "thread.h"


/* TEST: two threads total. if only thread running, should yield to CPU and keep running. */

void threadFunc(void* b){
	std::cout << "child thread yields" << std::endl;
	thread_yield();
	std::cout << "child thread completes" << std::endl;
}

void createThreads(void* a){
	std::cout << "master thread started"<< std::endl;
	std::cout << "child thread creating" << std::endl;
	thread_create(threadFunc, (void*) "thread");
	std::cout << "master thread yields" << std::endl;	
	thread_yield();
	std::cout << "master thread yields" << std::endl;
	thread_yield();
	std::cout << "master thread running" << std::endl;
	thread_yield();
	std::cout << "master thread running again, completes" << std::endl;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}