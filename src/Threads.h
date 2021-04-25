#ifndef __THREADS_HEADER
#define __THREADS_HEADER

#include <stdint.h>

int32_t CreateThread(uint64_t* id, void* (*routine)(void*), void* args);
int32_t JoinThread(uint64_t id, void** retval);
uint64_t LockedAdd(volatile uint64_t* recipient, uint64_t amount);
int32_t GetNumProcessors();

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)

 #include <pthread.h>
 #include <unistd.h>


 int CreateThread(pthread_t* id, void* (*routine)(void*), void* args)
 {
   return pthread_create(id, NULL, routine, args);
 }

 int JoinThread(pthread_t id, void** retval)
 {
   return pthread_join(id, retval);
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

 int32_t CreateThread(uint64_t* id, void* (*routine)(void*), void* args)
 {
   return CreateThread( 0, 0, routine, args, 0, id);
 }

 int32_t JoinThread(uint64_t id, void** retval)
 {
   WaitForSingleObject(id, FLT_MAX);
 }

 uint64_t LockedAdd(volatile uint64_t* recipient, uint64_t amount)
 {
   return InterlockedExchangeAdd64(recipient, amount);
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
