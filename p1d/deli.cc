#include <cstdlib>		// for general purpose utilities	
#include <iostream>		// to read standard input/output streams
#include <queue>		// for queue function
#include <vector>		// to alloy for dynamically allocated memory
#include <fstream>
#include <sstream>
#include "thread.h"

// global variables
int numCashiers;			// deli contains variable # of cashiers, shared state variable
int sandwichMaker = 1;		// always 1 sandwich maker
int boardSize;				// cashiers and maker communicate via fixed size cork board
int max_orders;				// board can hold max # of order requests

unsigned int lock = 1000000;
unsigned int cashierCV = 1000001;		// condition variable #1
unsigned int makerCV = 1000002;		// condition variable #2

// struct holding cashier information
typedef struct _cashierStruct_t {	// type defined as "_cashierStruct_t"
	std::queue<int> orders; 		// maker must know what to prepare, queue uses FIFO
	int cashierID; 					// maker must know which cashier put order in
} cashierStruct_t;					// local variable defined as "cashierStruct_t" 

// function declarations
void maker(void *vp);				// func for initializing maker + cashier threads
void cashier(void *cashierStruct);
std::queue<int>* cashierQueue;

// struct for each order
typedef struct _boardOfOrders_t {
	int cashierID;					// maker needs to know cashier associated with order
	int sandwichID;					// sandwichID lets maker prioritize order in which to make sandwiches
} boardOfOrders_t;

std::vector<boardOfOrders_t> corkBoard;		// a vector allows for dynamic allocation of sequence of objects (boardOfOrders_t)

/* create specified num of cashier threads to read in sandwich orders from file
create one thread for maker
from 'thread.h': int thread_create(thread_startfunc_t func, void *arg); */

void createDeli(void* a){	// void* vp: a void pointer can point to a variable of any date type
	/* start_preemptions() called after thread_libinit(), after thread system is initialized
	if not called, no interrupts will be generated. */
	//start_preemptions(true, true, 0);

	int start = 0;

	// creating cahsier threads
	while(start < numCashiers){
		cashierStruct_t* cashierStruct = new cashierStruct_t;
		cashierStruct->cashierID = start;
		cashierStruct->orders = cashierQueue[start];	// series of sandwich requests specified in input file
		thread_create(cashier, (void *) cashierStruct);	// creates a new thread, will call func pointed to by cashier(void *cashierStruct) and pass in cashierStruct
		start++;
	}

	// creating maker thread
	thread_create(maker, (void *) "makerThread");
}

// to check if board is full for maker
bool boardNotFull(){
	if (corkBoard.size() < max_orders) return true;
	return false;
}

// maker thread
void maker(void *vp){
	/* maker does not create sandwiches in FIFO order
	thread chooses next sandwich based on similarity -> reduce latency
	next sandwich maker should make is the one with closes # on board to last sandwich
	last sandwich initialized as -1 */

	/* this thread should only handle a request when board is at max capacity
	capacity depends on active # of cashiers
	max_orders when 1) when at least max_orders cashier threads are alive, largest # requests max_orders OR 
	2) when fewer than max_orders cashier threads alive, then largest # requests is # living cashier threads
	*/

	thread_lock(lock);

	int lastSandwich = -1;								// maker initializes last sandwich with -1

	while (numCashiers > 0 || corkBoard.size() > 0) {

		// if room on board
		while (boardNotFull()){							// maker waits for full board
			thread_wait(lock, makerCV);					// thread blocks on CV 2
		}
		if (numCashiers == 0) {
			break;
		}

		// find most similar sandwich to last sandwich made
		int mostSimilar = 1000;				// we simulate a list of 1,000 sandwiches
		int mostSimilarInd = 0;				// each sandwich assigned to unique #, 0-999
		int similarityScore;
		int start = 0;
		while (start < corkBoard.size()){
			similarityScore = corkBoard[start].sandwichID - lastSandwich;
			if (mostSimilar > abs(similarityScore)){
				mostSimilarInd = start;
				mostSimilar = abs(similarityScore);
			}
			start++;
		}

		lastSandwich = corkBoard[mostSimilarInd].sandwichID;			// last made sandwichID
		int lastMadeCashier = corkBoard[mostSimilarInd].cashierID;		// cashier of last made sandwich
		std::cout << "READY: cashier " << lastMadeCashier << " sandwich " << lastSandwich << std::endl;

		corkBoard.erase(corkBoard.begin() + mostSimilarInd);			// delete sandwich from board		
		thread_broadcast(lock, cashierCV);								// signal that board is open
		thread_signal(lock, lastMadeCashier);							// signal to cashier that sandwich is ready	
	}
	thread_unlock(lock);
	return;
}

