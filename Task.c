/*
 ============================================================================
 Name        : Task.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sched.h>
#include "Periodicity.h"
#include "Sched_new.h"
#include "Draw.h"

#define LOOP			1
#define STEP_COUNT		0.000001

extern task_par overload1_tk;
extern task_par overload2_tk;
extern task_par overload3_tk;
extern int 		last_step;
extern __u32 	policy;

float 			N;					// Value of counting task alone
int 			cond_mouse = 0; 	// Condition of mouse
									// (1 if pressed in the right area)
unsigned int 	esc = 0;  			// If 0 the application can execute,
									// otherwise exit

// Barrier is needed to avoid a thread to block due to the stark scheduling.
// Real time task has all the bandwidth and if a task like plotTask, that is
// all active, changes scheduler before another task, so he cannot goes
// in execution.
// We must be sure that all task has set its scheduler to the right class
// with the right priority

pthread_barrier_t 	barr;
pthread_mutex_t 	mux;	 	// Define a mutex
pthread_cond_t		cv;			// Define a cond. variable


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

//.............................................................................
// Creates a task with the given parameters
//.............................................................................

pthread_t create_task(void *f(void *), task_par *tp)
{
int			ret;
pthread_t	tid;

	ret = pthread_create(&tid, NULL, f, (void *) tp);
	if(ret != 0) {
		perror("Error creating task!\n");
		exit(1);
	}

	tp->tid = tid;

	return tid;
}

//.............................................................................
// Creates an overload task f withparameters tp and modify count and scan
// for the activation task
//.............................................................................

void activate_overloadtask(void *f(void *), task_par *tp, int *count,
		char *scan)
{
	create_task(f, tp);
	*count += 1;
	*scan = 0;
}

//.............................................................................
// Load video on the screen
//.............................................................................

void play_video(task_par *tp)
{
char	filename[18];
char	nframe[5];
BITMAP	*bmp;
Info_folder	*Ifolder =	&(tp->Ifolder);

	sprintf(nframe, "%d", tp->frame_index);
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
		// If it is more than a second
		// updating value framerate
		tp->frame_r = tp->frame_c;
		tp->frame_c = 0;
		time_add_ms(t, 1000);
	}
	if(tp->frame_r < 10 && policy != SCHED_DEADLINE)
	// Clear old state
		rectfill(screen, Ifolder->x_window, VIDEO_HEIGHT,
				Ifolder->x_window + COLUMN - 2,
				START_OVERLOAD_SCREEN - 2 ,
				BLACK);
	// Name video on screen
	textout_ex(screen, font, Ifolder->name,
			Ifolder->x_window + 10, VIDEO_HEIGHT,
			WHITE, BLACK);

	// Dmiss on screen
	if(policy != SCHED_DEADLINE)
		text_state(Ifolder->x_window + 80, VIDEO_HEIGHT,
				"Dmiss:", tp->dmiss);
	else
		textout_ex(screen, font, "Dmiss:-",
				Ifolder->x_window + 80, VIDEO_HEIGHT,
				WHITE, BLACK);
	// Frame Rate on screen
	text_state(Ifolder->x_window + 170, VIDEO_HEIGHT,
			"Frame-Rate(FPS):", tp->frame_r);

}

//.............................................................................
// Body of a task that plays a video on the screen
//.............................................................................

void *play_task(void *p)
{
task_par		*tp = (task_par *)p;
Info_folder		*Ifolder =	&(((task_par *)p)->Ifolder);
struct timespec	t;

	set_scheduler(policy, tp);
	set_affinity();
	wait_on_barr();

	// Setup t for counting the frame rate in a second
	wait_for_one_sec(&t);
	set_period(tp);
	while (!esc) {

		// Loading the video on the screen
		play_video(tp);

		// Loading Info video on the screen
		load_state(tp, &t);

		// Checking dl_miss & waiting for
		// the next activation
		deadline_miss(tp);
		wait_for_period(tp);

		// Updating index to the next frame in the folder
		tp->frame_index++;
		// Counting the frames in a second
		tp->frame_c++;

		if (tp->frame_index == (Ifolder->nframes + 1))
			tp->frame_index = 1;
	}

	pthread_mutex_lock(&console_mux);

	printf("Play task exit\n");

	pthread_mutex_unlock(&console_mux);
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
float 				n = 0, workload;
struct 	timespec 	t, now;
int 				y = 0, x = ORIGIN_X;
task_par 			*tp = (task_par*)p;

	set_sched_fifo(tp);
	set_affinity();
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
	} while (!esc);

	pthread_mutex_lock(&console_mux);

	printf("Plot task exit\n");

	pthread_mutex_unlock(&console_mux);
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
task_par tp;

	tp.priority = LOW_PRIORITY;
	N = 0;
	set_scheduler(SCHED_FIFO, &tp);
	set_affinity();

	clock_gettime(CLOCK_MONOTONIC, &t);
	do {
		time_add_ms(&t, FAKE_PERIOD);
		if(f > N)
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

	pthread_mutex_init(&mux, NULL);
	pthread_cond_init(&cv, NULL);
	show_mouse(screen);

	set_scheduler(policy, tp);
	set_affinity();
	wait_on_barr();

	if(policy == SCHED_DEADLINE)
		enable_hardware_cursor();

	set_period(tp);
	while (!esc) {
		pthread_mutex_lock(&mux);
		if (mouse_b & 1) {
			// If mouse sx pressed
			x = mouse_x;
			y = mouse_y;
			if (x > COLUMN * 2 && y < START_OVERLOAD_SCREEN) {
				// If pressed in the right area, wakes up thread
				cond_mouse = 1;
				pthread_cond_signal(&cv);
			}
			// Reset mouse state
			mouse_b = 0;
		}
		pthread_mutex_unlock(&mux);

		wait_for_period(tp);
	}

	pthread_mutex_lock(&console_mux);

	printf("Mouse task exit\n");

	pthread_mutex_unlock(&console_mux);
	pthread_exit(NULL);
}


//.............................................................................
// Body of the aperiodic task that must perform the action of a mouse event
//.............................................................................

void *action_mousetask(void *p)
{
int			i, work;
task_par 	*tp = (task_par *)p;

	set_scheduler(policy, tp);
	set_affinity();
	wait_on_barr();

	set_period(tp);
	while (!esc) {
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
	pthread_mutex_lock(&console_mux);

	printf("Action mouse exit\n");

	pthread_mutex_unlock(&console_mux);
	pthread_exit(NULL);
}


//.............................................................................
// Body of the  task that must create an overload task after a button press
// Animation of a ball that goes from left to right and viceversa in the screen
//.............................................................................

void *overload_task1(void *p)
{
int 		step_initial = 10,
			step_final = COLUMN - 12;
int 		step = 1, actual_positionx = 10,
			actual_positiony = START_OVERLOAD_SCREEN + 100;
task_par 	*tp = (task_par*)p;

	set_scheduler(policy, tp);
	set_affinity();

	set_period(tp);
	while (!esc) {
		// Erases old ball
		rectfill(screen, 0, START_OVERLOAD_SCREEN + 1,
				COLUMN - 1, START_GRAPH_SCREEN - 2,
				BLACK);
		// Draws ball
		circlefill(screen, actual_positionx,
				actual_positiony,
				10, GOLD);

		actual_positionx += step;

		// Check boundary limits
		if (actual_positionx == step_final)
			step = -1;
		if (actual_positionx == step_initial)
			step = 1;

		// Overload
		busy_wait(5);

		wait_for_period(tp);
	}

	pthread_exit(NULL);
}

//.............................................................................
// Body of the  task that must create an overload task after a button press
// The animation is a blinking circle
//.............................................................................

void *overload_task2(void *p)
{
task_par *tp = (task_par*)p;

	set_scheduler(policy, tp);
	set_affinity();

	set_period(tp);
	while (!esc) {
		// Draw Animation
		circlefill(screen, (3 * COLUMN) / 2,
				START_OVERLOAD_SCREEN + 100,
				50, RANDOM);
		// Overload
		busy_wait(30);

		wait_for_period(tp);
	}

	pthread_exit(NULL);
}

//.............................................................................
// Body of the  task that must create an overload task after a button press
// The animation is like a clock
//.............................................................................


void *overload_task3(void *p)
{
double		xf = ((5 * COLUMN) / 2) + 80,
			yf = (3 * START_OVERLOAD_SCREEN) / 2;
// Center point
double 		cx = (5 * COLUMN) / 2,
			cy = (3 * START_OVERLOAD_SCREEN) / 2;
// angle is 1 degree in radius
float 		angle = 0.0174533;
task_par 	*tp = (task_par*)p;

	set_scheduler(policy, tp);
	set_affinity();
	enable_hardware_cursor();

	set_period(tp);
	while (!esc) {
		// Clean a portion of screen
		rectfill(screen, 2 * COLUMN + 1, START_OVERLOAD_SCREEN + 1,
						 3 * COLUMN - 1, START_GRAPH_SCREEN - 2,
						BLACK);
		// Rotating a point of one degree
		xf = (xf - cx) * cos(angle) - (yf - cy) * sin(angle) + cx;
		yf = (xf - cx) * sin(angle) +  (yf - cy) * cos(angle) + cy;
		line(screen, cx, cy, xf, yf, RED);
		circle(screen, cx, cy, 80, RED);

		// Overload
		busy_wait(30);

		wait_for_period(tp);
	}

	pthread_exit(NULL);
}

//.............................................................................
// Body of the  task that must create an overload task after a button press
//.............................................................................

void *activator_task(void *p)
{
int 		count = 0;
char		scan;
task_par 	*tp = (task_par*)p;

	set_sched_fifo(tp);
	set_affinity();
	wait_on_barr();

	while (count < 4) {
		// Waiting for a input
		get_keycodes(&scan);
		// Check which tasks must be created
		if (scan == KEY_ENTER && count == 0)
			activate_overloadtask(overload_task1, &overload1_tk,
					&count, &scan);
		if (scan == KEY_ENTER && count == 1)
			activate_overloadtask(overload_task2, &overload2_tk,
				&count, &scan);
		if (scan == KEY_ENTER && count == 2)
			activate_overloadtask(overload_task3, &overload3_tk,
				&count, &scan);
		if (scan == KEY_ESC) {
			pthread_mutex_lock(&console_mux);

			printf("Start to exit\n");

			pthread_mutex_unlock(&console_mux);
			count = 4;
			esc = 1;
		}
	}

	pthread_mutex_lock(&console_mux);

	printf("Activator task exit\n");

	pthread_mutex_unlock(&console_mux);

	pthread_exit(NULL);
}
