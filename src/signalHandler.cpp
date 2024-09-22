#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <queue>
#include <mutex>
#include <set>

#include "define.hpp"
#include "crc16.hpp"

std::mutex mtx_chamadas_elevador;

// Verifica o CRC do buffer recebido
bool verifyRxCRC(unsigned char *rx_buffer, int rx_buffer_size)
{
    short recvCRC = calculaCRC(rx_buffer, rx_buffer_size);
    unsigned char recv_crc[2];
    std::memcpy(recv_crc, &recvCRC, 1);
    recvCRC = recvCRC >> 8;
    std::memcpy(recv_crc + 1, &recvCRC, 1);
    return ((*(rx_buffer + rx_buffer_size) == recv_crc[0]) && (*(rx_buffer + rx_buffer_size + 1) == recv_crc[1]));
}

// Lê dados do UART
int readFromUART(int *uart0_filestream, unsigned char *rx_buffer, int offset, int length)
{
    return read(*uart0_filestream, (void *)(rx_buffer + offset), length);
}

// Lida com mensagens relacionadas ao retorno de temperatura e PWM
void handleTemperaturePWMMessage(int *uart0_filestream, unsigned char *rx_buffer, int &rx_buffer_size, const char *msg)
{
    int rx_length = readFromUART(uart0_filestream, rx_buffer, 3, 7);
    rx_buffer_size += (rx_length - 7);
    if (verifyRxCRC(rx_buffer, rx_buffer_size))
    {
        printf("Mensagem recebida: %s\n", msg);
    }
    else
    {
        printf("Crc recebido nao valido\n");
    }
}

// Atualiza a posição dos andares dos elevadores
void updateElevatorFloorPosition(int position)
{
    switch (filaMensagensEnviadas.front())
    {
    case REQUEST_GROUND_FLOOR_E1:
        elevador1.terreo = position;
        break;
    case REQUEST_FIRST_FLOOR_E1:
        elevador1.prmAndar = position;
        break;
    case REQUEST_SECOND_FLOOR_E1:
        elevador1.sgdAndar = position;
        break;
    case REQUEST_THIRD_FLOOR_E1:
        elevador1.tcrAndar = position;
        break;
    case REQUEST_GROUND_FLOOR_E2:
        elevador2.terreo = position;
        break;
    case REQUEST_FIRST_FLOOR_E2:
        elevador2.prmAndar = position;
        break;
    case REQUEST_SECOND_FLOOR_E2:
        elevador2.sgdAndar = position;
        break;
    case REQUEST_THIRD_FLOOR_E2:
        elevador2.tcrAndar = position;
        break;
    default:
        break;
    }
    printf("Elevador chegou aqui: %d\n", position);
}

// Atualiza a posição dos elevadores com base na mensagem recebida
void updateElevatorPosition(int position)
{
    // Verifica a mensagem enviada e atualiza a posição do elevador correspondente
    if (filaMensagensEnviadas.front() == REQUEST_ENCODER_E1)
    {
        elevador1.pos = position;
        printf("Elevador 1 chegou aqui: %d\n", elevador1.pos);
    }
    else if (filaMensagensEnviadas.front() == REQUEST_ENCODER_E2)
    {
        elevador2.pos = position;
        printf("Elevador 2 chegou aqui:%d\n", elevador2.pos);
    }
    // Atualiza posição do elevador de acordo com o andar solicitado
    updateElevatorFloorPosition(position);
}

// Lida com mensagens relacionadas aos encoders
void handleEncoderMessage(int *uart0_filestream, unsigned char *rx_buffer, int &rx_buffer_size)
{
    int rx_length = readFromUART(uart0_filestream, rx_buffer, 3, 6);
    rx_buffer_size += (rx_length - 2);
    if (verifyRxCRC(rx_buffer, rx_buffer_size))
    {
        printf("Encoder lido com sucesso\n");
        int ret;
        memcpy(&ret, rx_buffer + 3, 4);
        // Atualiza a posição dos elevadores
        updateElevatorPosition(ret);
    }
    else
    {
        printf("Crc do encoder invalido\n");
    }
}

