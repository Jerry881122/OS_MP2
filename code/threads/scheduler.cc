// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{ 
    readyList = new List<Thread *>; 
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
	//cout << "Putting thread on ready list: " << thread->getName() << endl ;

    /*********************************************/    
    thread->setStatus(READY);
    // 此時設定 thread 的 status 由 JUST_CREATED to READY    
    /*********************************************/

    /*********************************************/
    readyList->Append(thread);
    // status 為 READY 的 thread 加入 readyList 中
    
    // cout << "In schduler::ReadyToRun \n\t";
    // cout << "Putting thread on ready list: " << thread->getName() << "\n\t" ;
    // Print();
    // cout << "\n";
    /*********************************************/
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (readyList->IsEmpty()) {
		return NULL;
    } else {
        // cout << "In Scheduler::FindNextToRun()" << endl; 
        // cout << "\t";
        // Print();
        Thread *a = readyList->RemoveFront();
        // cout << "\tNext = " << a->getName() << endl;
    	return a;
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    // 註 : 經由 finish() -> Sleep() -> Run() 的 finishing 為 "True"
    /********************************************/
    Thread *oldThread = kernel->currentThread;
    // 這邊的 currentThread 在上一層 Sleep() 已經將 status 設為 BLOCKED 了
    /********************************************/
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    /********************************************/
    if (finishing) {	// mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
	    toBeDestroyed = oldThread;
    }
    // finishing == True，即表示 process 真的完成了
    // 用 toBeDestroyed 指到他，等下會做 delete
    /********************************************/

    /********************************************/
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
    // 若 process 還沒完成，則會將 thread 資訊儲存起來
    /********************************************/

    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    /********************************************/
    kernel->currentThread = nextThread;  // switch to the next thread
    // 將下一個要跑 thread 放到 CPU
    nextThread->setStatus(RUNNING);      // nextThread is now running
    // 將 Thread 的 status 設為 RUNNING
    /********************************************/
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    /**************************************************************/
    // cout << "In scheduler::Run \n\t";
    // Print();
    // // cout << "operating thread = " << oldThread->getName() << endl;
    // cout << "Before SWITCH" << endl;
    // cout << "\toldThread = " << oldThread->getName() << endl;
    // cout << "\tnextThread = " << nextThread->getName() << endl;
    // cout << endl;
    /**************************************************************/
    // int rand_num = rand()%20;
    // cout << "Before SWITCH" << endl;
    // cout << "rand_num = " << rand_num << "  , thread = " << oldThread->getName() << endl;
    // cout << "rand_num = " << rand_num << "  , thread = " << nextThread->getName() << endl;
    // cout << endl;
    SWITCH(oldThread, nextThread);

    /**************************************************************/
    // cout << "In scheduler::Run \n\t";
    // Print();
    // cout << "operating thread = " << nextThread->getName() << endl;
    // cout << "After SWITCH" << endl;
    // cout << "\texecute " << oldThread->getName() << endl;
    // cout << "rand_num = " << rand_num << endl;
    // cout << endl;

    // cout << "\toldThread = " << oldThread->getName() << endl;
    // cout << "\tnextThread = " << nextThread->getName() << endl;
    // cout << endl;
    /**************************************************************/

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    /**********************************************/
    CheckToBeDestroyed();		// check if thread we were running
					            // before this one has finished
					            // and needs to be cleaned up
    // 若 ToBeDestroyed 有指到東西則 delete 他
    /**********************************************/
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    //cout << "Ready list contents : ";
    readyList->Apply(ThreadPrint);
    //cout << "\n";
}
