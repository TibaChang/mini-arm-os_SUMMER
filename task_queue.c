#include <stddef.h>
#include "malloc.h"
#include "os.h"



struct tNode {
	int tID;
	struct tNode* next;
};

// Two glboal variables to store address of front_task and rear_task tNodes. 
struct tNode* front_task = NULL;
struct tNode* rear_task = NULL;


void Enqueue_task(int tID) {
	struct tNode* temp = 
		(struct tNode*)malloc(sizeof(struct tNode));
	temp->tID = tID;

	temp->next = NULL;
	if(front_task == NULL && rear_task == NULL){
		front_task = rear_task = temp;
		return;
	}
	rear_task->next = temp;
	rear_task = temp;
}


void Dequeue_task() {
	struct tNode* temp = front_task;
	if(front_task == NULL) {
		//print("Queue is empty\n");
		return;
	}
	if(front_task == rear_task) {
		front_task = rear_task = NULL;
	}
	else {
		front_task = front_task->next;
	}
	free(temp);
}

int _Front_task() {
	if(front_task == NULL) {
		//print("front_task is empty\n");
		return -1;
	}
	return front_task->tID;
}

int isEmpty_task(){
	if(front_task == NULL){
		return 1;
	}else{
		return 0;
	}
	return 0;
}