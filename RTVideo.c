/*
 ============================================================================
 Name        : RTVideo.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "Draw.h"
#include "Sched_new.h"
#include "Task.h"

// Polling rate default is 125Hz
#define POLLING_PERIOD	8
#define VIDEO_PERIOD	30
// Num sample for Calibration
#define NUM_SAMPLES		30
// Num tasks in the application (without overload tasks)
#define NUM_TASKS		6
#define NUM_FRAME1		379
#define NUM_FRAME2		1440

task_par	tp_play1;		// Task plays Bunny's video
task_par	tp_play2;		// Task that plays Earth's video
task_par	mouse_t;		// Task that checks periodic mouse events
task_par	action_mt;		// Aperiodic Task that perform mouse events
task_par 	activation_t;	// Task which activates overload task
task_par 	plot_t; 		// Task that must plot workload function
task_par overload1_tk;		// Overload task ball
task_par overload2_tk;		// Overload task random circle
task_par overload3_tk;		// Overload task pseudo radar

extern float N;
__u32 policy = 0; // Policy of the scheduler


//.............................................................................
// Function checks the policy argument
//.............................................................................

void check_arg()
{
	switch(policy) {
	case SCHED_DEADLINE:
		printf("Policy: SCHED_DEADLINE\n");
		break;
	case SCHED_FIFO:
		printf("Policy: SCHED_FIFO\n");
		break;
	default:
		printf("Policy value is not correct\n");
		exit(1);
		break;
	}
}

//.............................................................................
// Function takes the policy argument
//.............................................................................

void get_arg(int argc, char *argv[])
{
char	c;

	if (argv[optind] == NULL || argv[optind + 1] == NULL) {
		printf("Mandatory argument(s) missing\n");
		exit(1);
	}
	while ((c = getopt (argc, argv, "s:")) != -1) {
		switch (c)
		{
		case 's':
			policy = atoi(optarg);
			break;
		case '?':
			if (optopt == 't')
				fprintf (stderr, "Option -%c requires an argument.\n",
						optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
						"Unknown option character `\\x%x'.\n",
						optopt);
			abort();
		default:
			abort();
		}
	}

}

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
	activation_t.deadline = activation_t.period = POLLING_PERIOD;

	// Init Plot Task
	plot_t.priority = LOW_PRIORITY;

	// Init overload tasks
	overload1_tk.period = overload1_tk.deadline = 5;
	overload2_tk.period = overload2_tk.deadline = 40;
	overload3_tk.period = overload3_tk.deadline = 5;
	overload1_tk.priority = overload2_tk.priority =
			overload3_tk.priority = HIGH_PRIORITY;

}

void create_Tasks()
{
int			i, ret;
pthread_t 	tid[NUM_TASKS];

	tid[0] = create_task(action_mousetask, &action_mt);
	tid[1] = create_task(play_task, &tp_play1);
	tid[2] = create_task(play_task, &tp_play2);
	tid[3] = create_task(plot_task, &plot_t);
	tid[4] = create_task(activator_task, &activation_t);
	tid[5] = create_task(mouse_task, &mouse_t);

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
int	main (int argc, char *argv[])
{
	// Getting and Checking policy
	get_arg(argc, argv);
	check_arg();
	// Init folder for affinity
	setup_affinity_folder();
	// Calibration for portability
	start_Calibration(NUM_SAMPLES);

	// Init Allegro and loading on screen
	// the interface
	init();
	draw_interface();

	pthread_barrier_init(&barr, NULL, NUM_TASKS);

	init_Tasks();
	create_Tasks();

	allegro_exit();

	printf("Demo Finished!\n");

	return EXIT_SUCCESS;
}
