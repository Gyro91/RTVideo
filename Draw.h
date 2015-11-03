/*
 * Draw.h
 *
 *  Created on: Oct 31, 2015
 *      Author: Matteo Rotundo
 */

#ifndef DRAW_H_
#define DRAW_H_

// Colors used
#define BLACK		0
#define BLUE		14
#define WHITE  		65535

extern void draw_interface();
extern void init();
extern void draw_axisX(int, int);
extern void draw_axisY(int, int);
extern void text_state(int x, int y, char *status, int value);

#endif /* DRAW_H_ */
