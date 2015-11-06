#include <stdio.h>
#include "Draw.h"

graph workload;
int start_time;

//.............................................................................
// Function for init Allegro stuffs
//.............................................................................

void init()
{
	allegro_init();
	install_keyboard();
	install_mouse();
	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED,
				SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	clear_to_color(screen, BLACK);
}

//.............................................................................
// Function for drawing axis Y
//.............................................................................

void draw_axisY(int x, int y)
{
	line(screen, x, y - HEIGHT_AXIS_Y, x, y, GOLD);
	triangle(screen, x, y - TIP_AXIS_Y,
			x + BASE_AXIS, y - HEIGHT_AXIS_Y,
			x - BASE_AXIS, y - HEIGHT_AXIS_Y,
			GOLD);
}

//.............................................................................
// Function for drawing axis X
//.............................................................................

void draw_axisX(int x, int y)
{
	line(screen, x, y, x + HEIGHT_AXIS_X, y, GOLD);
	triangle(screen, x + TIP_AXIS_X, y,
			x + HEIGHT_AXIS_X, y - BASE_AXIS,
			x + HEIGHT_AXIS_X, y + BASE_AXIS,
			GOLD);
}

//.............................................................................
// Function for drawing a scale on y. Axes y is in %, every 2 pixel there's 1%
//.............................................................................

void draw_scaleY()
{
int 	i;
char 	uinteger[4];

	for (i=10; i<=100; i+=10) {
		// Drawing a ref value on the axis
		line(screen, ORIGIN_X, ORIGIN_Y - i * 2,
				ORIGIN_X + 3, ORIGIN_Y - i * 2,
				GOLD);
		// Drawing a mark on the axis
		sprintf(uinteger, "%d", i);
		textout_ex(screen, font, uinteger,
				ORIGIN_X - 27, ORIGIN_Y - (i * 2) - 5,
				WHITE, BLACK);
	}
}

//.............................................................................
// Function for drawing a scale on x. Axes x is in sec, every 4 pixel
// there's 50ms
//.............................................................................

void draw_scaleX()
{
int		i;
char	uinteger[4];
int step = SCALE_X * (1000 / FAKE_PERIOD); // It's where is one second

	for (i=ORIGIN_X + step; i<=HEIGHT_AXIS_X; i+=step) {
		// Drawing a ref value on the axis
		line(screen, i, ORIGIN_Y,
				i, ORIGIN_Y - 3, GOLD);
		// Drawing a mark on the axis
		if( start_time < 100) {
			sprintf(uinteger, "%d", start_time);
			textout_ex(screen, font, uinteger,
				i, ORIGIN_Y + 5,
				WHITE, BLACK);
			start_time++;
		}
	}
}

//.............................................................................
// Function for drawing a cardinal axes for the Workload function
//.............................................................................

void draw_cardinal_axes()
{
	// Drawing axes
	draw_axisY(ORIGIN_X, ORIGIN_Y);
	draw_axisX(ORIGIN_X, ORIGIN_Y);

	// Drawing a scale for axes
	draw_scaleY();
	draw_scaleX();

	// Print info axes on screen
	textout_ex(screen, font, "Workload (%)",
			ORIGIN_X + 10, ORIGIN_Y - HEIGHT_AXIS_Y - 3,
			WHITE, BLACK);
	textout_ex(screen, font, "t (s)",
			ORIGIN_X + HEIGHT_AXIS_X + 10, ORIGIN_Y - 5,
			WHITE, BLACK);
}

//.............................................................................
// Function for drawing interface
//   ___________
//  |___|___|___|
//	|___|___|___|
//	| workload  |
//	|_axes_here_|
//.............................................................................

void draw_interface()
{
	// Drawing 2 rows

	line(screen, 0, 0, MAX_WIDTH, 0, BLUE);
	line(screen, 0, START_OVERLOAD_SCREEN - 1,
			MAX_WIDTH, START_OVERLOAD_SCREEN - 1,
			BLUE);
	line(screen, 0, START_GRAPH_SCREEN - 1,
			MAX_WIDTH,START_GRAPH_SCREEN - 1,
			BLUE);
	line(screen, 0, MAX_HEIGHT,
			MAX_WIDTH, MAX_HEIGHT,
			BLUE);

	//  Drawing 3 columns

	line(screen, 0, 0, 0, MAX_HEIGHT, BLUE);
	line(screen, MAX_WIDTH, 0,
			MAX_WIDTH, MAX_HEIGHT,
			BLUE);
	line(screen, COLUMN, 0, COLUMN,
			START_GRAPH_SCREEN - 1, BLUE);
	line(screen, COLUMN * 2, 0,
			COLUMN * 2,
			START_GRAPH_SCREEN - 1, BLUE);

	// Drawing axes for Workload function
	start_time = 1;
	draw_cardinal_axes();
}

//.............................................................................
// Function for creating text info of a video loaded on (x,y) in the screen
// It has this format: "status:value"
//.............................................................................

void text_state(int x, int y, char *status, int value)
{
char state[DIM_STATUS] = "\0";
char integer[DIM_INT];

		strcat(state, status);

		sprintf(integer,"%d", value);
		strcat(state, integer);

		textout_ex(screen, font, state,
					x, y,
					WHITE, BLACK);
}

//.............................................................................
// Function for drawing a new point on the graph
//.............................................................................

void draw_point(int x, int y)
{
int i = workload.index;

	workload.points[i].x = x;
	workload.points[i].y = ORIGIN_Y - (y * SCALE_Y);

	if(i == 0)
		putpixel(screen,
				x, ORIGIN_Y - (y * SCALE_Y),
				RED);
	else
		line(screen, x, ORIGIN_Y - (y * SCALE_Y),
				workload.points[i - 1].x,
				workload.points[i - 1].y,
				RED);

	workload.index++;
}

//.............................................................................
// Function for cleaning the workload graph
//.............................................................................

void clean_graph()
{
	rectfill(screen,
			MAX_WIDTH, MAX_HEIGHT,
			0, START_GRAPH_SCREEN, BLACK);
	draw_cardinal_axes();
	workload.index = 0;
}
