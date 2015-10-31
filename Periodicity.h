/*
 * Periodicity.h
 *
 *  Created on: Oct 31, 2015
 *      Author: Matteo Rotundo
 */

#ifndef PERIODICITY_H_
#define PERIODICITY_H_

#include <time.h>

extern int time_cmp(struct timespec *t1, struct timespec *t2);
extern void time_add_ms(struct timespec *dst, long int ms);
extern void add_timespec(struct timespec *dst, long int s, long int ns);
extern void time_copy(struct timespec *dst, const struct timespec *src);

#endif /* PERIODICITY_H_ */
