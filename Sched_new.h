/*
 ============================================================================
 Name        : Sched_new.h
 Author      : Matteo Rotundo
 ============================================================================
 */

#ifndef SCHED_NEW_H_
#define SCHED_NEW_H_

#include <unistd.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include "Task.h"

#define gettid() syscall(__NR_gettid)

#define SCHED_DEADLINE	6

/* XXX use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr		314
#define __NR_sched_getattr		315
#endif

#ifdef __i386__
#define __NR_sched_setattr		351
#define __NR_sched_getattr		352
#endif

#ifdef __arm__
#define __NR_sched_setattr		380
#define __NR_sched_getattr		381
#endif

extern pthread_mutex_t console_mux;

struct sched_attr {
  __u32 size;

  __u32 sched_policy;
  __u64 sched_flags;

  /* SCHED_NORMAL, SCHED_BATCH */
  __s32 sched_nice;

  /* SCHED_FIFO, SCHED_RR */
  __u32 sched_priority;

  /* SCHED_DEADLINE (nsec) */
  __u64 sched_runtime;
  __u64 sched_deadline;
  __u64 sched_period;
};

extern int sched_setattr(pid_t pid,
                  const struct sched_attr *attr,
                  unsigned int flags);

extern int sched_getattr(pid_t pid,
                  struct sched_attr *attr,
                  unsigned int size,
                  unsigned int flags);

extern void set_scheduler(__u32, task_par *tp);
extern void set_sched_fifo(task_par *tp);
extern void set_sched_deadline(task_par *tp);
extern void set_affinity();
extern void set_sched_other();
extern void setup_affinity_folder();

#endif /* SCHED_NEW_H_ */
