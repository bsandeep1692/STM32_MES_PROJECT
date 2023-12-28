// Console IO is a wrapper between the actual in and output and the console code
// In an embedded system, this might interface to a UART driver.

#include "consoleIo.h"
#include <stdio.h>
//#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal.h"
#include "string.h"
extern UART_HandleTypeDef huart3;
uint8_t ch = 0; // received character
uint8_t rx_bufferio[256]; // buffer for received commands
uint8_t rx_counter = 0; // counter to keep track of buffer position
uint8_t buffer_ready = 0; // flag to determine if a complete command has been received
uint8_t uart_tx_complete = 1; // flag to determine if a complete command has been received
//use the windows conio.h for kbhit, or a POSIX reproduction

//their original
/*static int getch_noblock() {
    if (_kbhit())
        return _getch();
    else
        return EOF;
}*/


eConsoleError ConsoleIoInit(void)
{
	HAL_UART_Receive_IT(&huart3, &ch, 1); // initiate reception
	return CONSOLE_SUCCESS;
}

/* This RX interrupt gets triggered once x number of bytes are recieved*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	//HAL_UART_Transmit_IT(&huart3,&ch,1); // echo

    //TODO: handle cases of buffer overflow gracefully
	rx_bufferio[rx_counter++] = ch; // load each character to buffer
    if(ch == '\r' || ch == '\n')
        buffer_ready = 1; // if a new-line character or a carriage return character is received, set the relevant flag

    HAL_UART_Receive_IT(huart, &ch, 1); // continue receiving character(s)
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uart_tx_complete =1;
}

//this is where the main edits went to make this code portable to STM32:
eConsoleError ConsoleIoReceive(uint8_t* buffer, const uint32_t bufferLength, uint32_t *readLength)
{
    if(buffer_ready) { // if a complete command is received
        buffer_ready = 0; // unset flag
        memcpy(buffer,rx_bufferio,rx_counter); // copy the received command to library buffer
        *readLength = rx_counter; // set appropriate length

        memset(rx_bufferio,0,256); // clear receive buffer
        rx_counter = 0; // clear pointer so we can start from the beginning next time around
    } else {
        buffer[0] = '\n'; // if no command is received, send the '>'
        *readLength = 0;
    }

    return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoSendString(const char *buffer)
{
	uint8_t len = strlen(buffer);
	//uart_tx_complete = 0;
	//HAL_UART_Transmit_IT(&huart3, (uint8_t*)buffer, len);
	HAL_UART_Transmit(&huart3, (uint8_t*)buffer, len,10000);
	//while (uart_tx_complete==0);
	return CONSOLE_SUCCESS;
}

