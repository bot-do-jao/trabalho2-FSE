#include <iostream>
#include <cmath>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>   
#include <termios.h> 

#include "define.hpp"
#include "elevatorControl.hpp"
#include "ssd1306.h"
#include "uartComm.hpp"
#include "handleDisplay.hpp"

/**
 * @brief Função de tratamento para o sinal de alarme (exemplo).
 * 
 * @param sig Código do sinal recebido.
 */
void handleAlarm(int sig)
{
    // std::cout << "Sinal de alarme recebido: " << sig << std::endl;
}

/**
 * @brief Função de tratamento para o sinal de término do programa.
 * 
 * @param sig Código do sinal recebido.
 */
void handleEndSignal(int sig)
{
    printf("SINAL DE TÉRMINO RECEBIDO: Finalizando operações e limpando a tela.\n");
    devePararMovimentoElevador = true; // Sinaliza para parar o movimento do elevador
    ssd1306_clearScreen();             // Limpa o display
}

/**
 * @brief Lê a temperatura do sensor especificado no caminho do arquivo.
 * 
 * @param filepath Caminho do arquivo de leitura do sensor de temperatura.
 * @return float Temperatura lida em graus Celsius, ou um valor negativo em caso de erro.
 */
float getTemperature(const char *filepath)
{
    int temp_file = open(filepath, O_RDONLY | O_NOCTTY | O_NDELAY); // Abre o arquivo do sensor de temperatura

    char buff[32];                          // Buffer para armazenar o valor lido
    int alright = read(temp_file, buff, sizeof(buff) - 1); // Lê o valor do arquivo
    close(temp_file);                       // Fecha o arquivo após a leitura

    if (alright > 0) // Verifica se a leitura foi bem-sucedida
    {
        int temp = std::stoi(buff); // Converte o valor lido para inteiro
        return (floor((((float)temp / 1000.0) * 100) + 0.5) / 100); // Converte e arredonda para float
    }

    printf("Temperatura do sensor invalida %s\n", filepath); // Log de erro
    return -1; // Retorna valor negativo indicando erro
}

/**
 * @brief Envia as temperaturas dos sensores via UART e atualiza o display.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void sendTemperaturesAndControlDisplay(int *uart0_filestream)
{
    std::pair<float, float> temps = handleTemperature(); // Obtém as temperaturas dos sensores
    sendTempInfo(uart0_filestream, ADDR_E1, temps.first);  // Envia temperatura do sensor 1 via UART
    sendTempInfo(uart0_filestream, ADDR_E2, temps.second); // Envia temperatura do sensor 2 via UART
    printDisplay(elevador1.estadoElevador, elevador2.estadoElevador, temps.first, temps.second); // Atualiza o display com as informações
}

/**
 * @brief Lida com a leitura das temperaturas dos sensores conectados ao sistema.
 * 
 * @return std::pair<float, float> Par de temperaturas lidas dos sensores.
 */
std::pair<float, float> handleTemperature()
{
    // Caminhos dos arquivos dos sensores de temperatura
    const char *filepath[] = {
        "/sys/bus/i2c/devices/i2c-1/1-0077/iio:device1/in_temp_input",
        "/sys/bus/i2c/devices/i2c-1/1-0076/iio:device0/in_temp_input"};

    std::pair<float, float> temps;

    // Lê as temperaturas dos dois sensores
    temps.first = getTemperature(filepath[0]);
    temps.second = getTemperature(filepath[1]);

    return temps; // Retorna as temperaturas lidas
}
