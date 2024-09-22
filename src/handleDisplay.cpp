#include "ssd1306.h"

/**
 * @brief Exibe informações sobre o estado dos elevadores e suas temperaturas no display.
 * 
 * @param e1_dir Direção do elevador 1 (-1 para descendo, 0 para parado, 1 para subindo).
 * @param e2_dir Direção do elevador 2 (-1 para descendo, 0 para parado, 1 para subindo).
 * @param e1_temp Temperatura do elevador 1 em graus Celsius.
 * @param e2_temp Temperatura do elevador 2 em graus Celsius.
 */
void printDisplay(int e1_dir, int e2_dir, float e1_temp, float e2_temp)
{
    // Configura a fonte fixa para o display
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_clearScreen(); // Limpa o display para novas informações

    // Mensagens a serem exibidas no display
    char msg1[128] = "Elevador 1 -> ";
    char msg2[128] = "Temp.: ";
    char msg3[128] = "Elevador 2 -> ";
    char msg4[128] = "Temp.: ";
    char temp[20];

    // Formata a temperatura do elevador 1 e adiciona à mensagem
    sprintf(temp, "%.2f", e1_temp);
    strcat(msg2, temp);
    strcat(msg2, "ºC");

    // Formata a temperatura do elevador 2 e adiciona à mensagem
    sprintf(temp, "%.2f", e2_temp);
    strcat(msg4, temp);
    strcat(msg4, "ºC");

    // Atualiza a mensagem com o estado do elevador 1
    switch (e1_dir)
    {
    case -1:
        strcat(msg1, "DESCENDO"); // Elevador 1 está descendo
        break;
    case 0:
        strcat(msg1, "PARADO"); // Elevador 1 está parado
        break;
    case 1:
        strcat(msg1, "SUBINDO"); // Elevador 1 está subindo
        break;
    default:
        strcat(msg1, "ESTADO DESCONHECIDO"); // Estado desconhecido
        break;
    }

    // Atualiza a mensagem com o estado do elevador 2
    switch (e2_dir)
    {
    case -1:
        strcat(msg3, "DESCENDO"); // Elevador 2 está descendo
        break;
    case 0:
        strcat(msg3, "PARADO"); // Elevador 2 está parado
        break;
    case 1:
        strcat(msg3, "SUBINDO"); // Elevador 2 está subindo
        break;
    default:
        strcat(msg3, "ESTADO DESCONHECIDO"); // Estado desconhecido
        break;
    }

    // Imprime as mensagens no display OLED nas posições especificadas
    ssd1306_printFixed(0, 8, msg1, STYLE_NORMAL);  // Estado do elevador 1
    ssd1306_printFixed(0, 24, msg2, STYLE_NORMAL); // Temperatura do elevador 1
    ssd1306_printFixed(0, 48, msg3, STYLE_NORMAL); // Estado do elevador 2
    ssd1306_printFixed(0, 64, msg4, STYLE_NORMAL); // Temperatura do elevador 2
}

/**
 * @brief Exemplo de demonstração de texto no display OLED.
 * Exibe uma mensagem de teste para verificar se o display está funcionando corretamente.
 */
void textDemo()
{
    ssd1306_setFixedFont(ssd1306xled_font6x8); // Configura a fonte a ser utilizada no display
    ssd1306_clearScreen(); // Limpa o display antes de imprimir a mensagem
    ssd1306_printFixed(0, 8, "Averiguando.", STYLE_NORMAL); // Imprime uma mensagem de verificação no display
}

/**
 * @brief Configura e inicializa o display OLED.
 * 
 * @return int Retorna 0 em caso de sucesso.
 */
int setupDisplay()
{
    // Define a fonte fixa a ser utilizada em todo o display
    ssd1306_setFixedFont(ssd1306xled_font6x8);

    // Inicializa o display OLED no modo I2C com resolução de 128x64 pixels
    ssd1306_128x64_i2c_init();

    ssd1306_clearScreen(); // Limpa o display
    textDemo(); // Exibe a mensagem de demonstração no display
    return 0; // Retorna 0 indicando sucesso na inicialização do display
}
