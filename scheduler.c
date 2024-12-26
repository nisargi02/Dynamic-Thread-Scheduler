/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * scheduler.c
 */

#undef _FORTIFY_SOURCE

#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <setjmp.h>
#include "system.h"
#include "scheduler.h"


/**
 * Needs:
 *   setjmp()
 *   longjmp()
 */

/* research the above Needed API and design accordingly */

struct thread {
    jmp_buf ctx;

    struct {
        char *memory; /* aligned memory */
        char *memory_initial;
    } stack;

    struct {
        void *arg;
        scheduler_fnc_t fnc;
    } code;

    enum {
        INIT,
        RUNNING,
        SLEEPING,
        TERMINATED
    } status;

    struct thread* link;
};

struct thread *head, *curr_thread;
jmp_buf ctx;

int scheduler_create(scheduler_fnc_t fnc, void *arg) {
    size_t pg_size = page_size();
    struct thread *new_thread = (struct thread*)malloc(sizeof(struct thread));

    if(!new_thread) {
        EXIT("Error creating new thread memory");
    }

    new_thread->status = INIT;
    new_thread->code.fnc = fnc;
    new_thread->code.arg = arg;
    new_thread->stack.memory_initial = (void *) malloc(1024*1024);

    if(!new_thread->stack.memory_initial) {
        EXIT("Error creating stack memory");
    }

    new_thread->stack.memory = memory_align(new_thread->stack.memory_initial, pg_size);
    new_thread->link = NULL;

    if(head == NULL) {
        head = new_thread;
        curr_thread = new_thread;
    }
    else {
        struct thread *curr = head;
        new_thread->link = curr;
        head = new_thread;
    }

    return 0;
}

/* get new thread after yield */
static struct thread* get_candidate(void) {
    struct thread *curr = curr_thread->link;

    while(1) {
        if(curr && (curr->status == INIT || curr->status == SLEEPING)) {
            return curr;
        }

        if(curr == NULL) {
            curr = head;
        } 
        else {
            curr = curr->link;
        }

        if(curr_thread && curr == curr_thread && curr_thread->status == TERMINATED) {
            printf("\nAll processes terminated\n");
            return NULL;
        }
    }

}

/* get the candidate and longjmp() */
static void schedule(void) {
    struct thread *candidate = get_candidate();

    if(!candidate) {
        return;
    }
    else {
        uint64_t rsp;
        size_t pagesize;
        pagesize = page_size();
        curr_thread = candidate;
        if(candidate->status == INIT) {
            rsp = (uint64_t)curr_thread->stack.memory+pagesize;
            __asm__ volatile("mov %[rs], %%rsp \n" : [rs] "+r"(rsp)::);
            curr_thread->status = RUNNING;
            curr_thread->code.fnc(candidate->code.arg);
            curr_thread->status = TERMINATED;
            longjmp(ctx, 1);
        } 
        else if(candidate->status == SLEEPING) {
            curr_thread->status = RUNNING;
            longjmp(curr_thread->ctx, 1);
        }
    }
}

/* free memory at the end */
static void destroy(void) {
    struct thread *temp, *curr;
    temp = head;
    while(temp != NULL) {
        FREE(temp->stack.memory_initial);
        curr = temp;
        temp = temp->link;
        FREE(curr);
    }
    curr_thread = NULL;
    head = NULL;
}

void scheduler_execute(void) {
    setjmp(ctx);
    schedule();
    destroy();
}

void scheduler_yield(void) {
    if(!setjmp(curr_thread->ctx)) {
        curr_thread->status = SLEEPING;
        longjmp(ctx, 1);
    }
    return;
}