#ifndef __THREADS_HEADER
#define __THREADS_HEADER

#include <stdint.h>

void CreateThread(uint64_t* id, void* (*routine)(void*), void* args);
void JoinThread(uint64_t id, void** retval);
uint64_t LockedAdd(volatile uint64_t* recipient, uint64_t amount);
int32_t GetNumProcessors();

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)

 #include <pthread.h>
 #include <unistd.h>


 void CreateThread(pthread_t* id, void* (*routine)(void*), void* args, void* handle)
 {
   pthread_create(id, NULL, routine, args);
 }

 void JoinThread(void* id, void** retval)
 {
   pthread_join(*(pthread_t*)id, retval);
 }

 uint64_t LockedAdd(volatile uint64_t* recipient, uint64_t amount)
 {
   //this might be gcc dependent
  return __sync_fetch_and_add(recipient, amount);
 }

 int32_t GetNumProcessors()
 {
   return sysconf(_SC_NPROCESSORS_ONLN);
 }


#elif defined(_WIN32)

 /*TODO*/
 /*THIS IS UNTESTED*/
 #include <windows.h>

 //TODO: rename this so it doesn't have the same name as the windows create thread
 void CreateThread(uint64_t* id, void* (*routine)(void*), void* args)
 {
   DWORD threadID;
   HANDLE threadHandle = CreateThread( (LPSECURITY_ATTRIBUTES)NULL, (SIZE_T)NULL, (LPTHREAD_START_ROUTINE)routine, args, (DWORD)0, &threadID);
   CloseHandle(threadHandle);
 }

void JoinThread(void* id, void** retval)
 {
   WaitForSingleObject((HANDLE)id, FLT_MAX);
 }

 //NOTE: This casts from unsigned to signed... this is okay?
 uint64_t LockedAdd(volatile uint64_t* recipient, uint64_t amount)
 {
   return InterlockedExchangeAdd64((int64_t*)recipient, amount);
 }

 int32_t GetNumProcessors()
 {
   SYSTEM_INFO sysinfo;
   GetSystemInfo(&sysinfo);
   return sysinfo.dwNumberOfProcessors;
 }

#else
#error Thread wrapper not implemented on this OS!

#endif //OS


#endif //__THREADS_HEADER
