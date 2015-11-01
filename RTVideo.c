/*
 ============================================================================
 Name        : RTVideo.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <allegro.h>
#include <pthread.h>
#include <time.h>
#include "Draw.h"
#include "Periodicity.h"

#define MAIN_PERIOD		50
#define LOOP			1
#define VIDEO_HEIGHT	200
#define VIDEO_WIDTH		320

struct info_folder {
	char path[10];	// Path frame folder
	int nframes;	// Number of frames in the folder
	int x_window;	// For Display the video in a
	int y_window;	// certain Window
};

struct info_folder Ifolder1;
struct info_folder Ifolder2;

//.............................................................................
// Body of a task that plays a video on the screen
//.............................................................................

void	*play_task(void *p)
{
int		i = 1, period = 33;
char 	filename[18];
char 	nframe[5];
BITMAP	*bmp;
struct timespec t;
struct info_folder *Ifolder = (struct info_folder *)p;

		clock_gettime(CLOCK_MONOTONIC, &t);
		time_add_ms(&t, period);
		while(LOOP) {
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
			textout_ex(screen, font, "Video1",
					Ifolder->x_window + 20, 200,
					WHITE, BLACK);
			clock_nanosleep(CLOCK_MONOTONIC,
							TIMER_ABSTIME, &t, NULL);
			time_add_ms(&t, period);

			i++;

			if(i == (Ifolder->nframes + 1))
				i = 1;
		}

	pthread_exit(NULL);
}

//.............................................................................
// Function creates a task to play Video
// Ifold is the struct to memorize all useful parameters
// dir is the path where are located the frames of the video
// nframes is the number of frames in the directory
// x & y are the coordinates where to display the video
//.............................................................................

void 	create_PlayTask(struct info_folder* Ifold, char * dir, int nframes,
						int x, int y)
{
int ret;
pthread_t tid;

	Ifold->nframes	= nframes;
	Ifold->x_window	= x;
	Ifold->y_window	= y;
	strcpy(Ifold->path, dir);

	ret = pthread_create(&tid, NULL, play_task, (void *)Ifold);
	if(ret != 0) {
		perror("Error creating task video\n");
		exit(1);
	}
}

int		main ()
{
struct timespec t, count, now, now_old;

	init();
	draw_interface();

	memset(&count, 0, sizeof(count));
	create_PlayTask(&Ifolder1, "Video1/f_", 379, 0, 0);
	create_PlayTask(&Ifolder2, "Video2/f_", 1440, 336, 3);
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
