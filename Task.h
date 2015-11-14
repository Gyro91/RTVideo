/*
 ============================================================================
 Name        : Task.h
 Author      : Matteo Rotundo
 ============================================================================
 */

#ifndef TASK_H_
#define TASK_H_

#include <pthread.h>

#define LOW_PRIORITY	1
#define HIGH_PRIORITY	99

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
	int frame_index;			// Next frame to be loaded
	int frame_r;				// Value of the frame rate
	int frame_c;				// Counter for the frame rate
	struct timespec at;			// Next activ. time
	struct timespec dl;			// Abs. Deadline
	Info_folder Ifolder;		// Info folder video
} task_par;

extern pthread_barrier_t barr;

extern void *play_task(void *p);
extern void *calibration_task(void *p);
extern void *plot_task(void *p);
extern void *mouse_task(void *p);
extern void *action_mousetask(void *p);
extern void *activator_task(void *p);
extern pthread_t create_task(void *f(void *), task_par *tp);

#endif /* TASK_H_ */
