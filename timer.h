/* mytimer.h */

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

void timer_start();
void timer_stop();
double timer_value();
void timer_stats(char* message);
