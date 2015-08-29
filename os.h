#ifndef __OS_H_
#define __OS_H_

#define STACK_SIZE 256
#define MAX_HEAPS 4096
#define xMutex_lock 1
#define xMutex_unlock -1
#define xMutex_success 1
#define xMutex_fail 0

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x20)
#define INPUT_STR_MAX 30

typedef struct __Mutex{
	const int lock;
	const int unlock;
	int status;
}xMutex;


#endif
