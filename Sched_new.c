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


int test_affinity()
{
	cpu_set_t bitmap;

	sched_getaffinity(0,sizeof(bitmap), &bitmap);
	if( CPU_COUNT(&bitmap) == 1 && CPU_ISSET(0, &bitmap) != 0 )
		return 1;
	else
		return 0;
}

void set_affinity()
{
cpu_set_t	bitmap;

	CPU_ZERO(&bitmap); // Resetting bitmap
	CPU_SET(0, &bitmap); // Setting bitmap to zero

	// Taking cpu-0
	sched_setaffinity(0, sizeof(bitmap), &bitmap);

	if(test_affinity() == 0)
		exit(1);
}

void setup_affinity_folder()
{
FILE	*f;
char 	cpuset_folder[100];
char 	cpuset_file[100];

	strcpy(cpuset_folder, "/sys/fs/cgroup/cpuset/taskdl");

	printf("#Creating folder \"%s\"\n", cpuset_folder);
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

	// Sets which CPU will be used by the taskss

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

void set_affinityx()
{
FILE * f;
char cpuset_file[100];

	strcpy(cpuset_file, "/sys/fs/cgroup/cpuset/taskdl");

	strcat(cpuset_file, "/tasks");
	f = fopen(cpuset_file, "w");
	if (f == NULL) {
		printf("Error opening file \"%s\"\n", cpuset_file);
		exit(1);
	}

	fprintf(f, "%ld\n", gettid());

	fclose(f);

	if(test_affinity() == 0)
		exit(1);
}
