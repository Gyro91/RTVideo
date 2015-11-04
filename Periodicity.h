/*
 * Periodicity.h
 *
 *  Created on: Oct 31, 2015
 *      Author: Matteo Rotundo
 */

#ifndef PERIODICITY_H_
#define PERIODICITY_H_

#include <time.h>
#include <pthread.h>

typedef struct info_folder_ {
	char name[16];				// Name of the video
	char path[10];				// Path frame folder
	int nframes;				// Number of frames in the folder
	int x_window;				// For Display the video in a
	int y_window;				// certain Window
} Info_folder;

typedef struct task_par_ {
	pthread_t tid;				// Task tid
	int arg;					// Task argument
	int period;					// In milliseconds
	int deadline;				// Relative (ms)
	int priority;				// In [0,99]
	int dmiss;					// Number of misses
	int frame_r;				// Value of the frame rate
	int frame_c;				// Counter for the frame rate
	struct timespec at;			// Next activ. time
	struct timespec dl;			// Abs. Deadline
	Info_folder Ifolder;		// Info folder video
} task_par;

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
