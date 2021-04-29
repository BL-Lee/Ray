#ifndef __TIMER_HEADER
#define __TIMER_HEADER

#include <time.h>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)

typedef struct _Timer
{
  struct timespec timer_start, timer_finish;
}Timer;


void startTimer(Timer* t)
{
  clock_gettime(CLOCK_MONOTONIC, &t->timer_start);
}
void endTimer(Timer* t)
{
  clock_gettime(CLOCK_MONOTONIC, &t->timer_finish);  
}
double getTimeElapsedMS(Timer* t)
{
  double elapsed = (t->timer_finish.tv_sec - t->timer_start.tv_sec) * 1000.0;
  elapsed += (t->timer_finish.tv_nsec - t->timer_start.tv_nsec) / 1000000.0;
  return elapsed;
}

#elif defined(_WIN32)

typedef struct _Timer
{
  clock_t timer_start, timer_finish;  
}Timer;

//This is split up into OS because on POSIX, apple clock() will not
//return wall clock time for multi threaded programs

void startTimer(Timer* t)
{
  t->timer_start = clock();
}
void endTimer(Timer* t)
{
  t->timer_finish = clock();
}
double getTimeElapsedMS(Timer* t)
{
  //get the seconds elapsed
  double elapsed = (double)(t->timer_finish - t->timer_start) / CLOCKS_PER_SEC;
  elapsed *= 1000.0;
  return elapsed;
}


#else
#error Timer wrapper not written for this OS yet!
#endif //OS

#endif //__TIMER_HEADER
