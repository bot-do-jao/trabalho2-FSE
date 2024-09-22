#include <wiringPi.h>
#include <softPwm.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "define.hpp"

#include "uartComm.hpp"
#include "signalHandler.hpp"
#include "elevatorControl.hpp"

#include "pid.hpp"

#include <unistd.h>
#include <math.h>
#include <iostream>

// Variáveis globais para controle do estado e movimento dos elevadores
enum Estado estado;
int pwm, paraOndeIr_1 = INT16_MIN, paraOndeIr_2 = INT16_MIN;

    bool moveElevador1 = false;
bool moveElevador2 = false;
bool altMoveElevador1 = false;
bool altMoveElevador2 = false;

/**
 * @brief Inicializa a configuração dos pinos GPIO para controle dos elevadores.
 * Define os pinos como entradas ou saídas conforme a necessidade e inicializa o PWM.
 */
void initGPIOConfig()
{
    wiringPiSetupGpio();

    pinMode(E1_DIR1, OUTPUT);
    pinMode(E1_DIR2, OUTPUT);
    pinMode(E1_POTM, OUTPUT);
    pinMode(E1_GRND, INPUT);
    pinMode(E1_FIRS, INPUT);
    pinMode(E1_SECN, INPUT);
    pinMode(E1_THIR, INPUT);

    softPwmCreate(E1_POTM, 1, 100);

    pinMode(E2_DIR1, OUTPUT);
    pinMode(E2_DIR2, OUTPUT);
    pinMode(E2_POTM, OUTPUT);
    pinMode(E2_GRND, INPUT);
    pinMode(E2_FIRS, INPUT);
    pinMode(E2_SECN, INPUT);
    pinMode(E2_THIR, INPUT);

    softPwmCreate(E2_POTM, 1, 100);
}

/**
 * @brief Atualiza a direção do movimento dos elevadores.
 * @param dir1 Direção do motor (0 ou 1) para a primeira entrada de direção.
 * @param dir2 Direção do motor (0 ou 1) para a segunda entrada de direção.
 * @param id Identificador do elevador (0 para E1, 1 para E2).
 */
void updateDir(int dir1, int dir2, unsigned char id)
{
    if (id)
    {
        digitalWrite(E2_DIR1, (dir1 ? HIGH : LOW));
        digitalWrite(E2_DIR2, (dir2 ? HIGH : LOW));
    }
    else
    {
        digitalWrite(E1_DIR1, (dir1 ? HIGH : LOW));
        digitalWrite(E1_DIR2, (dir2 ? HIGH : LOW));
    }
}

/**
 * @brief Atualiza a potência do PWM dos elevadores e envia as informações via UART.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param pow Valor do PWM a ser definido.
 * @param id Identificador do elevador (0 para E1, 1 para E2).
 */
void updatePow(int *uart0_filestream, int pow, unsigned char id)
{
    if (id)
        softPwmWrite(E2_POTM, pow);
    else
        softPwmWrite(E1_POTM, pow);

    sendPWMInfo(uart0_filestream, id, pow);
    elevador1.pwm = (id ? elevador1.pwm : pow);
    elevador2.pwm = (id ? pow : elevador2.pwm);
}

/**
 * @brief Espera uma resposta do elevador via UART por um intervalo de tempo especificado.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param intervalo Intervalo de tempo em microsegundos para aguardar.
 */
void waitForResponse(int *uart0_filestream, useconds_t intervalo)
{
    ualarm(intervalo, 0);
    pause();
    rxMessage(uart0_filestream);
}

/**
 * @brief Calibra um andar específico do elevador, ajustando a posição conforme necessário.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id Identificador do elevador (0 para E1, 1 para E2).
 * @param floor Pino correspondente ao andar a ser calibrado.
 * @param id_message Código da mensagem de requisição.
 */
