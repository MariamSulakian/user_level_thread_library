#include <iostream>
#include "thread.h"


/* TEST: thread functions called before libinit */

void createThreads(void* a){
	std::cout << "All Done: libinit has been called." << std::endl;
}

void threadFunc(void* b){
	int a = 1;
}

// for testing
int main(int argc, char *argv[]){
		int lock = 1;
		int cv = 2;

		if(thread_create(threadFunc, (void *) "creating_thread"))
			std::cout << "CORRECT: thread_create fails" << std::endl;

		if(thread_broadcast(lock, cv))
			std::cout << "CORRECT: thread_broadcast fails" << std::endl;

		if(thread_wait(lock, cv))
			std::cout << "CORRECT: thread_wait fails" << std::endl;

		if(thread_lock(lock))
			std::cout << "CORRECT: thread_lock fails" << std::endl;

		if(thread_yield())
			std::cout << "CORRECT: thread_yield fails" << std::endl;

		thread_libinit(createThreads, (void *) "libinit_1");
		return 0;
}