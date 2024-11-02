/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"

#include "synchconsole.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename); // need to change to (interrupt to fileSystem) and (CreateFile to )
}
// void SysPrintInt(int val){	//ori//原本的
// 	// cout << "from ksyscall.h call SysPrintInt\n";
// 	kernel->synchConsoleOut->PutInt(val);
// }//10/31 check example// kernal->interrupt->PrintInt(val);

//10/31患的/////////////////////////////////////
void SysPrintInt(int value){
	kernel->interrupt->PrintInt(value);
}
//10/31 check example// kernal->interrupt->PrintInt(val);
/////////////////////////// new add 10/2 for SysOpen
OpenFileId SysOpen(char *name)
{
	return kernel->fileSystem->OpenFile_MP1_(name);
}
int SysWrite(char *buffer, int size, OpenFileId id)
{
  return kernel->fileSystem->WriteFile_MP1_(buffer, size, id);
}

int SysRead(char *buffer, int size, OpenFileId id)
{
  return kernel->fileSystem->ReadFile_MP1_(buffer, size, id);
}

int SysClose(OpenFileId id)
{
  return kernel->fileSystem->CloseFile_MP1_(id);
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
