/*
 ============================================================================
 Name        : Sched_new.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include "Sched_new.h"
#include <stdio.h>

pthread_mutex_t console_mux = PTHREAD_MUTEX_INITIALIZER;

int sched_setattr(pid_t pid,
                  const struct sched_attr *attr,
                  unsigned int flags)
{
  return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
                  struct sched_attr *attr,
                  unsigned int size,
                  unsigned int flags)
{
  return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

void set_scheduler(int priority) {
int		ret;
struct 	sched_attr attr;

	attr.size = sizeof(attr);
	attr.sched_flags =    0;
	attr.sched_nice =     0;
	attr.sched_priority = priority;

	attr.sched_policy =   SCHED_FIFO;
	attr.sched_runtime =  0;
	attr.sched_period =   0;
	attr.sched_deadline = 0;

	ret = sched_setattr(0, &attr, 0);
	if (ret < 0) {
		pthread_mutex_lock(&console_mux);
		perror("ERROR: sched_setattr");
		printf("priority: %d\n",
				attr.sched_priority);
		pthread_mutex_unlock(&console_mux);
		pthread_exit(NULL);
	}
}
