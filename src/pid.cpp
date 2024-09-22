#include <stdio.h>

// Definição das constantes e variáveis do controlador PID
double saida_medida, sinal_de_controle;
double referencia = 0.0;
double Kp = 0.5;  // Ganho Proporcional
double Ki = 0.05;  // Ganho Integral
double Kd = 40.0;  // Ganho Derivativo
const int T = 500; // Período de Amostragem (ms)
const int sinal_de_controle_MAX = 100; // Limite superior do sinal de controle
const int sinal_de_controle_MIN = -100; // Limite inferior do sinal de controle
double erro_total1 = 0.0, erro_anterior1 = 0.0; // Variáveis de controle PID1
double erro_total2 = 0.0, erro_anterior2 = 0.0; // Variáveis de controle PID2

// Função para limitar um valor dentro dos limites especificados
double limita_valor(double valor, double max, double min) {
    if (valor > max) return max;
    if (valor < min) return min;
    return valor;
}

// Função para configurar as constantes do controlador PID
void pid_configura_constantes(double Kp_, double Ki_, double Kd_) {
    Kp = Kp_;
    Ki = Ki_;
    Kd = Kd_;
}

// Função para atualizar a referência de controle
void pid_atualiza_referencia(float referencia_) {
    referencia = (double) referencia_;
}

// Função genérica para cálculo do sinal de controle PID
double calcula_pid(double erro, double* erro_total, double* erro_anterior) {
    *erro_total = limita_valor(*erro_total + erro, sinal_de_controle_MAX, sinal_de_controle_MIN);

    double delta_error = erro - *erro_anterior;
    double sinal = Kp * erro + (Ki * T) * (*erro_total) + (Kd / T) * delta_error;

    *erro_anterior = erro;
    return limita_valor(sinal, sinal_de_controle_MAX, sinal_de_controle_MIN);
}

// Função de controle PID para o primeiro sistema
double pid_controle1(double saida_medida) {
    double erro = referencia - saida_medida;
    printf("\nDEBUG:\nREFERÊNCIA: %f\nSAÍDA MEDIDA: %f\n", referencia, saida_medida);
    return calcula_pid(erro, &erro_total1, &erro_anterior1);
}

// Função de controle PID para o segundo sistema
double pid_controle2(double saida_medida) {
    double erro = referencia - saida_medida;
    printf("\nDEBUG:\nREFERÊNCIA: %f\nSAÍDA MEDIDA: %f\n", referencia, saida_medida);
    return calcula_pid(erro, &erro_total2, &erro_anterior2);
}