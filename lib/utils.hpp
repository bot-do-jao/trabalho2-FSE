#include <utility>
#include "define.hpp"

#ifndef UTILS_H_
#define UTILS_H_

void handleAlarm(int sig);
void handleEndSignal(int sig);
float getTemperature(const char *filepath);
void sendTemperaturesAndControlDisplay(int *uart0_filestream);
std::pair<float, float> handleTemperature();

#endif /* UTILS_H_ */