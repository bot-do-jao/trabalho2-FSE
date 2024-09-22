#include <utility>
#include "define.hpp"

#ifndef UART_COMM_H_
#define UART_COMM_H_


void configureUARTOptions(int *uart0_filestream, struct termios *options);
bool openUARTFilestream(int *uart0_filestream);
void displayUARTInitStatus(bool status);
void initUARTConfig(int *uart0_filestream);
void writeDataToUART(int *uart0_filestream, unsigned char *tx_buffer, int tx_buffer_size, int message_id);
void sendMessage(int *uart0_filestream, unsigned char *tx_buffer, int tx_buffer_size, int message_id);
void rxMessage(int *uart0_filestream);
void handleRxMessages(int *uart0_filestream);

#endif /* UART_COMM_H_ */