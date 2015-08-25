#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"
#include "os.h"
#include "malloc.h"
#include "mySTR.c"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x20)
#define INPUT_STR_MAX 255


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
	//USART2_BRR why we don't set it,but it still works?
	//USART2_CR3:DMA need to enable?
}

static void delay(volatile int count)
{
	count *= 50000;
	while (count--);
}



void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

void print_char(const char *str)
{
	if (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
	}
}



int check_commands(const char *commands,uint32_t index)
{
	if(!strncmp(commands,"help",4) && index ==4){
		print_str("command:help\n");
		print_str("Welcome to mini-shell\n");
		print_str("Supported commands:(those NOT belong to \"commands\" will be repeated once on the shell!)\n");
		print_str("help\n");
		print_str("ps");
		return 1;
	}
	else if(!strncmp(commands,"ps",2) && index ==2){
		char * buf = (char *)malloc(2 * sizeof(int));
		print_str("command:ps\n");
		print_str("=====Thread_ID=====Thread_Name=====\n");
		for(int j=0;j<MAX_TASKS &&tasks[j].in_use == 1;j++){
			print_str("         ");
			itoa(tasks[j].thread_tID,buf);
			print_str(buf);
			print_str("             ");
			print_str(tasks[j].thread_name);
			print_str("\n");
		}
		free(buf);
		return 1;
	}
	else{
		return 0;
	}
}

void receive_str()
{
	//const char const *input_str = "Your input:   ";
	char str_buffer[INPUT_STR_MAX];
	int index = 0;
	print_str("\nPlease input something or \"help\"\n");
	print_str("mini-shell:$");
	while(1) {
		if(*(USART2_SR) & USART_FLAG_RXNE) {
			str_buffer[index] =( *(USART2_DR) & 0x1FF);
			if((int)str_buffer[index]==0xD){//'\r' is 0xD in ascii
				print_str("\n");

				if(!check_commands(str_buffer,index)){
					print_str("Your input:");
					for(int j=0;j<index;j++){
						print_char(&str_buffer[j]);
					}
				}
				print_str("\n");
				index = 0;
				print_str("\nmini-shell:$");
			}
			else{
				print_char(&str_buffer[index]);
				index++;
			}
		}
	}
}

static void busy_loop(void *str)
{
	while (1) {
		//print_str(str);
		//print_str(": Running...\n");
		delay(1000);
	}
}


void shell(void *userdata)
{
	receive_str();
}

void test2(void *userdata)
{
	busy_loop(userdata);
}

void test3(void *userdata)
{
	busy_loop(userdata);
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 5 ms per tick. */
#define TICK_RATE_HZ 200

int main(void)
{
	char *str1 = "Shell", *str2 = "Task2", *str3 = "Task3";

	usart_init();

	if (thread_create(shell,str1, (void *) str1) == -1)
		print_str("Shell creation failed\r\n");

	if (thread_create(test2,str2, (void *) str2) == -1)
		print_str("Thread 2 creation failed\r\n");

	if (thread_create(test3,str3, (void *) str3) == -1)
		print_str("Thread 3 creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
