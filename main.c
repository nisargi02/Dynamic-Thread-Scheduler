/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include "system.h"
#include "scheduler.h"
#include <signal.h> /* for SIGALRM */
#include <unistd.h> /* for alarm(1) */

void signal_handler(int sig) {
	assert(SIGALRM == sig);
	printf("\nSignal Trigerred \n");
	signal(SIGALRM, signal_handler);
	alarm(1);
	scheduler_yield();
}

static void
_thread_(void *arg)
{
	const char *name;
	int i;

	name = (const char *)arg;
	for (i=0; i<100; ++i) {
		printf("%s %d\n", name, i);
		us_sleep(20000);
		/* scheduler_yield(); */
	}
}

int
main(int argc, char *argv[])
{
	alarm(1);
	if(SIG_ERR == signal(SIGALRM, signal_handler)) {
		perror("\nSignal Error \n");
	}

	UNUSED(argc);
	UNUSED(argv);

	if (scheduler_create(_thread_, "hello") ||
	    scheduler_create(_thread_, "world") ||
	    scheduler_create(_thread_, "love") ||
	    scheduler_create(_thread_, "this") ||
	    scheduler_create(_thread_, "course!")) {
		TRACE(0);
		return -1;
	}
	scheduler_execute();
	return 0;
}
