#include <utility>
#include "define.hpp"

#ifndef SIGNAL_HANDLER_H_
#define SIGNAL_HANDLER_H_

bool verifyRxCRC(unsigned char *rx_buffer, int rx_buffer_size);
int readFromUART(int *uart0_filestream, unsigned char *rx_buffer, int offset, int length);
void handleTemperaturePWMMessage(int *uart0_filestream, unsigned char *rx_buffer, int &rx_buffer_size, const char *msg); 
void handleEncoderMessage(int *uart0_filestream, unsigned char *rx_buffer, int &rx_buffer_size); 
void updateElevatorPosition(int position); 
void updateElevatorFloorPosition(int position); 
void handleButtonMessage(int *uart0_filestream, unsigned char *rx_buffer, int &rx_buffer_size); 
void updateButtonStates(unsigned char *rx_buffer);
std::pair<unsigned char *, int> handleTxBuffer(Mensagem msg);
int fillAdditionalData(const Mensagem &msg, unsigned char *tx_msg, int tx_buffer_size);
void handleMessage(int *uart0_filestream, unsigned char *rx_buffer);
std::pair<unsigned char *, int> handleTxBuffer(Mensagem msg);

#endif 