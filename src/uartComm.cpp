#include <iostream>
#include <stdio.h>
#include <unistd.h>  
#include <fcntl.h>   
#include <termios.h> 
#include <queue>
#include <mutex>

#include "define.hpp"
#include "signalHandler.hpp"

/**
 * @brief Configura as opções de comunicação UART, como taxa de transmissão e formato dos dados.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param options Estrutura termios que armazena as configurações de comunicação.
 */
void configureUARTOptions(int *uart0_filestream, struct termios *options) {
    options->c_cflag = B115200 | CS8 | CLOCAL | CREAD; // Configura baud rate para 115200, 8 bits de dados, sem controle de fluxo.
    options->c_iflag = IGNPAR;                         // Ignora erros de paridade nos bytes recebidos.
    options->c_oflag = 0;                              // Saída pura, sem processamento.
    options->c_lflag = 0;                              // Desativa entrada canônica e outros processamentos de linha.
    tcflush(*uart0_filestream, TCIFLUSH);              // Limpa os dados recebidos ainda não lidos.
    tcsetattr(*uart0_filestream, TCSANOW, options);    // Aplica as configurações imediatamente.
}

/**
 * @brief Abre o filestream da UART para comunicação.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @return true se a UART foi aberta com sucesso, false caso contrário.
 */
bool openUARTFilestream(int *uart0_filestream) {
    // Abre o dispositivo UART /dev/serial0 em modo leitura/escrita sem bloqueio.
    *uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    return (*uart0_filestream != -1); // Retorna verdadeiro se a abertura foi bem-sucedida.
}

/**
 * @brief Exibe o status de inicialização da UART.
 * 
 * @param status Status de inicialização (true se bem-sucedido, false se falhou).
 */
void displayUARTInitStatus(bool status) {
    if (!status)
        printf("Uart com erro\n");
    else
        printf("Uart conectada\n");
}

/**
 * @brief Inicializa a configuração da UART.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void initUARTConfig(int *uart0_filestream) {
    struct termios options;

    bool status = openUARTFilestream(uart0_filestream); // Abre o filestream da UART.
    displayUARTInitStatus(status);                      // Exibe o status de inicialização.

    if (status) { // Se a UART foi aberta com sucesso, configura as opções.
        tcgetattr(*uart0_filestream, &options); // Obtém as configurações atuais da UART.
        configureUARTOptions(uart0_filestream, &options); // Configura as opções da UART.
    }
}

/**
 * @brief Escreve dados no UART.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param tx_buffer Buffer de transmissão contendo os dados.
 * @param tx_buffer_size Tamanho do buffer de transmissão.
 * @param message_id Identificador da mensagem para rastreamento.
 */
void writeDataToUART(int *uart0_filestream, unsigned char *tx_buffer, int tx_buffer_size, int message_id) {
    int count = write(*uart0_filestream, tx_buffer, tx_buffer_size); // Escreve dados no UART.
    if (count <= 0) {
        printf("Falha na transmissao da UART\n"); // Log de erro na transmissão.
    } else {
        filaMensagensEnviadas.push(message_id); // Adiciona o ID da mensagem na fila para controle.
    }
}

/**
 * @brief Envia uma mensagem via UART com proteção de mutex.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 * @param tx_buffer Buffer de transmissão contendo os dados.
 * @param tx_buffer_size Tamanho do buffer de transmissão.
 * @param message_id Identificador da mensagem para rastreamento.
 */
void sendMessage(int *uart0_filestream, unsigned char *tx_buffer, int tx_buffer_size, int message_id) {
    if (*uart0_filestream != -1) { // Verifica se o filestream da UART está aberto.
        std::lock_guard<std::mutex> lock(mtx_envia_mensagem); // Bloqueia o mutex para evitar concorrência na transmissão.
        writeDataToUART(uart0_filestream, tx_buffer, tx_buffer_size, message_id); // Envia os dados.
    }
}

/**
 * @brief Lida com a recepção de mensagens via UART.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void rxMessage(int *uart0_filestream) {
    if (*uart0_filestream != -1) { // Verifica se o filestream da UART está aberto.
        unsigned char rx_buffer[RX_BUFFER_SIZE + 1]; // Buffer de recepção.
        int rx_length = read(*uart0_filestream, (void *)rx_buffer, 3); // Lê os três primeiros bytes da mensagem.
        if (rx_length > 0) {
            handleMessage(uart0_filestream, rx_buffer); // Lida com a mensagem recebida.
        } else {
            printf("Falha nos dados da UART\n"); // Log de erro na leitura.
        }
    }
}

/**
 * @brief Função que lida continuamente com as mensagens recebidas via UART em uma thread separada.
 * 
 * @param uart0_filestream Ponteiro para o filestream UART.
 */
void handleRxMessages(int *uart0_filestream) {
    while (!parar) { // Executa enquanto não for solicitado parar.
        if (deveLerMensagem) { // Verifica se há uma nova mensagem para ser lida.
            rxMessage(uart0_filestream); // Lê e processa a mensagem.
            deveLerMensagem = false; // Reseta a flag de leitura.
        }
    }
}
