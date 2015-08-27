#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"
#include "os.h"
#include "malloc.h"
#include "mySTR.c"
#include "queue.c"

#define print_addQueue(a,b) Enqueue(a,b)



void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t) (0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00008B00;// old:0x4B00 = 0100 1011 0000 0000; new:1000 1011 0000 0000=0x8B00
	*(GPIOA_CRH) = 0x44444444;//reset value
	*(GPIOA_IDR) = 0x00000000;//reset value
	*(GPIOA_ODR) = 0x00000000;//reset value
	*(GPIOA_BSRR) = 0x00000000;//reset value
	*(GPIOA_BRR) = 0x00000000;//reset value,why we don't set it,but it still works?

	*(USART2_CR1) = 0x0000000C;//Enable TX/RX
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;//0x2000=0010 0000 0000 0000;UE: USART enable
}

/*
static void delay(volatile int count)
{
	count *= 50000;
	while (count--);
}
*/

int acquire_Semaphore(xSemaphore *x){
	if(x->status == xSemaphore_unlock){
		x->status = xSemaphore_lock;
		return xSemaphore_success;
	}else if(x->status == xSemaphore_lock){
		return xSemaphore_fail;
	}
	return xSemaphore_fail;
}

void release_Semaphore(xSemaphore *x){
	x->status = xSemaphore_unlock;
}

xSemaphore *print_Semaphore=&(xSemaphore){
	.lock = xSemaphore_lock,
	.unlock = xSemaphore_unlock,
	.status = xSemaphore_unlock
};


void print_char(const char *str)
{
	if (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
	}
}

void print_str(const char *str)
{
	while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
	release_Semaphore(print_Semaphore);
}

void print_str_specific(const char *str,int count)
{
	while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
	for (int i = 0; i < count; i++){
		print_char(str++);
	}
	release_Semaphore(print_Semaphore);
}


void print_queue(){
	if(!isEmpty()){
		while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
		while(!isEmpty()){
			print(Front());
			Dequeue();
		}
		release_Semaphore(print_Semaphore);
	}
}



int check_commands(const char *commands,uint32_t index)
{
	if(!strncmp(commands,"help",4) && index ==4){
		while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
		print("\nWelcome to mini-shell\n");
		print("Supported commands:\n");
		print("- help\n");
		print("- ps\n");
		print("- \"up arrow\"\n");
		release_Semaphore(print_Semaphore);
		return 1;
	}
	else if(!strncmp(commands,"ps",2) && index ==2){
		while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
		print("\n===tID===Thread_Name====\n");
		for(int j=0;j<MAX_TASKS &&tasks[j].in_use == 1;j++){
			char * buf = (char *)malloc(2 * sizeof(int));
			itoa(tasks[j].thread_tID,buf);
			print("    ");
			print(buf);
			print("      ");
			print(tasks[j].thread_name);
			print("\n");
			free(buf);
		}
		release_Semaphore(print_Semaphore);
		
		return 1;
	}
	else if(!strncmp(commands,"test",4) && index ==4){
		print_addQueue("TEST1\n",6);
		print_addQueue("TEST2\n",6);
		print_addQueue("TEST3\n",6);
		print_addQueue("TEST4\n",6);
		print_addQueue("TEST5\n",6);
		return 1;
	}
	else{
		return 0;
	}
}

void receive_str()
{
	char * history_1 = (char *)malloc(255 * sizeof(char));
	int history_count = 1;
	int history_index = 0;
	int up_count = 0;

	char str_buffer[INPUT_STR_MAX];
	int index = 0;
	print_str("\nInput something or \"help\"\n");
	print_str("mini-shell:$");
	while(1) {
		if(*(USART2_SR) & USART_FLAG_RXNE) {
			str_buffer[index] =( *(USART2_DR) & 0xFF);

			if((int)str_buffer[index]==0xD){//'\r' is 0xD in ascii
				print_char("\n");
				release_Semaphore(print_Semaphore);
				if(!check_commands(str_buffer,index)){
					print_str("Your input:");
					print_str_specific(str_buffer,index);
				}
				/*
				Record input history
				*/
				switch(history_count){
					case 1:
						strncpy(history_1,str_buffer,index);
						history_index = index;
						break;
				}
				history_count = 0;
				history_count++;

				print_str("\n");
				index = 0;
				print_str("\nmini-shell:$");
			}else if(!strncmp(str_buffer,"A",1)){
				up_count++;
				switch(up_count){
					case 1:
						index = history_index;
						strncpy(str_buffer,history_1,history_index);
						print_str_specific(str_buffer,index);
						break;
				}
				up_count = 0;
			}else{
				if(index == 0 ){
					print_str("\nmini-shell:$");
					while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
				}
				print_char(&str_buffer[index]);
				index++;
			}
		}
	}
	free(history_1);
}

/*
static void busy_loop(void *str)
{
	while (1) {
		//print_str(str);
		//print_str(": Running...\n");
		delay(1000);
	}
}
*/

void shell(void *userdata)
{
	receive_str();
}

int fib(int n){
 
    if(n==0)
        return 0;
 
    if(n==1)
        return 1;
 
    return (fib(n-1)+fib(n-2));
 
}

void test_fib(void *userdata)
{
	for (int i = 2; i < 47; ++i)
	{
		char * buf = (char *)malloc(8* sizeof(int));
		itoa(fib(i),buf);
		while(acquire_Semaphore(print_Semaphore) != xSemaphore_success);//busy wait
		print("\nfibonacci seq :");
		print(buf);
		free(buf);
		print(" , \n");
		release_Semaphore(print_Semaphore);
	}
}



void Print_task(void *userdata)
{
	while(1){
		print_queue();
	}
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 2.5 ms per tick. */
#define TICK_RATE_HZ 400

int main(void)
{
	char *str1 = "Shell", *str2 = "Fib_task", *str3 = "Print_task";

	usart_init();

	if (thread_create(shell,str1, (void *) str1) == -1)
		print_str("Shell creation failed\r\n");

	if (thread_create(test_fib,str2, (void *) str2) == -1)
		print_str("Fib_task creation failed\r\n");

	if (thread_create(Print_task,str3, (void *) str3) == -1)
		print_str("Print_task creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