void calibrateFloor(int *uart0_filestream, unsigned char id, unsigned char floor, unsigned char id_message)
{
    useconds_t intervalo = 100 * 1000; // 100 ms
    useconds_t intervalo2 = 50 * 1000; // 50 ms
    bool jobDone = false;

    while (!jobDone)
    {
        // Movimenta o elevador na direção desejada
        updatePow(uart0_filestream, 50, id);
        updateDir(1, 0, id);
        waitForResponse(uart0_filestream, intervalo);

        if (digitalRead(floor))
        {
            // Freia o elevador ao chegar no andar
            updateDir(1, 1, id);
            updatePow(uart0_filestream, 0, id);
            waitForResponse(uart0_filestream, intervalo2);

            if (digitalRead(floor))
            {
                // Obtém informações do encoder para ajuste fino
                getEncoderInfo(uart0_filestream, id, id_message);
                jobDone = true;
            }
        }
        else
        {
            // Muda a direção e aumenta a potência caso não esteja no andar correto
            updateDir(0, 1, id);
            updatePow(uart0_filestream, 100, id);
            waitForResponse(uart0_filestream, intervalo);
        }
    }
}

/**
 * @brief Calibra todos os andares de um elevador específico.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id Identificador do elevador (0 para E1, 1 para E2).
 */
void calibrateAllFloors(int *uart0_filestream, unsigned char id)
{
    useconds_t intervalo = 50 * 1000; // 50 ms

    calibrateFloor(uart0_filestream, id, (id ? E2_GRND : E1_GRND), (id ? REQUEST_GROUND_FLOOR_E2 : REQUEST_GROUND_FLOOR_E1));
    calibrateFloor(uart0_filestream, id, (id ? E2_FIRS : E1_FIRS), (id ? REQUEST_FIRST_FLOOR_E2 : REQUEST_FIRST_FLOOR_E1));
    calibrateFloor(uart0_filestream, id, (id ? E2_SECN : E1_SECN), (id ? REQUEST_SECOND_FLOOR_E2 : REQUEST_SECOND_FLOOR_E1));
    calibrateFloor(uart0_filestream, id, (id ? E2_THIR : E1_THIR), (id ? REQUEST_THIRD_FLOOR_E2 : REQUEST_THIRD_FLOOR_E1));

    updatePow(uart0_filestream, 0, id); // Para o elevador
    waitForResponse(uart0_filestream, intervalo);
    updateDir(0, 0, id); // Desativa o motor
}

/**
 * @brief Realiza a calibração inicial de todos os elevadores.
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void calibrateElevators(int *uart0_filestream)
{
    calibrateAllFloors(uart0_filestream, ADDR_E1); // Calibra todos os andares do elevador 1
    calibrateAllFloors(uart0_filestream, ADDR_E2); // Calibra todos os andares do elevador 2
}

/**
 * @brief Aguarda que o elevador alcance o andar desejado com um timeout.
 * @param floor Pino correspondente ao andar a ser monitorado.
 * @param timeout Tempo máximo de espera em microsegundos.
 * @return true se o andar foi alcançado, false caso contrário.
 */
bool waitForFloorReached(unsigned char floor, useconds_t timeout)
{
    useconds_t start_time = micros();
    while (!digitalRead(floor))
    {
        if (micros() - start_time >= timeout)
            return false;
    }
    return true;
}

/**
 * @brief Aguarda por mensagens via UART e processa-as em um intervalo de tempo especificado.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param intervalo Intervalo de tempo em microsegundos para aguardar.
 */
void waitAndProcessMessages(int *uart0_filestream, useconds_t intervalo)
{
    ualarm(intervalo, 0);
    pause();
    rxMessage(uart0_filestream);
}

/**
 * @brief Realiza a calibração inicial do elevador até alcançar um andar.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id Identificador do elevador (0 para E1, 1 para E2).
 * @param floor Pino correspondente ao andar a ser calibrado.
 * @param intervalo Tempo de espera para mensagens.
 * @return true se o elevador alcançou o andar, false caso contrário.
 */
bool performInitialCalibration(int *uart0_filestream, unsigned char id, unsigned char floor, useconds_t intervalo)
{
    updatePow(uart0_filestream, 50, id); // Inicia o movimento do elevador com 50% do PWM
    updateDir(1, 0, id);
    waitAndProcessMessages(uart0_filestream, intervalo);

    return waitForFloorReached(floor, intervalo);
}

/**
 * @brief Calibra o elevador em um andar específico.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id Identificador do elevador (0 para E1, 1 para E2).
 * @param floor Pino correspondente ao andar a ser calibrado.
 * @param id_message Código da mensagem de requisição.
 */
