#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <thread>
#include <string>
#include <unistd.h>  
#include <fcntl.h>   
#include <termios.h> 

#include "define.hpp"
#include "crc16.hpp"
#include "elevatorCentral.hpp"
#include "handleDisplay.hpp"
#include "uartComm.hpp"
#include "utils.hpp"
#include "elevatorControl.hpp"

// Funcao para inicializar configuracoes do sistema como display, UART e GPIO
void initializeSystem(int *uart0_filestream)
{
    setupDisplay(); // Inicializa configuracao do display
    initUARTConfig(uart0_filestream); // Inicializa configuracao da UART
    initGPIOConfig(); // Inicializa configuracao dos pinos GPIO
}

// Funcao para configurar handlers de sinais do sistema
void setupSignalHandlers()
{
    signal(SIGALRM, handleAlarm); // Configura o handler para o sinal de alarme
    signal(SIGINT, handleEndSignal); // Configura o handler para o sinal de interrupcao (ex: Ctrl+C)
}

// Funcao para criar threads para diferentes tarefas do sistema
void createThreads(std::thread &threadMsg, std::thread &threadEncoder, std::thread &threadElevador, int *uart0_filestream)
{
    threadMsg = std::thread(handleRxMessages, uart0_filestream); // Thread para tratar mensagens recebidas
    threadEncoder = std::thread(readEncoders, uart0_filestream); // Thread para ler valores dos encoders
    threadElevador = std::thread(handleElevators, uart0_filestream); // Thread para controlar operacoes dos elevadores
}

// Funcao para processar diferentes tarefas em intervalos de tempo especificos
void processTasks(int *uart0_filestream, unsigned int &counter)
{
    if ((counter % TIME_SEND_TEMP) == 0)
    {
        sendTemperaturesAndControlDisplay(uart0_filestream); // Envia dados de temperatura e atualiza o display
    }
    if ((counter % TIME_SEND_PWM) == 0)
    {
        sendPWMs(uart0_filestream); // Envia sinais PWM
    }
    if ((counter % TIME_READ_BUTTONS) == 0)
    {
        handleReadButtons(uart0_filestream); // Le entradas dos botoes do elevador
    }
    if ((counter % TIME_MOVE_ELEVATOR) == 0)
    {
        mover = true; // Sinaliza que e hora de mover o elevador
    }
    if ((counter % TIME_READ_ENCODERS) == 0)
    {
        deveLerEncoders = true; // Sinaliza que e hora de ler os encoders
    }
    if (!filaMensagensEnviadas.empty() && (counter % TIME_READ_RX_BUFFER) == 0)
    {
        deveLerMensagem = true; // Sinaliza que e hora de ler mensagens do buffer de recepcao, se houver mensagens na fila
    }
}

int main()
{
    int uart0_filestream; // Descritor de arquivo da UART
    initializeSystem(&uart0_filestream); // Inicializa configuracoes do sistema
    setupSignalHandlers(); // Configura handlers para sinais de interrupcao e alarme

    calibrateElevators(&uart0_filestream); // Calibra posicoes dos elevadores

    // Criacao de threads para tratar diferentes tarefas
    std::thread varThreadMsg, varThreadEncoder, varThreadElevador;
    createThreads(varThreadMsg, varThreadEncoder, varThreadElevador, &uart0_filestream);

    unsigned int counter = 0; // Contador para acompanhar os intervalos de tempo
    useconds_t interval = 10 * 1000; // Intervalo para ualarm em microsegundos (10 milissegundos)

    while (!parar) // Loop principal que executa ate que 'parar' seja true
    {
        ualarm(interval, 0); // Define um alarme para o intervalo especificado
        pause(); // Aguarda por um sinal
        processTasks(&uart0_filestream, counter); // Executa tarefas agendadas com base no valor do contador
        counter = (counter + 1) % 301; // Incrementa o contador e reinicia a cada 300 iteracoes
    }

    // Aguarda todas as threads completarem antes de sair
    varThreadMsg.join();
    varThreadEncoder.join();
    varThreadElevador.join();

    close(uart0_filestream); 
    return 0; 
}
