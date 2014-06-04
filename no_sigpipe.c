/* no_sigpipe.c - ignore sigpipe signals -- handle the error synchronously */

#include "commonincludes.h"

void
no_sigpipe(void)
{
	struct sigaction action;


	action.sa_handler = SIG_IGN;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if (sigaction(SIGPIPE, &action, NULL)  < 0) {
		perror("sigaction SIGPIPE");
		exit(EXIT_FAILURE);
	}
}	/* no_sigpipe */

/* vi: set autoindent tabstop=8 shiftwidth=8 : */
