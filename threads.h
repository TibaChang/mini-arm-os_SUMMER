#ifndef __THREADS_H__
#define __THREADS_H__

#include <stdint.h>
#define MAX_TASKS 10

void thread_start();
int thread_create(void (*run)(void*),char *thread_name, void* userdata,int priority);
void thread_kill(int thread_id);
void thread_self_terminal();

/* Thread Control Block */
typedef struct {
	void *stack;
	void *orig_stack;
	uint8_t in_use;
	char *thread_name;
	int thread_tID;
	int priority;
	int scheduled;
} tcb_t;

extern tcb_t tasks[MAX_TASKS];
extern int lastTask;

#endif
