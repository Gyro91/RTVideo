#include <allegro.h>

// Colors used
#define BLACK	0
#define BLUE	14
#define WHITE  	65535
#define GOLD makecol16(255,215,0)

// Constants for handling screen
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define MAX_WIDTH 1023
#define MAX_HEIGHT 767

// Constants useful for drawing axes Workload function
#define ORIGIN_X 20
#define ORIGIN_Y 747
#define TIP_AXIS_Y 303
#define HEIGHT_AXIS_Y 300
#define TIP_AXIS_X 953
#define HEIGHT_AXIS_X 950
#define BASE_AXIS 3


//.............................................................................
// Function for init Allegro stuffs
//.............................................................................

void init()
{
	allegro_init();
	install_keyboard();
	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED,
				SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	clear_to_color(screen, BLACK);
}

void draw_axisY(int x, int y)
{
	line(screen, x, y - HEIGHT_AXIS_Y, x, y, GOLD);
	triangle(screen, x, y - TIP_AXIS_Y,
			x + BASE_AXIS, y - HEIGHT_AXIS_Y,
			x - BASE_AXIS, y - HEIGHT_AXIS_Y,
			GOLD);
}


void draw_axisX(int x, int y)
{
	line(screen, x, y, x + HEIGHT_AXIS_X, y, GOLD);
	triangle(screen, x + TIP_AXIS_X, y,
			x + HEIGHT_AXIS_X, y - BASE_AXIS,
			x + HEIGHT_AXIS_X, y + BASE_AXIS,
			GOLD);
}

void draw_cardinal_axes()
{
	draw_axisY(ORIGIN_X, ORIGIN_Y);
	draw_axisX(ORIGIN_X, ORIGIN_Y);

	textout_ex(screen, font, "Workload",
			ORIGIN_X + 10, ORIGIN_Y - HEIGHT_AXIS_Y,
			WHITE, BLACK);
	textout_ex(screen, font, "Time",
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
	line(screen, 0, 215, MAX_WIDTH, 215, BLUE);
	line(screen, 0, 430, MAX_WIDTH, 430, BLUE);
	line(screen, 0, MAX_HEIGHT, MAX_WIDTH, MAX_HEIGHT, BLUE);

	//  Drawing 3 columns

	line(screen, 0, 0, 0, MAX_HEIGHT, BLUE);
	line(screen, MAX_WIDTH, 0, MAX_WIDTH, MAX_HEIGHT, BLUE);
	line(screen, 335, 0, 335, 430, BLUE);
	line(screen, 670, 0, 670, 430, BLUE);

	// Drawing axes for Workload function

	draw_cardinal_axes();
}
