#include <unistd.h> 

#ifndef ELEVATOR_CENTRAL_H_
#define ELEVATOR_CENTRAL_H_

// Functions

void initGPIOConfig();
void updateDir(int dir1, int dir2, unsigned char id);
void updatePow(int *uart0_filestream, int pow, unsigned char id);
void waitForResponse(int *uart0_filestream, useconds_t intervalo);
void calibrateFloor(int *uart0_filestream, unsigned char id, unsigned char floor, unsigned char id_message);
void calibrateAllFloors(int *uart0_filestream, unsigned char id);
void calibrateElevators(int *uart0_filestream);
bool waitForFloorReached(unsigned char floor, useconds_t timeout);
void waitAndProcessMessages(int *uart0_filestream, useconds_t intervalo);
bool performInitialCalibration(int *uart0_filestream, unsigned char id, unsigned char floor, useconds_t intervalo);
void calibrate(int *uart0_filestream, unsigned char id, unsigned char floor, unsigned char id_message);
void handleReadButtons(int *uart0_filestream);
void handleWriteButtons(int *uart0_filestream, unsigned char id_button, unsigned char button_value);
void readEncoders(int *uart0_filestream);
void moveElevator(int *uart0_filestream, int pwm, int id_elevator);
void coordinate_movement_elevator(int *uart0_filestream);
void verify_elevator_arrival(int *uart0_filestream);
void handleElevators(int *uart0_filestream);

#endif /* ELEVATOR_CENTRAL_H_ */
