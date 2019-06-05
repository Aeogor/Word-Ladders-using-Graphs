/* mytimer.c */

//
// Functions for getting the time 
// difference between two points in
// a program. A common use is to time
// how long a function or algorithm takes.
//
// Joshua Castor
// Windows 10 Home Edition with GCC 5.4.0 (for Cygwin)
// U. of Illinois, Chicago
// CS251, Fall 2016
// Based off of code given by Prof. Joe Hummel
// 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "timer.h"

static clock_t myTimerStart = 0;
static clock_t myTimerEnd = 0;

void timer_start()
{
	myTimerStart = clock();
}

void timer_stop()
{
	myTimerEnd = clock();
}

double timer_value()
{
	return (((double) myTimerEnd) - ((double) myTimerStart)) / CLOCKS_PER_SEC;
}

void timer_stats(char* message)
{
	if(!message)	
	  printf("Time: %lf seconds\n", timer_value());
	else
	  printf("%s%lf seconds\n", message, timer_value());
}