// Atualiza o estado dos botões dos elevadores
void updateButtonStates(unsigned char *rx_buffer)
{
    Botoes *recv = (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? &bts_elevador1 : &bts_elevador2);
    std::tuple<unsigned char *, int, int> it_list[] = {
        {&recv->bt_terreo_up, PRIORITY_BTN_GND_UP, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_GND_UP : E2_BTN_GND_UP)},
        {&recv->bt_prm_down, PRIORITY_BTN_FST_DOWN, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_FST_DOWN : E2_BTN_FST_DOWN)},
        {&recv->bt_prm_up, PRIORITY_BTN_FST_UP, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_FST_UP : E2_BTN_FST_UP)},
        {&recv->bt_sgd_down, PRIORITY_BTN_SCD_DOWN, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_SCD_DOWN : E2_BTN_SCD_DOWN)},
        {&recv->bt_sgd_up, PRIORITY_BTN_SCD_UP, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_SCD_UP : E2_BTN_SCD_UP)},
        {&recv->bt_tcr_down, PRIORITY_BTN_THR_DOWN, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_THR_DOWN : E2_BTN_THR_DOWN)},
        {&recv->bt_emerg, PRIORITY_BTN_EMERGENC, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_EMERGENC : E2_BTN_EMERGENC)},
        {&recv->bt_terreo, PRIORITY_BTN_GROUND, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_GROUND : E2_BTN_GROUND)},
        {&recv->bt_prm, PRIORITY_BTN_FIRST, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_FIRST : E2_BTN_FIRST)},
        {&recv->bt_sgd, PRIORITY_BTN_SECOND, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_SECOND : E2_BTN_SECOND)},
        {&recv->bt_tcr, PRIORITY_BTN_THIRD, (filaMensagensEnviadas.front() == REQUEST_BUTTONS_E1 ? E1_BTN_THIRD : E2_BTN_THIRD)}
    };

    mtx_chamadas_elevador.lock();

    for (int i = 0; i < sizeof(it_list) / sizeof(it_list[0]); ++i)
    {
        *std::get<0>(it_list[i]) = rx_buffer[i + 2];
        if (rx_buffer[i + 2] && !hashChamadasElevador.count(std::get<2>(it_list[i])))
        {
            filaChamadaElevador.emplace(std::get<1>(it_list[i]), std::get<2>(it_list[i]));
            hashChamadasElevador.emplace(std::get<2>(it_list[i]));
        }
    }

    mtx_chamadas_elevador.unlock();
}


// Lida com a mensagem de botões
void handleButtonMessage(int *uart0_filestream, unsigned char *rx_buffer, int &rx_buffer_size)
{
    int rx_length = readFromUART(uart0_filestream, rx_buffer, 3, (int)(QTD_BUTTONS_READ + 0x01));
    rx_buffer_size += (rx_length - 2);
    if (verifyRxCRC(rx_buffer, rx_buffer_size))
    {
        updateButtonStates(rx_buffer);
    }
    else
    {
        printf("Crc dos botoes invalido\n");
    }
}

// Preenche os dados adicionais conforme o subcódigo
int fillAdditionalData(const Mensagem &msg, unsigned char *tx_msg, int tx_buffer_size)
{
    switch (msg.subCod)
    {
    case 0xD1:
        memcpy(tx_msg + tx_buffer_size, &msg.id, 1);
        tx_buffer_size += 1;
        memcpy(tx_msg + tx_buffer_size, &msg.tmp, 4);
        tx_buffer_size += 4;
        break;
    case 0xC2:
        memcpy(tx_msg + tx_buffer_size, &msg.id, 1);
        tx_buffer_size += 1;
        memcpy(tx_msg + tx_buffer_size, &msg.pwm, 4);
        tx_buffer_size += 4;
        break;
    case 0xC1:
        memcpy(tx_msg + tx_buffer_size, &msg.id, 1);
        tx_buffer_size += 1;
        break;
    default:
        break;
    }

    if (msg.cod == 0x03 || msg.cod == 0x06)
    {
        memcpy(tx_msg + tx_buffer_size, &msg.numBytes, 1);
        tx_buffer_size += 1;
        if (msg.cod == 0x06)
        {
            memcpy(tx_msg + tx_buffer_size, &msg.val, 1);
            tx_buffer_size += 1;
        }
    }

    memcpy(tx_msg + tx_buffer_size, msg.matr, 4);
    tx_buffer_size += 4;
    return tx_buffer_size;
}

// Função que prepara o buffer para transmissão com a mensagem e o CRC
std::pair<unsigned char *, int> handleTxBuffer(Mensagem msg)
{
    int tx_buffer_size = 3;
    unsigned char *tx_msg = (unsigned char *)malloc(TX_BUFFER_SIZE * sizeof(unsigned char));
    tx_msg[0] = msg.endereco;
    tx_msg[1] = msg.cod;
    tx_msg[2] = msg.subCod;

    // Preenche os dados adicionais conforme o subcódigo
    tx_buffer_size = fillAdditionalData(msg, tx_msg, tx_buffer_size);

    short recvCRC = calculaCRC(tx_msg, tx_buffer_size);
    std::memcpy(tx_msg + tx_buffer_size, &recvCRC, 1);
    recvCRC = recvCRC >> 8;
    std::memcpy(tx_msg + tx_buffer_size + 1, &recvCRC, 1);
    tx_buffer_size += 2;

    return {tx_msg, tx_buffer_size};
}

// Função principal que lida com as mensagens recebidas do UART
void handleMessage(int *uart0_filestream, unsigned char *rx_buffer)
{
    int rx_buffer_size = 3;

    if (rx_buffer[0] == 0x00)
    {
        switch (rx_buffer[1])
        {
        case 0x23:
        case 0x16:
            handleTemperaturePWMMessage(uart0_filestream, rx_buffer, rx_buffer_size, "Elevador recebeu PWM/Temperatura");
            break;
        case 0x03:
            handleButtonMessage(uart0_filestream, rx_buffer, rx_buffer_size);
            break;
        default:
            printf("mensagem invalida\n");
            break;
        }
    }

    filaMensagensEnviadas.pop();
}