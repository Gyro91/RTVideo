/*
 ============================================================================
 Name        : Drah.h
 Author      : Matteo Rotundo
 ============================================================================
 */

#ifndef DRAW_H_
#define DRAW_H_
#include <allegro.h>

// Colors used
#define BLACK	0
#define BLUE	14
#define WHITE  	65535
#define GOLD 	makecol16(255, 215, 0)
#define RED 	makecol16(255, 0, 0)
#define RANDOM	makecol(rand()%255, rand()%255, rand()%255)

// Constants for handling screen
#define SCREEN_WIDTH 			1024
#define SCREEN_HEIGHT 			675
#define MAX_WIDTH 				1023
#define MAX_HEIGHT 				674
#define START_OVERLOAD_SCREEN	216
#define COLUMN					335

// Constants useful for drawing axes Workload function
#define START_GRAPH_SCREEN 	431
#define ORIGIN_X 			30
#define ORIGIN_Y			650
#define TIP_AXIS_Y 			213
#define HEIGHT_AXIS_Y 		210
#define TIP_AXIS_X 			943
#define HEIGHT_AXIS_X 		940
#define BASE_AXIS 			3
// How many pixel for 100ms
#define SCALE_X				4
// How many pixel for 1%
#define SCALE_Y				2

// Max dimension for a string that contains an integer
#define DIM_INT		11
// Max dimension for the status text info of a video
#define DIM_STATUS	28

#define FAKE_PERIOD	150

// Struct to define a point on the graph
typedef struct point_ {
	int x;
	int y;
} point;

// Struct to define a graph
typedef struct graph_ {
	int index;		// Actual point drawn
	point points[HEIGHT_AXIS_X];
} graph;

extern void draw_interface();
extern void init();
extern void draw_axisX(int, int);
extern void draw_axisY(int, int);
extern void text_state(int x, int y, char *status, int value);
extern void draw_point(int x, int y);
extern void clean_graph();

#endif /* DRAW_H_ */