void calibrate(int *uart0_filestream, unsigned char id, unsigned char floor, unsigned char id_message)
{
    useconds_t intervalo1 = 100 * 1000; // 100 ms
    useconds_t intervalo2 = 50 * 1000;  // 50 ms

    bool reachedFloor = performInitialCalibration(uart0_filestream, id, floor, intervalo1);

    if (reachedFloor)
    {
        // Freia e para o elevador
        updateDir(1, 1, id);
        updatePow(uart0_filestream, 0, id);
        waitAndProcessMessages(uart0_filestream, intervalo1);

        waitAndProcessMessages(uart0_filestream, intervalo2);

        if (digitalRead(floor)) // Confirma se está no andar correto
        {
            getEncoderInfo(uart0_filestream, id, id_message);
            waitAndProcessMessages(uart0_filestream, intervalo1);
        }
    }
    else
    {
        // Ajuste fino para descida do elevador
        updateDir(0, 1, id);
        updatePow(uart0_filestream, 100, id);
        waitAndProcessMessages(uart0_filestream, intervalo1);
    }
}

/**
 * @brief Envia comando para leitura dos botões pressionados no painel do elevador.
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void handleReadButtons(int *uart0_filestream)
{
    Mensagem msg;
    msg.endereco = 0x01;
    msg.cod = 0x03;
    msg.subCod = 0x00;
    msg.numBytes = QTD_BUTTONS_READ;

    std::pair<unsigned char *, int> tx_msg = handleTxBuffer(msg);
    sendMessage(uart0_filestream, tx_msg.first, tx_msg.second, REQUEST_BUTTONS_E1);

    free(tx_msg.first);

    msg.subCod = 0xA0;
    msg.numBytes = QTD_BUTTONS_READ;

    tx_msg = handleTxBuffer(msg);
    sendMessage(uart0_filestream, tx_msg.first, tx_msg.second, REQUEST_BUTTONS_E2);

    free(tx_msg.first);
}

/**
 * @brief Envia comando para alterar o estado dos botões do painel do elevador.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id_button Identificador do botão.
 * @param button_value Valor a ser enviado ao botão.
 */
void handleWriteButtons(int *uart0_filestream, unsigned char id_button, unsigned char button_value)
{
    Mensagem msg;
    msg.endereco = 0x01;
    msg.cod = 0x06;
    msg.subCod = id_button;
    msg.numBytes = 0x01;
    msg.val = button_value;

    int id_message = (id_button >= 0x00 && id_button <= 0x0A ? SEND_BUTTONS_E1 : SEND_BUTTONS_E2);

    std::pair<unsigned char *, int> tx_msg = handleTxBuffer(msg);
    sendMessage(uart0_filestream, tx_msg.first, tx_msg.second, id_message);
}

/**
 * @brief Lê periodicamente as informações dos encoders dos elevadores.
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void readEncoders(int *uart0_filestream)
{
    while (!parar)
    {
        if (deveLerEncoders)
        {
            getEncoderInfo(uart0_filestream, ADDR_E1, REQUEST_ENCODER_E1);
            getEncoderInfo(uart0_filestream, ADDR_E2, REQUEST_ENCODER_E2);
            deveLerEncoders = false;
        }
    }
}

/**
 * @brief Controla o movimento de um elevador baseado no valor de PWM fornecido.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param pwm Valor do PWM que define a velocidade e direção do movimento.
 * @param id_elevator Identificador do elevador (0 para E1, 1 para E2).
 */
void moveElevator(int *uart0_filestream, int pwm, int id_elevator)
{
    if (pwm > 0)
    {
        updateDir(1, 0, id_elevator); // Sobe
    }
    else if (pwm == 0)
    {
        updateDir(0, 0, id_elevator); // Para
    }
    else
    {
        updateDir(0, 1, id_elevator); // Desce
    }

    updatePow(uart0_filestream, abs(pwm), id_elevator);

    // Atualiza estado do elevador
    auto &elevator = (id_elevator == ADDR_E1) ? elevador1 : elevador2;
    if (pwm > 0)
        elevator.estadoElevador = SUBINDO;
    else if (pwm == 0)
        elevator.estadoElevador = PARADO;
    else
        elevator.estadoElevador = DESCENDO;

    printf("Elevador %d em movimento: PWM %d\n", id_elevator + 1, pwm);
}

/**
 * @brief Coordena o movimento do elevador baseado nas chamadas em espera.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id_elevator Identificador do elevador (0 para E1, 1 para E2).
 */
