#ifndef PID_H_
#define PID_H_

double limita_valor(double valor, double max, double min);
void pid_configura_constantes(double Kp_, double Ki_, double Kd_);
void pid_atualiza_referencia(float referencia_);
double calcula_pid(double erro, double* erro_total, double* erro_anterior);
double pid_controle1(double saida_medida);
double pid_controle2(double saida_medida);

#endif /* PID_H_ */