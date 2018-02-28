/* start by implementing thread_libinit, thread_create, and thread_yield
don't worry first about disbaling and enabling interrupts
after you get that system working, implement the monitor functions
finally add calls to interrupt_disable() and interrupt_enable() to ensure that the library works with arbitrary yield points
*/
#include <map>
#include <cassert>
#include "interrupt.h"
#include "thread.h"
#include <queue>
#include <deque>
#include <string>
#include <ucontext.h>
#include <iostream>
#include <cstdlib>      // for general purpose utilities

//#define MAX_QUEUE_SIZE 100

typedef struct _thread {
   char* stack;
   ucontext_t* ucontext_ptr;
   std::string status;
} thread;

typedef struct _lock {
   thread* owner;
   std::deque<thread*> waiting;
} Lock;

static bool initialized = false;
static int thread_count = 0;
static thread* curr_thread;
static ucontext_t *temp_context;
static std::deque<thread*> ready_queue;
static std::map<int, Lock*> lock_map;
static std::map< std::pair<int, int>, std::deque<thread*> > conditions_map;
typedef void (*thread_startfunc_t)(void *);

void deleteThread(thread* thread){
   delete[] curr_thread->stack;
   curr_thread->ucontext_ptr->uc_stack.ss_sp = NULL;
   curr_thread->ucontext_ptr->uc_stack.ss_size = 0;
   curr_thread->ucontext_ptr->uc_stack.ss_flags = 0;
   curr_thread->ucontext_ptr->uc_link = NULL;
   delete curr_thread->ucontext_ptr;
   delete curr_thread;
   curr_thread = NULL;
   thread_count = thread_count-1;
}

static int start(thread_startfunc_t func, void *arg) {  
      interrupt_enable();
      func(arg);
      interrupt_disable();    
      curr_thread->status = "COMPLETED";
      swapcontext(curr_thread->ucontext_ptr, temp_context);
      return 0;
}

int thread_libinit(thread_startfunc_t func, void *arg) {
   interrupt_disable();
   if (initialized) {
      //interrupt_enable();
      return -1;
   }
   initialized = true;
   thread* master_thread;
   try{ 
      master_thread= new thread;
      master_thread->ucontext_ptr= new ucontext_t;
      getcontext(master_thread->ucontext_ptr);
      master_thread->stack = new char [STACK_SIZE]; 
      master_thread->ucontext_ptr->uc_stack.ss_sp = master_thread->stack;
      master_thread->ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
      master_thread->ucontext_ptr->uc_stack.ss_flags = 0;
      master_thread->ucontext_ptr->uc_link = NULL;
      master_thread->status = "RUNNING";
      makecontext(master_thread->ucontext_ptr, (void (*)()) start, 2, func, arg);
      ready_queue.push_back(master_thread);
   }
   catch (std::exception e){
      delete master_thread->ucontext_ptr;
      delete[] master_thread->stack;
      delete master_thread;
      //interrupt_enable();
      return -1;
   }
   curr_thread = ready_queue.front();
   curr_thread->status = "RUNNING";
   ready_queue.pop_front();
   try{
      temp_context = new ucontext_t;
   } 
   catch (std::exception e) {
      delete temp_context;
      //interrupt_enable(); 
      return -1;
   }  
   getcontext(temp_context);
   swapcontext(temp_context, curr_thread->ucontext_ptr);
   while (!ready_queue.empty()) {
      if (curr_thread->status == "COMPLETED") {
         deleteThread(curr_thread);
      }
      curr_thread = ready_queue.front();
      curr_thread->status = "RUNNING";
      ready_queue.pop_front();
      swapcontext(temp_context, curr_thread->ucontext_ptr);
   }
   if(curr_thread != NULL){
      deleteThread(curr_thread);
   }     
   std::cout << "Thread library exiting.\n"; 
   exit(0);
}

int thread_create(thread_startfunc_t func, void *arg) {
   interrupt_disable();
   if (!initialized) {
      interrupt_enable();
      return -1;
   }   
   thread* newThread;  
   try {
      newThread = new thread;
      newThread->ucontext_ptr = new ucontext_t;
      getcontext(newThread->ucontext_ptr);
      newThread->stack = new char [STACK_SIZE]; 
      newThread->ucontext_ptr->uc_stack.ss_sp = newThread->stack;
      newThread->ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
      newThread->ucontext_ptr->uc_stack.ss_flags = 0;
      newThread->ucontext_ptr->uc_link = NULL;
      makecontext(newThread->ucontext_ptr, (void (*)()) start, 2, func, arg);
      newThread->status = "READY";
      ready_queue.push_back(newThread); //add thread to ready queue
      thread_count++;  
   }
   catch(std::exception e) {
      delete newThread->ucontext_ptr;
      delete[] newThread->stack;
      delete newThread;
      interrupt_enable();
      return -1;
   }
   interrupt_enable(); 
   return 0; 
}

int thread_yield(void) {
   interrupt_disable();
   try {
      if (!initialized) {
         interrupt_enable();
         return -1;
      }   
      curr_thread->status = "READY";
      ready_queue.push_back(curr_thread);
      swapcontext(curr_thread->ucontext_ptr, temp_context);
      interrupt_enable();    
      return 0;
   } 
   catch(std::exception e) {
      interrupt_enable();
      return -1;
   }
}

