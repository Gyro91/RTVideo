/*
 ============================================================================
 Name        : Task.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>
#include "Periodicity.h"
#include "Sched_new.h"
#include "Draw.h"

// Resolution for videos
#define VIDEO_HEIGHT	200
#define VIDEO_WIDTH		320

#define LOOP			1
#define STEP_COUNT		0.000001

#define UNUSED(x)		(void)x

extern int 	last_step;
float 		N;			// Value of counting task alone
int cond_mouse = 0; 	// Condition of mouse (1 if pressed in the right area)

// Barrier is needed to avoid a thread to block due to the stark scheduling
// Real time task has all the bandwidth and if a task like plotTask, that is
// all active, changes scheduler before another task, so he cannot goes
// in execution.
// We must be sure that all task has set its scheduler to the right class
// with the right priority
pthread_barrier_t barr;

pthread_mutex_t mux;	 // Define a mutex
pthread_cond_t	cv;		// 	Define a cond. variable

void wait_on_barr()
{
int	rc;

	rc = pthread_barrier_wait(&barr);
	if((rc != 0) &&
			(rc != PTHREAD_BARRIER_SERIAL_THREAD)) {
		printf("Could not wait on barrier\n");
		exit(1);
	}
}

void set_affinity()
{
	cpu_set_t bitmap;

	CPU_ZERO(&bitmap); // resetting bitmap
	CPU_SET(0, &bitmap); // setting bitmap to zero

	sched_setaffinity(0, sizeof(bitmap), &bitmap); // taking cpu-0
}

//.............................................................................
// Load video on the screen
//.............................................................................

void play_video(task_par *tp, int i)
{
char	filename[18];
char	nframe[5];
BITMAP	*bmp;
Info_folder	*Ifolder =	&(tp->Ifolder);

	sprintf(nframe, "%d", i);
	strcpy(filename, Ifolder->path);
	strcat(filename, nframe);
	strcat(filename, ".bmp");

	bmp = load_bitmap(filename, NULL);
	if (bmp == NULL) {
		printf("File not found\n");
		exit(1);
	}

	blit(bmp, screen, 0, 0, Ifolder->x_window, Ifolder->y_window,
			VIDEO_WIDTH + 14, VIDEO_HEIGHT);

}

//.............................................................................
// Load state of the task on the screen
//.............................................................................

void load_state(task_par *tp, struct timespec *t)
{
Info_folder	*Ifolder =	&(tp->Ifolder);
struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	if (time_cmp(&now,t) >= 0) {
		tp->frame_r = tp->frame_c;
		tp->frame_c = 0;
		time_add_ms(t, 1000);
	}

	textout_ex(screen, font, Ifolder->name,
			Ifolder->x_window + 10, 200,
			WHITE, BLACK);

	text_state(Ifolder->x_window + 80, 200, "Dmiss:", tp->dmiss);
	text_state(Ifolder->x_window + 170, 200, "Frame-Rate(sec):", tp->frame_r);

}

//.............................................................................
// Body of a task that plays a video on the screen
//.............................................................................

void *play_task(void *p)
{
int				i = 1;
task_par		*tp = (task_par *)p;
Info_folder		*Ifolder =	&(((task_par *)p)->Ifolder);
struct timespec	t;

	set_scheduler(99);
	set_affinity();
	wait_on_barr();
	// Setup t for counting the frame rate in a second
	wait_for_one_sec(&t);
	set_period(tp);
	while(LOOP) {

		// Loading the video on the screen
		play_video(tp, i);

		// Loading Info video on the screen
		load_state(tp, &t);

		// Checking dl_miss & waiting for
		// the next activation
		deadline_miss(tp);
		wait_for_period(tp);

		// Updating index to the next frame in the folder
		i++;
		// Counting the frames in a second
		tp->frame_c++;

		if (i == (Ifolder->nframes + 1))
			i = 1;
	}

	pthread_exit(NULL);
}

//.............................................................................
// This is the body of task that must plot the workload function on the screen
// For computing the workload we apply:
//
// Workload = 1 - n / N
//
// n is the value of counting when other task are active
// N is the value of counting when the task is alone
//.............................................................................

void *plot_task(void *p)
{
float 	n = 0, workload;
struct 	timespec t, now;
int y = 0, x = ORIGIN_X;

	UNUSED(p);
	set_affinity();
	set_scheduler(1);
	wait_on_barr();

	clock_gettime(CLOCK_MONOTONIC, &t);
	do {
		time_add_ms(&t, FAKE_PERIOD);
		// Computing workload
		workload = 1 - n / N;
		// Plotting workload
		y = workload * 100;
		draw_point(x, y);
		// Moving to the next time
		x = x + SCALE_X;
		if(x == last_step) {
			// Updating graph
			clean_graph();
			x = ORIGIN_X;
		}
		// Counting
		n = 0;
		do {
			n += STEP_COUNT;
			clock_gettime(CLOCK_MONOTONIC, &now);
		} while (time_cmp(&now, &t) < 0);
	} while (LOOP);

	pthread_exit(NULL);
}

//.............................................................................
// This is the body of task that must computes the calibration
// We want a task that counts the workload of a taskset online. For this
// we apply this formula:
//
//	Workload = 1 - n / N
//
// The calibration task has to find N, namely a counting variable in a fake
// period when the task is alone.
// @param p is the number of samples that we want to consider in our
// calibration
//.............................................................................

void *calibration_task(void *p)
{
int 	sec = *(int *)p;
float 	f = 0;
struct 	timespec t, now;

	N = 0;
	set_affinity();
	set_scheduler(1);
	clock_gettime(CLOCK_MONOTONIC, &t);
	do {
		time_add_ms(&t, FAKE_PERIOD);
		if( f > N)
			N = f;
		f = 0;
		sec--;
		do {
			f += STEP_COUNT;
			clock_gettime(CLOCK_MONOTONIC, &now);
		} while (time_cmp(&now, &t) < 0);
	} while (sec >= 0);

	pthread_exit(NULL);
}

//.............................................................................
// Body of the task that must handles mouse events
//.............................................................................

void *mouse_task(void *p)
{
int 		x, y;
task_par 	*tp = (task_par *)p;

	show_mouse(screen);

	pthread_mutex_init(&mux, NULL);
	pthread_cond_init(&cv, NULL);

	set_affinity();
	set_scheduler(98);
	wait_on_barr();

	set_period(tp);
	while(LOOP) {
		pthread_mutex_lock(&mux);
		if (mouse_b & 1) {
			// if mouse pressed
			x = mouse_x;
			y = mouse_y;
			if (x > COLUMN * 2 && y < START_OVERLOAD_SCREEN) {
				// if pressed in the right area, wakes up thread
				cond_mouse = 1;
				pthread_cond_signal(&cv);
			}
			mouse_b = 0;
		}
		pthread_mutex_unlock(&mux);

		wait_for_period(tp);
	}
	pthread_exit(NULL);
}


//.............................................................................
// Body of the aperiodic task that must perform the action of a mouse event
//.............................................................................

void *action_mousetask(void *p)
{
int			i, work;
task_par 	*tp = (task_par *)p;

	set_affinity();
	set_scheduler(99);
	wait_on_barr();

	set_period(tp);
	while(LOOP) {
		pthread_mutex_lock(&mux);
		while (!cond_mouse) {
			// Wait mouse pressed in the right area
			pthread_cond_wait(&cv, &mux);
		}
		mouse_b = 0;
		cond_mouse = 0;
		pthread_mutex_unlock(&mux);
		// Do a job
		printf("Mouse pressed!\n");
		ellipse(screen, (COLUMN * 5) / 2,
					START_OVERLOAD_SCREEN / 2,
					100, 100, RANDOM);
		for(i=0; i<10000000; i++)
			work++;

		wait_for_period(tp);
	}
	pthread_exit(NULL);
}

