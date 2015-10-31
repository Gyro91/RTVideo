/*
 ============================================================================
 Name        : RTVideo.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <stdlib.h>
#include <allegro.h>
#include <stdio.h>
#include "Draw.h"
#include "Periodicity.h"

#define MAIN_PERIOD 50
#define LOOP 1
#define COUNT_ALONE 2053566




int main ()
{
struct timespec t, count, now, now_old;

	init();
	draw_interface();

	memset(&count, 0, sizeof(count));

	// Main is a fake periodic task
	// It starts to count time in a period of 50ms
	// When it is active.
	clock_gettime(CLOCK_MONOTONIC, &t);
	do {
		time_add_ms(&t, MAIN_PERIOD);
		printf("count %ld.%ld\n", count.tv_sec, count.tv_nsec);
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
