#include "threads.h"
#include "os.h"
#include "malloc.h"
#include "reg.h"
#include "task_queue.c"


#define THREAD_PSP	0xFFFFFFFD


tcb_t tasks[MAX_TASKS];
int lastTask;
int count_in_use_thread = 0;
int scheduled_count = -1;
static int first = 1;


void scheduler_queue(){

	for(int j = 0;j<MAX_TASKS;j++){
		tasks[j].scheduled = 0;

	}

	for(int k = 0;k<count_in_use_thread;k++){//push all in_use to queue

		int max_priority = -1;
		int enqueue_task_ID = -1;

		for(int i =0;i<MAX_TASKS;i++){
			if((tasks[i].in_use==1) && (tasks[i].priority > max_priority) && (tasks[i].scheduled == 0) ){
				max_priority = tasks[i].priority;
				enqueue_task_ID = i;
			}
		}
		Enqueue_task(enqueue_task_ID);
		tasks[enqueue_task_ID].scheduled = 1;
	}

}

/* FIXME: Without naked attribute, GCC will corrupt r7 which is used for stack
 * pointer. If so, after restoring the tasks' context, we will get wrong stack
 * pointer.
 */
void __attribute__((naked)) pendsv_handler()
{
	/* Save the old task's context */
	asm volatile("mrs   r0, psp\n"
	             "stmdb r0!, {r4-r11, lr}\n");
	/* To get the task pointer address from result r0 */
	asm volatile("mov   %0, r0\n" : "=r" (tasks[lastTask].stack));

	/*create Scheduler queue*/
	if( scheduled_count > count_in_use_thread){
		scheduler_queue();
		scheduled_count = 1;
	}

	/* Find a new task to run */
	while (1) {

		    lastTask = _Front_task();
			Dequeue_task();
			scheduled_count++;

			/* Move the task's stack pointer address into r0 */
			asm volatile("mov r0, %0\n" : : "r" (tasks[lastTask].stack));

			/* Restore the new task's context and jump to the task */
			asm volatile("ldmia r0!, {r4-r11, lr}\n"
			             "msr psp, r0\n"
			             "bx lr\n");
	}
}

void systick_handler()
{
	*SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void thread_start()
{
	scheduler_queue();
	scheduled_count = 1;
	lastTask = _Front_task();
	Dequeue_task();
	scheduled_count++;

	/* Save kernel context */
	asm volatile("mrs ip, psr\n"
	             "push {r4-r11, ip, lr}\n");

	/* To bridge the variable in C and the register in ASM,
	 * move the task's stack pointer address into r0.
	 * http://www.ethernut.de/en/documents/arm-inline-asm.html
	 */

	asm volatile("mov r0, %0\n" : : "r" (tasks[lastTask].stack));

	/* Load user task's context and jump to the task */
	asm volatile("msr psp, r0\n"
	             "mov r0, #3\n"
	             "msr control, r0\n"
	             "isb\n"
	             "pop {r4-r11, lr}\n"
	             "pop {r0}\n"
	             "bx lr\n");
}

int thread_create(void (*run)(void *),char *thread_name, void *userdata,int priority)
{
	/* Find a free thing */
	int threadId = 0;
	uint32_t *stack;

	for (threadId = 0; threadId < MAX_TASKS; threadId++) {
		if (tasks[threadId].in_use == 0)
			break;
	}

	if (threadId == MAX_TASKS)
		return -1;

	/* Create the stack */
	stack = (uint32_t *) malloc(STACK_SIZE * sizeof(uint32_t));
	tasks[threadId].orig_stack = stack;

	if (stack == 0)
		return -1;


	stack += STACK_SIZE - 32; /* End of stack, minus what we are about to push */
	if (first) {
		stack[8] = (unsigned int) run;
		stack[9] = (unsigned int) userdata;
		first = 0;
	} else {
		stack[8] = (unsigned int) THREAD_PSP;
		stack[9] = (unsigned int) userdata;
		stack[14] = (unsigned) &thread_self_terminal;
		stack[15] = (unsigned int) run;
		stack[16] = (unsigned int) 0x21000000; /* PSR Thumb bit */
	}

	/* Construct the control block */
	tasks[threadId].stack = stack;
	tasks[threadId].in_use = 1;
	tasks[threadId].thread_name = thread_name;
	tasks[threadId].priority = priority;
	tasks[threadId].thread_tID = threadId;
	count_in_use_thread++;

	return threadId;
}

void thread_kill(int thread_id)
{
	tasks[thread_id].in_use = 0;

	/* Free the stack */
	free(tasks[thread_id].orig_stack);
}

void thread_self_terminal()
{
	/* This will kill the stack.
	 * For now, disable context switches to save ourselves.
	 */
	asm volatile("cpsid i\n");
	thread_kill(lastTask);
	asm volatile("cpsie i\n");

	/* And now wait for death to kick in */
	while (1);
}
