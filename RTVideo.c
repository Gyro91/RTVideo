/*
 ============================================================================
 Name        : RTVideo.c
 Author      : Matteo Rotundo
 ============================================================================
 */
#include <stdlib.h>
#include <allegro.h>
#include "Draw.h"


int main ()
{
	init();
	draw_interface();

	readkey();

	allegro_exit();

	return EXIT_SUCCESS;
}