int thread_lock(unsigned int lock) {
      interrupt_disable();
      if (!initialized) {
         interrupt_enable();
         return -1;
      }   
      if (lock_map.find(lock)==lock_map.end()){ //if lock not in map, acquire it and add it 
         Lock* lock_struct;
         try {
            lock_struct = new Lock;
            lock_struct->owner = curr_thread;
            lock_map[lock] = lock_struct;
         } 
         catch(std::exception e) {
            delete lock_struct;
            interrupt_enable();
            return -1;
         }
      }
      else if (lock_map.find(lock)!=lock_map.end()) {
         if(lock_map[lock]->owner==NULL){ //if lock in map and doesn't have an owner
            lock_map[lock]->owner = curr_thread;
         }
         else if(lock_map[lock]->owner==curr_thread){ //error if thread already holds the lock
            interrupt_enable();
            return -1;
         }
         else { //if lock in map but diff thread has it
            curr_thread->status = "BLOCKED";
            lock_map[lock]->waiting.push_back(curr_thread);
            swapcontext(curr_thread->ucontext_ptr, temp_context);
            lock_map[lock]->owner = curr_thread;
         }
      }
      //std::cout << "Lock aquired.\n";
      interrupt_enable();    
      return 0;
}
int thread_unlock(unsigned int lock) {
   interrupt_disable();
   try {
      if (!initialized) {
         interrupt_enable();
         return -1;
      }   
      if (lock_map.find(lock)==lock_map.end()){ //error if try to unlock a lock that was never locked 
         interrupt_enable(); 
         //std::cout << "Unlock 1.\n";
         return -1;  
      }
      if(lock_map[lock]->owner == NULL){ //error if try to unlock a lock with no owner
      		interrupt_enable();
      		//std::cout << "Unlock 2.\n";
      		return -1;
      	}
      if(lock_map[lock]->owner != curr_thread){ //error if try to unlock lock with different owner
      		interrupt_enable();
      		//std::cout << "Unlock 3.\n";
      		return -1;
      }
      if(lock_map[lock]->owner==curr_thread){ //if current thread is holding lock 
         lock_map[lock]->owner = NULL;
         if (!lock_map[lock]->waiting.empty()) {
            lock_map[lock]->waiting.front()->status = "READY";
            ready_queue.push_back(lock_map[lock]->waiting.front());
            lock_map[lock]->waiting.pop_front();
         }
      }
      interrupt_enable();    
      return 0;
   }
   catch(std::exception e) {
      interrupt_enable();
      return -1;
   }
}

int thread_unlock_for_wait(unsigned int lock) {
   try {
      if (!initialized) {
         return -1;
      }   
      if (lock_map.find(lock)==lock_map.end()){ //error if try to unlock a lock that was never locked  
         return -1;  
      }
      if(lock_map[lock]->owner == NULL){ //error if try to unlock a lock with no owner
      		return -1;
      	}
      if(lock_map[lock]->owner != curr_thread){ //error if try to unlock lock with different owner
      		return -1;
      }
      if(lock_map[lock]->owner==curr_thread){ //if current thread is holding lock 
         lock_map[lock]->owner = NULL;
         if (!lock_map[lock]->waiting.empty()) {
            lock_map[lock]->waiting.front()->status = "READY";
            ready_queue.push_back(lock_map[lock]->waiting.front());
            lock_map[lock]->waiting.pop_front();
         }
      }    
      return 0;
   }
   catch(std::exception e) {
      return -1;
   }
}

int thread_wait(unsigned int lock, unsigned int cond) {
   interrupt_disable();
   try {
      if (!initialized) {
         interrupt_enable();
         return -1;
      }  
      if (thread_unlock_for_wait(lock)) {
      	 interrupt_enable();
         return -1;
      }
      curr_thread->status = "BLOCKED";
      std::pair <int,int> lock_cond = std::make_pair (lock, cond);
      if (conditions_map.find(lock_cond)==conditions_map.end()){ //if not already in conditions_map
         std::deque<thread*> wait_queue;
         wait_queue.push_back(curr_thread);
         conditions_map[lock_cond]=wait_queue;       
      }
      else {
         conditions_map[lock_cond].push_back(curr_thread);
      }
      swapcontext(curr_thread->ucontext_ptr, temp_context);
      interrupt_enable();   
      thread_lock(lock); 
      return 0;
   }
   catch(std::exception e) {
      interrupt_enable();
      return -1;
   }
}

int thread_signal(unsigned int lock, unsigned int cond) {
   interrupt_disable();
   try {
      if (!initialized) {
         interrupt_enable();
         return -1;
      }
      std::pair <int,int> lock_cond = std::make_pair (lock, cond);
      if (conditions_map.find(lock_cond)!=conditions_map.end()){ //if in conditions_map
         if (!conditions_map[lock_cond].empty()) { //if it's in the conditions map and not empty, pop first thread off to ready queue
            conditions_map[lock_cond].front()->status = "READY";
            ready_queue.push_back(conditions_map[lock_cond].front());
            conditions_map[lock_cond].pop_front();
         }
      }
      interrupt_enable();    
      return 0;
   }
   catch(std::exception e) {
      interrupt_enable();
      return -1;
   }
}

int thread_broadcast(unsigned int lock, unsigned int cond) {
   interrupt_disable(); 
   try {
      if (!initialized) {
         interrupt_enable();
         return -1;
      }       
      std::pair <int,int> lock_cond = std::make_pair (lock, cond);
      if (conditions_map.find(lock_cond)!=conditions_map.end()){ //if in conditions_map
         while (!conditions_map[lock_cond].empty()) { //if it's in the conditions map and not empty, pop first thread off to ready queue
            conditions_map[lock_cond].front()->status = "READY";
            ready_queue.push_back(conditions_map[lock_cond].front());
            conditions_map[lock_cond].pop_front();
         }
      }
      interrupt_enable();    
      return 0;
   }
   catch(std::exception e) {
      interrupt_enable();
      return -1;
   }
}
