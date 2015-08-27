#include "mySTR.c"
#include "malloc.h"
#include "os.h"


void print(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

struct Node {
	char data[40];
	struct Node* next;
};

// Two glboal variables to store address of front and rear nodes. 
struct Node* front = NULL;
struct Node* rear = NULL;


void Enqueue(const char *str,int length) {
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	if(length == 0){
		strncpy(temp->data,str,strlen(str)); 
	}
	else{
		strncpy(temp->data,str,length); 
	}

	temp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = temp;
		return;
	}
	rear->next = temp;
	rear = temp;
}


void Dequeue() {
	struct Node* temp = front;
	if(front == NULL) {
		print("Queue is empty\n");
		return;
	}
	if(front == rear) {
		front = rear = NULL;
	}
	else {
		front = front->next;
	}
	free(temp);
}

char* Front() {
	if(front == NULL) {
		print("Front is empty\n");
		return "EMPTY";
	}
	return front->data;
}

int isEmpty(){
	if(front == NULL){
		return 1;
	}else{
		return 0;
	}
}