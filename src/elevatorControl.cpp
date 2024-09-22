#include "define.hpp"
#include "signalHandler.hpp"
#include "uartComm.hpp"

void sendTempInfo(int *uart0_filestream, unsigned char idElevator, float value)
{
    Mensagem msg;
    msg.endereco = 0x01;
    msg.cod = 0x16;
    msg.subCod = 0xD1;
    msg.id = idElevator;
    msg.tmp = value;

    std::pair<unsigned char *, int> tx_msg = handleTxBuffer(msg);
    sendMessage(uart0_filestream, tx_msg.first, tx_msg.second, (idElevator ? SEND_TEMPERATURE_E2 : SEND_TEMPERATURE_E1));

    free(tx_msg.first);
}

void sendPWMInfo(int *uart0_filestream, unsigned char idElevator, int value)
{
    Mensagem msg;
    msg.endereco = 0x01;
    msg.cod = 0x16;
    msg.subCod = 0xC2;
    msg.id = idElevator;
    msg.pwm = value;

    std::pair<unsigned char *, int> tx_msg = handleTxBuffer(msg);
    sendMessage(uart0_filestream, tx_msg.first, tx_msg.second, (idElevator ? SEND_PWM_E2 : SEND_PWM_E1));

    free(tx_msg.first);
}

void sendPWMs(int *uart0_filestream)
{
    sendPWMInfo(uart0_filestream, ADDR_E1, elevador1.pwm);
    sendPWMInfo(uart0_filestream, ADDR_E2, elevador2.pwm);
}

int direcao1 = INT16_MIN;
int direcao2 = INT16_MIN;

void getEncoderInfo(int *uart0_filestream, unsigned char idElevator, unsigned char id_message)
{
    Mensagem msg;
    msg.endereco = 0x01;
    msg.cod = 0x23;
    msg.subCod = 0xC1;
    msg.id = idElevator;

    std::pair<unsigned char *, int> tx_msg = handleTxBuffer(msg);
    sendMessage(uart0_filestream, tx_msg.first, tx_msg.second, id_message);

    free(tx_msg.first);
}