#include <time.h>


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
	dst->tv_sec += s;
	dst->tv_nsec += ns;
	if (dst->tv_nsec >= 1e9) {
		    dst->tv_nsec -= 1e9;
		    dst->tv_sec += 1;
	}
}