void noSandwichesLeft(){
	numCashiers = numCashiers - 1;
	if (numCashiers < max_orders) 
		max_orders = numCashiers;
	if (!boardNotFull()) 
		thread_signal(lock, makerCV);
}

// cashier thread
void cashier(void *cashierStruct){
	thread_lock(lock);

	std::queue<int> orderOfSandwiches = ((cashierStruct_t*) cashierStruct)->orders; // cashiers orders must be completed in FIFO
	int cashierID = ((cashierStruct_t*)cashierStruct)->cashierID;
	
	while(!orderOfSandwiches.empty()) {					// keep thread on until no more sandwiches

		// check if cashier can add to board
		while (!boardNotFull()) {						// board full, cashier cannot add
			thread_wait(lock, cashierCV);				// if cashier cannot add, wait for maker to complete orders
		}

		/* if cashier can add to board, then add
		cashier thread complete when all orders fulfilled */
		boardOfOrders_t sandwichOrder;
		sandwichOrder.cashierID = cashierID;
		sandwichOrder.sandwichID = orderOfSandwiches.front(); 	// front finds head of queue
		orderOfSandwiches.pop();								// remove next element in queue

		corkBoard.push_back(sandwichOrder);						// new order to corkboard: new element at end of vector
		int sandwichCashier = sandwichOrder.cashierID;
		int sandwichID = sandwichOrder.sandwichID;
		std::cout << "POSTED: cashier " << sandwichCashier << " sandwich " << sandwichID << std::endl;

		if(!boardNotFull())
			thread_signal(lock, makerCV);						// signal maker for new order
		thread_wait(lock, cashierID);							// wait for maker to signal for next order

	}
	// if no more sandwiches left
	noSandwichesLeft();
	thread_unlock(lock); 
}

// for testing
int main(int argc, char *argv[]){

	if (argc<=2) return 0;									// check condition for # of args

	else {
		// setting values to global variables
		max_orders = atoi(argv[1]);  						// arg 1: max # orders board can hold, atoi converts string to int
		numCashiers = argc - 2;								/* arg 2: # input files, 1 input file per cashier, input file for cashier c is argv[c+2] where 0 <= c < numCashiers
											  			   argc is # strings pointed to by argv, so 1 + # arguments. First argument is max orders for board so we must do argc-2. */
		
		if (numCashiers < max_orders) max_orders = numCashiers;
		
		cashierQueue = new std::queue<int> [numCashiers]; 	// to keep track of cashiers using FIFO policy

		// file input for each cashier contains that cashier's series of sandwich orders
		// each line of input file specifies requested sandwich (0-999)
		// use ifstream to open each file read-only 
	
		int start = 0;
		while (start < numCashiers){
			std::queue<int> orders;
			std::ifstream file(argv[start+2]);
			std::string line;
			
			// make cashier queue with orders
			while (getline(file, line)){
				int order;
				std::istringstream stream1(line);
				stream1 >> order;
				orders.push(order);
			}
			cashierQueue[start] = orders;
			start = start + 1;
		}
		/* creates and runs the first thread, to be called only once, will not return to calling func
		transfers control to createDeli(void* vp) and passes in the arg */
		thread_libinit(createDeli, (void *) "p1d: deli");
	}
	return 0;
}