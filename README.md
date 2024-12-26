Dynamic Thread Scheduler

Developed a dynamic thread scheduler library with an API akin to the POSIX pthread library, providing the capability to create threads and enable cooperative concurrent execution among them.

Also have implemented an automatic context switch (e.g., every second) from one thread to another without the need for the user threads to call scheduler_yield().