void coordinate_movement_elevator(int *uart0_filestream, int id_elevator)
{
    auto &elevator = (id_elevator == ADDR_E1) ? elevador1 : elevador2;
    auto &filaChamadaElevador = (id_elevator == ADDR_E1) ? heapChamadasElevador1 : heapChamadasElevador2;
    auto &current_direction = (id_elevator == ADDR_E1) ? direcao1 : direcao1;
    auto &mov_elevator = (id_elevator == ADDR_E1) ? moveElevador1 : moveElevador2;
    auto &gen_mov_elevator = (id_elevator == ADDR_E1) ? altMoveElevador1 : altMoveElevador2;
    auto &paraOndeIr = (id_elevator == ADDR_E1) ? paraOndeIr_1 : paraOndeIr_2;

    if (filaChamadaElevador.size() && current_direction == INT16_MIN && !mov_elevator && !gen_mov_elevator)
    {
        paraOndeIr = filaChamadaElevador.top().second;
        mov_elevator = true;
    }

    if (paraOndeIr != INT16_MIN && mov_elevator && !gen_mov_elevator)
    {
        switch (paraOndeIr)
        {
        case E1_BTN_EMERGENC:
        case E2_BTN_EMERGENC:
            current_direction = INT16_MIN;
            paraOndeIr = INT16_MIN;
            handleWriteButtons(uart0_filestream, paraOndeIr, 0x00);
            moveElevator(uart0_filestream, 0, id_elevator);
            filaChamadaElevador.pop();
            mov_elevator = false;
            break;
        default:
            // Define a direção e PWM baseado na posição atual e destino
            elevator.pwm = floor(pid_controle1(elevator.pos));
            moveElevator(uart0_filestream, elevator.pwm, id_elevator);
            break;
        }
    }
}

/**
 * @brief Verifica se o elevador chegou ao andar desejado e realiza ações apropriadas.
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param id_elevator Identificador do elevador (0 para E1, 1 para E2).
 */
void verify_elevator_arrival(int *uart0_filestream, int id_elevator)
{
    auto &elevator = (id_elevator == ADDR_E1) ? elevador1 : elevador2;
    auto &current_direction = (id_elevator == ADDR_E1) ? direcao1 : direcao1;
    auto &gen_mov_elevator = (id_elevator == ADDR_E1) ? altMoveElevador1 : altMoveElevador2;
    auto &mov_elevator = (id_elevator == ADDR_E1) ? moveElevador1 : moveElevador2;
    auto &paraOndeIr = (id_elevator == ADDR_E1) ? paraOndeIr_1 : paraOndeIr_2;

    if ((current_direction == elevator.terreo && digitalRead(elevator.terreo)) ||
        (current_direction == elevator.prmAndar && digitalRead(elevator.prmAndar)) ||
        (current_direction == elevator.sgdAndar && digitalRead(elevator.sgdAndar)) ||
        (current_direction == elevator.tcrAndar && digitalRead(elevator.tcrAndar)))
    {
        printf("Atenção: Elevador %d chegou no andar!\n", id_elevator + 1);
        moveElevator(uart0_filestream, 0, id_elevator);
        gen_mov_elevator = false;
        mov_elevator = false;

        if (hashChamadasElevador.count(paraOndeIr))
        {
            hashChamadasElevador.erase(paraOndeIr);
            filaChamadaElevador.pop();
        }

        if (current_direction == elevator.terreo)
        {
            handleWriteButtons(uart0_filestream, E1_BTN_GND_UP, 0x00);
        }
        // Adicionar para os outros botões e andares conforme necessário

        current_direction = INT16_MIN;
        paraOndeIr = INT16_MIN;
    }
}

/**
 * @brief Função principal para o controle dos elevadores, coordena o movimento e a chegada.
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void handleElevators(int *uart0_filestream)
{
    while (!devePararMovimentoElevador)
    {
        if (mover)
        {
            printf("INFO: Movendo elevadores!\n");
            coordinate_movement_elevator(uart0_filestream, ADDR_E1);
            coordinate_movement_elevator(uart0_filestream, ADDR_E2);
            verify_elevator_arrival(uart0_filestream, ADDR_E1);
            verify_elevator_arrival(uart0_filestream, ADDR_E2);
            mover = false;
        }
    }

    // Para ambos os elevadores antes de encerrar o programa
    moveElevator(uart0_filestream, 0, ADDR_E1);
    moveElevator(uart0_filestream, 0, ADDR_E2);
    parar = true;
}
