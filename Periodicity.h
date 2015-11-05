/*
 ============================================================================
 Name        : Periodicity.h
 Author      : Matteo Rotundo
 ============================================================================
 */

#ifndef PERIODICITY_H_
#define PERIODICITY_H_

#include <time.h>
#include "Task.h"

extern int time_cmp(struct timespec *, struct timespec *);
extern void time_add_ms(struct timespec *, long int);
extern void add_timespec(struct timespec *, long int, long int);
extern void time_copy(struct timespec *, const struct timespec *);
extern int deadline_miss(task_par *);
extern void wait_for_period(task_par *);
extern void set_period(task_par *);
extern void wait_for_one_sec(struct timespec *);
extern void busy_wait(int ms);

#endif /* PERIODICITY_H_ */
