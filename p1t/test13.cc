#include <iostream>
#include <queue>
#include "thread.h"


/* TEST: wait + broadcast with one lock, two cvs */

unsigned int lock = 2;
unsigned int cv1 = 702;
unsigned int cv2 = 196;

int apples;
int eaters = 0;

void consumer(void* b){
	thread_lock(lock);

	eaters++;
	int threadNum = (long int) b;

	std::cout << "consumer thread " << b << " aquired lock" << std::endl;
	
	while (apples == 0){
		std::cout << "consumer thread blocks. no apples." << std::endl;
		thread_wait(lock, cv1);
		std::cout << "consumer thread " << b << " wakes up from block" << std::endl;
	}

	apples--;

	if (apples == 0) {
		std::cout << "consumer thread broadcasts to producer that there is room" << std::endl;
		thread_broadcast(lock, cv2);
	} else {
		std::cout << "consumer thread broadcasts there there is some space" << std::endl;
		thread_broadcast(lock, cv1);
	}

	eaters--;
	std::cout << "consumer thread " << b << " completes" << std::endl;
	thread_unlock(lock);
}

void producer(void* a){
	thread_lock(lock);

	std::cout << "producer thread aquired lock" << std::endl;
	
	while (eaters > 0){

		while (apples > 0){
			std::cout << "producer thread blocks. waits for consumer." << std::endl;
			thread_wait(lock, cv2);
		}

		apples++;

		std::cout << "producer thread broadcasts that there are apples to eat" << std::endl;
		thread_broadcast(lock, cv1);
	}
	
	std::cout << "producer thread completes" << std::endl;
	thread_unlock(lock);
}

void createThreads(void* a){
	apples = 0;
	thread_create(consumer, (void*) 1);
	thread_create(consumer, (void*) 2);
	thread_create(consumer, (void*) 3);
	thread_create(producer, (void*) "producer");
	std::cout << "master thread completes" << std::endl;
}

// for testing
int main(int argc, char *argv[]){
		thread_libinit(createThreads, (void *) "lib_created");
		return 0;
}
