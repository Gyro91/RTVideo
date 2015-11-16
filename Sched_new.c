/*
 ============================================================================
 Name        : Sched_new.c
 Author      : Matteo Rotundo
 ============================================================================
 */

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Sched_new.h"
#define ALL_TASKS 	9
// Utilizazion factor for one CPU
#define Uf			1

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

//.............................................................................
// Sets SCHED_FIFO
//.............................................................................

void set_sched_fifo(task_par *tp)
{
int		ret;
struct 	sched_attr attr;

	attr.size = sizeof(attr);
	attr.sched_flags =    0;
	attr.sched_nice =     0;
	attr.sched_priority = tp->priority;

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

//.............................................................................
// Sets SCHED_DEADLINE
//.............................................................................

void set_sched_deadline(task_par *tp)
{
int		ret;
struct 	sched_attr attr;

	attr.size = sizeof(attr);
	attr.sched_flags =    0;
	attr.sched_nice =     0;
	attr.sched_priority = 0;

	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime =  tp-> deadline * ((float)Uf / (float)(ALL_TASKS))
			* 1000 * 1000;
	attr.sched_period =   tp->period * 1000 * 1000;
	attr.sched_deadline = tp->deadline * 1000 * 1000;

	ret = sched_setattr(0, &attr, 0);
	if (ret < 0) {
		pthread_mutex_lock(&console_mux);
		perror("ERROR: sched_setattr");
		perror("ERROR: sched_setattr");
		printf("runtime: %lld\nperiod: %lld\ndeadline: %lld\n",
		           attr.sched_runtime,
		           attr.sched_period,
		           attr.sched_deadline);
		pthread_mutex_unlock(&console_mux);
		pthread_exit(NULL);
	}
}


//.............................................................................
// Sets SCHED_OTHER
//.............................................................................

void set_sched_other()
{
int		ret;
struct 	sched_attr attr;

	attr.size = sizeof(attr);
	attr.sched_flags =    0;
	attr.sched_nice =     0;
	attr.sched_priority = 0;

	attr.sched_policy = SCHED_OTHER;
	attr.sched_runtime = 0;
	attr.sched_period =   0;
	attr.sched_deadline = 0;

	ret = sched_setattr(0, &attr, 0);
	if (ret < 0) {
		pthread_mutex_lock(&console_mux);
		perror("ERROR: sched_setattr");
		perror("ERROR: sched_setattr");
		printf("runtime: %lld\nperiod: %lld\ndeadline: %lld\n",
		           attr.sched_runtime,
		           attr.sched_period,
		           attr.sched_deadline);
		pthread_mutex_unlock(&console_mux);
		pthread_exit(NULL);
	}
}

//.............................................................................
// Sets the right scheduler
//.............................................................................

void set_scheduler(__u32 policy, task_par *tp)
{
	if ( policy == SCHED_FIFO)
		set_sched_fifo(tp);

	if ( policy == SCHED_DEADLINE)
		set_sched_deadline(tp);

}

//.............................................................................
// Test if affinity to core-0 is ok
// @return 1 if all is correct, 0 otherwise
//.............................................................................

int test_affinity()
{
	cpu_set_t bitmap;

	sched_getaffinity(0,sizeof(bitmap), &bitmap);
	if(CPU_COUNT(&bitmap) == 1 && CPU_ISSET(0, &bitmap) != 0)
		return 1;
	else
		return 0;
}


//.............................................................................
// Creating task folder with the options for my taskset
//.............................................................................

void setup_affinity_folder()
{
FILE	*f;
char 	cpuset_folder[100];
char 	cpuset_file[100];

	strcpy(cpuset_folder, "/sys/fs/cgroup/cpuset/taskset");

	printf("Creating folder \"%s\"\n", cpuset_folder);
	rmdir(cpuset_folder);
	if (mkdir(cpuset_folder, S_IRWXU)) {
		printf("Error creating CPUSET folder\n");
		pthread_exit(NULL);
	}

	// Updates the memory node

	strcpy(cpuset_file, cpuset_folder);
	strcat(cpuset_file, "/cpuset.mems");
	f = fopen(cpuset_file, "w");
	if (f == NULL) {
		printf("Error opening file \"%s\"\n", cpuset_file);
		exit(1);
	}
	fprintf(f, "0");
	fclose(f);

	// Sets which CPU will be used by the tasks

	strcpy(cpuset_file, cpuset_folder);
	strcat(cpuset_file, "/cpuset.cpus");
	f = fopen(cpuset_file, "w");
	if (f == NULL) {
		printf("Error opening file \"%s\"\n", cpuset_file);
		exit(1);
	}

	fprintf(f, "0");

	fclose(f);

}

//.............................................................................
// Sets affinity for the task caller
//.............................................................................

void set_affinity()
{
FILE * f;
char cpuset_file[100];

	strcpy(cpuset_file, "/sys/fs/cgroup/cpuset/taskset");

	strcat(cpuset_file, "/tasks");
	f = fopen(cpuset_file, "w");
	if (f == NULL) {
		printf("Error opening file \"%s\"\n", cpuset_file);
		exit(1);
	}

	fprintf(f, "%ld\n", gettid());

	fclose(f);

	if (test_affinity() == 0) {
		pthread_mutex_lock(&console_mux);
		printf("Error set affinity\n");
		pthread_mutex_unlock(&console_mux);
		exit(1);
	}
}
