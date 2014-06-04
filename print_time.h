/* print_time.h - header file for
 *		  program to print out time differences in microseconds
 */

#ifndef PRINT_TIME_H
#define PRINT_TIME_H

#include <unistd.h>

#if defined(_POSIX_TIMERS) && _POSIX_TIMERS != -1

	/* use the newer Posix high-resolution time facilities */
	/* must be linked on all machines with -lrt */

	#include <time.h>

	#define PER_SECOND	1000000000
	#define time_type	struct timespec
	#define fraction_field	tv_nsec
	#define get_time(where)	clock_gettime(CLOCK_REALTIME, where)

#else

	/* use the older (BSD) high-resolution time facilities */

	#include <sys/time.h>

	#define PER_SECOND	1000000
	#define time_type	struct timeval
	#define fraction_field	tv_usec
	#define get_time(where)	gettimeofday(where, NULL)

#endif



/* prints the elapsed time between "start" and "done"
 * when called, start is the starting time (or NULL to start at time 0),
 *		done is the finishing time
 *		message is extra text to print after the time (or NULL)
 */
extern void
print_time( time_type *before, time_type *done, char *mess );


/* calculate the elapsed time between "start" and "done"
 * when called, start is the starting time (or NULL to start at time 0),
 *		done is the finishing time
 *		
 */
extern double
TimeInterval( time_type *before, time_type *done);


#endif
