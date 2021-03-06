/*
 ============================================================================
 Name        : Periodicity.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include "Periodicity.h"
#include <assert.h>
#include <limits.h>
#include <sched.h>
#include "Sched_new.h"

extern __u32 policy;

//.............................................................................
// Copies the second passed time to the first one
//.............................................................................

void time_copy(struct timespec *dst, const struct timespec *src)
{
  dst->tv_sec = src->tv_sec;
  dst->tv_nsec = src->tv_nsec;
}

//.............................................................................
// Compares two times
//
// returns:
// 		1 if the first time passed is greater than the second one
// 		0 if times are equal
// 	   -1 otherwise
//.............................................................................

int time_cmp(struct timespec *t1, struct timespec *t2)
{
  if (t1->tv_sec > t2->tv_sec) return 1;
  if (t1->tv_sec < t2->tv_sec) return -1;
  if (t1->tv_nsec > t2->tv_nsec) return 1;
  if (t1->tv_nsec < t2->tv_nsec) return -1;
  return 0;
}


//.............................................................................
// Adds ms to the given time
//.............................................................................

void time_add_ms(struct timespec *dst, long int ms)
{
  dst->tv_sec += ms/1000;
  dst->tv_nsec += (ms % 1000) * 1e6;
  if (dst->tv_nsec > 1e9) {
    dst->tv_nsec -= 1e9;
    dst->tv_sec++;
  }
}

//.............................................................................
// Adds seconds and nanoseconds to the given time
//.............................................................................

void add_timespec(struct timespec *dst, long int s, long int ns)
{
	// if could be overflow
	if (ns > 0 && dst->tv_nsec > LONG_MAX - ns) {
		s += ns / (long int) 1e9;
		ns = ns % (long int) 1e9;
	} // ns could be negative example 4.1s and 3.9s
	else if(ns < 0) {
		dst->tv_sec--;
		ns += 1e9;
	}
	dst->tv_nsec += ns;
	dst->tv_sec += s;

	assert(dst->tv_nsec >= 0);

}

//.............................................................................
// Reads the currrent time and computes the next activation time and the
// absolute deadline of the task.
//.............................................................................

void set_period(task_par *tp)
{
struct timespec	t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), &t);
	time_copy(&(tp->dl), &t);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->deadline);
}

//.............................................................................
// Suspends the calling thread until the next activation and, when awaken,
// updates activation time and deadline
//.............................................................................

void wait_for_period(task_par *tp)
{
	if(policy == SCHED_FIFO)
		clock_nanosleep(CLOCK_MONOTONIC,
				TIMER_ABSTIME, &(tp->at), NULL);
	if(policy == SCHED_DEADLINE)
		sched_yield();
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->deadline);
}

//.............................................................................
// Adds a delay of 1 sec to the given time
//.............................................................................

void wait_for_one_sec(struct timespec *t)
{
	clock_gettime(CLOCK_MONOTONIC, t);
	time_add_ms(t, 1000);
}

//.............................................................................
// If the thread is still in execution when reactivated, it increments
// value of dmiss and returns 1, otherwise return 0.
//.............................................................................

int deadline_miss(task_par *tp)
{
int	ret;
struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	if (time_cmp(&now, &tp->dl) > 0) {
		tp->dmiss++;
		ret = 1;
	}
	else
		ret = 0;

	return ret;
}


void busy_wait(int ms)
{
  struct timespec t, now;

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t);
  time_add_ms(&t, ms);
  do {
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
  } while (time_cmp(&now, &t) < 0) ;
}
