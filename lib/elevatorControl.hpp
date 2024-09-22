#include <utility>
#include "define.hpp"

#ifndef ELEVATOR_CONTROL_H_
#define ELEVATOR_CONTROL_H_

void sendTempInfo(int *uart0_filestream, unsigned char idElevator, float value);
void sendPWMInfo(int *uart0_filestream, unsigned char idElevator, int value);
void sendPWMs(int *uart0_filestream);
void getEncoderInfo(int *uart0_filestream, unsigned char idElevator, unsigned char id_message);
void sendTemperaturesAndControlDisplay(int *uart0_filestream);
std::pair<float, float> handleTemperature();

#endif /* ELEVATOR_CONTROL_H_ */