/* print_time.c - function to print out time differences in microseconds */

#include "commonincludes.h"


/* prints the elapsed time between "start" and "done"
 * when called, start is the starting time (or NULL to start at time 0),
 *		done is the finishing time
 *		message is extra text to print after the time (or NULL)
 */
void
print_time( time_type *start, time_type *done, char *mess )
{
	int secs, fraction;
	double delta;

	secs = done->tv_sec;
	fraction = done->fraction_field;
	if (start != NULL) {
		secs -= start->tv_sec;
		fraction -= start->fraction_field;
		if (fraction < 0) {
			secs--;
			fraction += PER_SECOND;
		}
	}
	delta = ((double)secs) + ((double)fraction)/((double)PER_SECOND);
	if (mess != NULL)
		printf("%10.6f: %s\n", delta, mess);
	else
		printf("%10.6f\n", delta);
	fflush(stdout);
}	/* print_time */



double
TimeInterval( time_type *start, time_type *done)
{
	int secs, fraction;
	double delta;

	secs = done->tv_sec;
	fraction = done->fraction_field;
	if (start != NULL) {
		secs -= start->tv_sec;
		fraction -= start->fraction_field;
		if (fraction < 0) {
			secs--;
			fraction += PER_SECOND;
		}
	}
	delta = ((double)secs) + ((double)fraction)/((double)PER_SECOND);
	return delta;
}


/* vi: set autoindent tabstop=8 shiftwidth=8 : */
