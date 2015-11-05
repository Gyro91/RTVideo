/*
 ============================================================================
 Name        : Task.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "Periodicity.h"
#include "Sched_new.h"
#include "Draw.h"

// Resolution for videos
#define VIDEO_HEIGHT	200
#define VIDEO_WIDTH		320

#define LOOP			1
#define FAKE_PERIOD		100
#define STEP_COUNT		0.000001

#define UNUSED(x)		(void)x
float N;


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
	set_scheduler(1);
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
		if(x > HEIGHT_AXIS_X) {
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

