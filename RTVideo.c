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

#define MAIN_PERIOD		50
#define LOOP			1
#define VIDEO_HEIGHT	200
#define VIDEO_WIDTH		320


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

void load_state(task_par *tp)
{
Info_folder	*Ifolder =	&(tp->Ifolder);

	textout_ex(screen, font, Ifolder->name,
			Ifolder->x_window + 20, 200,
			WHITE, BLACK);

}
//.............................................................................
// Body of a task that plays a video on the screen
//.............................................................................

void *play_task(void *p)
{
int		i = 1;
Info_folder	*Ifolder =	&(((task_par *)p)->Ifolder);

		set_period((task_par *)p);
		while(LOOP) {
			play_video((task_par *)p, i);
			load_state((task_par *)p);
			wait_for_period((task_par *)p);

			i++;

			if (i == (Ifolder->nframes + 1))
				i = 1;
		}

	pthread_exit(NULL);
}

//.............................................................................
// Function creates a task to play Video
// namevideo is the name of the video to load
// tp is the struct to memorize all useful parameters for the task
// dir is the path where are located the frames of the video
// nframes is the number of frames in the directory
// x & y are the coordinates where to display the video
//.............................................................................

void create_PlayTask(task_par *tp, char *namevideo, char *dir,
				int nframes, int x, int y)
{
int			ret;
pthread_t	tid;

	tp->period = 13;
	tp->deadline = 13;
	tp->dmiss = 0;
	tp->priority = 0;

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
struct timespec t, count, now, now_old;

	init();
	draw_interface();

	memset(&count, 0, sizeof(count));
	create_PlayTask(&tp_play1, "Bunny wake up",
			"Video1/f_", 379, 0, 0);
	create_PlayTask(&tp_play2, "Earth Rotating",
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
		memset(&now_old, 0, sizeof(now_old));

		do {
			// If needed because first time add now to count
			// & now_old is 0 ---> count = now (WRONG)
			if(now_old.tv_sec > 0 && now_old.tv_nsec > 0 &&
			  (now.tv_nsec - now_old.tv_nsec) > 0)
				add_timespec(&count, (now.tv_sec - now_old.tv_sec),
							(now.tv_nsec - now_old.tv_nsec));

			time_copy(&now_old, &now);
			clock_gettime(CLOCK_MONOTONIC, &now);
		} while (time_cmp(&now, &t) < 0);
	} while (LOOP);

	allegro_exit();

	return EXIT_SUCCESS;
}
