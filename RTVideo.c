/*
 ============================================================================
 Name        : RTVideo.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include "Draw.h"
#include "Sched_new.h"
#include "Task.h"

// Polling rate default is 125Hz
#define POLLING_PERIOD	8
#define VIDEO_PERIOD	30
// Num sample for Calibration
#define NUM_SAMPLES		40
// Num tasks in the application
#define NUM_TASKS		6
#define NUM_FRAME1		379
#define NUM_FRAME2		1440

task_par	tp_play1;
task_par	tp_play2;
task_par	mouse_t;
task_par	action_mt;
task_par 	activation_t;
task_par 	plot_t;

extern float N;


//.............................................................................
// Function creates a task for the Calibration of the counter variable
//.............................................................................

void start_Calibration(int samples)
{
int			ret;
pthread_t	tid;

	printf("Start Calibration!\nWaiting...\n");

	ret = pthread_create(&tid, NULL, calibration_task, (void *)&samples);
	if(ret != 0) {
		perror("Error creating calibration task\n");
		exit(1);
	}

	ret = pthread_join(tid, NULL);
	if(ret != 0) {
		perror("Error join on calibration task\n");
		exit(1);
	}

	printf("Calibration finished!\n");
}

//.............................................................................
// Function inits a task that plays Video
// @parsm namevideo is the name of the video to load
// @param tp is the struct to memorize all useful parameters for the task
// @param dir is the path where are located the frames of the video
// @param nframes is the number of frames in the directory
// @param x & y are the coordinates where to display the video
//.............................................................................

void init_PlayTask(task_par *tp, char *namevideo, char *dir,
				int nframes, int x, int y)
{
	tp->priority = HIGH_PRIORITY;
	tp->deadline = tp->period = VIDEO_PERIOD;

	tp->frame_index = 1;

	tp->Ifolder.nframes	= nframes;
	tp->Ifolder.x_window = x;
	tp->Ifolder.y_window = y;
	strcpy(tp->Ifolder.name, namevideo);
	strcpy(tp->Ifolder.path, dir);
}

//.............................................................................
// Function inits all the tasks
//.............................................................................

void init_Tasks()
{
	// Init PlayTasks
	init_PlayTask(&tp_play1, "Bunny",
			"Video1/f_", NUM_FRAME1,
			VIDEO1_X, VIDEO1_Y);
	init_PlayTask(&tp_play2, "Earth",
			"Video2/f_", NUM_FRAME2,
			VIDEO2_X, VIDEO2_Y);

	// Init MouseTasks
	mouse_t.deadline = mouse_t.period = POLLING_PERIOD;
	mouse_t.priority = HIGH_PRIORITY;
	action_mt.deadline = action_mt.period = POLLING_PERIOD;
	action_mt.priority = HIGH_PRIORITY;

	// Init Activation Task
	activation_t.priority = HIGH_PRIORITY;

	// Init Plot Task
	plot_t.priority = LOW_PRIORITY;
}

void create_Tasks()
{
int			i, ret;
pthread_t 	tid[NUM_TASKS];

	tid[0] = create_task(action_mousetask, &action_mt);
	tid[1] = create_task(play_task, &tp_play1);
	tid[2] = create_task(play_task, &tp_play2);
	tid[3] = create_task(plot_task, &plot_t);
	tid[4] = create_task(mouse_task, &mouse_t);
	tid[5] = create_task(activator_task, &activation_t);

	// No join on action_mousetask, it could be blocked (no mouse events)
	// It could prevent exit with esc from application
	for(i=1; i<NUM_TASKS; i++) {
		ret = pthread_join(tid[i], NULL);
		if(ret != 0) {
			perror("Error join on Task \n");
			exit(1);
		}
	}
}
int	main ()
{
	setup_affinity_folder();
	// Calibration for portability
	start_Calibration(NUM_SAMPLES);

	// Init Allegro and loading on screen
	// the interface
	init();
	draw_interface();

	pthread_barrier_init(&barr,  NULL, NUM_TASKS);

	init_Tasks();
	create_Tasks();

	allegro_exit();

	printf("Demo Finished!\n");

	return EXIT_SUCCESS;
}
