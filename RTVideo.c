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


// Structures for the 2 tasks that play videos
task_par tp_play1;
task_par tp_play2;

extern float N;

//.............................................................................
// Function creates a task for plotting workload function
//.............................................................................

void create_PlotTask()
{
int			ret;
pthread_t	tid;

	ret = pthread_create(&tid, NULL, plot_task, NULL);
	if(ret != 0) {
		perror("Error creating plot task\n");
		exit(1);
	}

	ret = pthread_join(tid, NULL);
	if(ret != 0) {
		perror("Error join on plot task\n");
		exit(1);
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
// Function creates a task to play Video
// @parsm namevideo is the name of the video to load
// @param tp is the struct to memorize all useful parameters for the task
// @param dir is the path where are located the frames of the video
// @param nframes is the number of frames in the directory
// @param x & y are the coordinates where to display the video
//.............................................................................

void create_PlayTask(task_par *tp, char *namevideo, char *dir,
				int nframes, int x, int y)
{
int			ret;
pthread_t	tid;

	tp->period = 30;
	tp->deadline = 30;

	tp->Ifolder.nframes	= nframes;
	tp->Ifolder.x_window = x;
	tp->Ifolder.y_window = y;
	strcpy(tp->Ifolder.name, namevideo);
	strcpy(tp->Ifolder.path, dir);

	ret = pthread_create(&tid, NULL, play_task, (void *)tp);
	if(ret != 0) {
		perror("Error creating task video\n");
		exit(1);
	}
	tp->tid = tid;
}

int	main ()
{

	start_Calibration(30);

	init();
	draw_interface();

	create_PlayTask(&tp_play1, "Bunny",
			"Video1/f_", 379, 0, 0);
	create_PlayTask(&tp_play2, "Earth",
			"Video2/f_", 1440, 336, 3);

	create_PlotTask();


	allegro_exit();

	return EXIT_SUCCESS;
}
