/*
 ============================================================================
 Name        : RTVideo.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <time.h>
#include "Draw.h"
#include "Periodicity.h"

#define LOOP			1

// Fake period of the main
#define MAIN_PERIOD		50

// Resolution for videos
#define VIDEO_HEIGHT	200
#define VIDEO_WIDTH		320

// Structures for the 2 tasks that play videos
task_par tp_play1;
task_par tp_play2;


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
// Function creates a task to play Video
// @parma namevideo is the name of the video to load
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
struct timespec t, count, now, old;

	init();
	draw_interface();

	memset(&count, 0, sizeof(count));
	create_PlayTask(&tp_play1, "Bunny",
			"Video1/f_", 379, 0, 0);
	create_PlayTask(&tp_play2, "Earth",
			"Video2/f_", 1440, 336, 3);

	// Main is a fake periodic task
	// It starts to count time in a period of 50ms
	// When it is active.
	clock_gettime(CLOCK_MONOTONIC, &t);
	do {
		time_add_ms(&t, MAIN_PERIOD);
	//	printf("count %ld.%ld\n", count.tv_sec, count.tv_nsec);
		memset(&count, 0, sizeof(count));
		memset(&now, 0, sizeof(now));
		memset(&old, 0, sizeof(old));

		do {
			// If needed because first time add now to count
			// & old is 0 ---> count = now (WRONG)
			if(old.tv_sec > 0 && old.tv_nsec > 0)
				add_timespec(&count, (now.tv_sec - old.tv_sec),
							(now.tv_nsec - old.tv_nsec));

			time_copy(&old, &now);
			clock_gettime(CLOCK_MONOTONIC, &now);
		} while (time_cmp(&now, &t) < 0);
	} while (LOOP);

	allegro_exit();

	return EXIT_SUCCESS;
}
