// #include <bcm2835.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>  
#include <fcntl.h>   
#include <termios.h> 
#include <cmath>
#include <queue>
#include <mutex>
#include <set>

#include "crc16.hpp"
#include "define.hpp"
#include "elevatorCentral.hpp"

// INSTANTIATING GLOBAL VARIABLES
bool parar = false;
bool mover = false;
bool deveLerEncoders = false;
bool deveLerMensagem = false;
bool devePararMovimentoElevador = false;
std::priority_queue<std::pair<int, int>> filaChamadaElevador;

std::mutex mtx_envia_mensagem;
std::queue<int> filaMensagensEnviadas;
std::set<int> hashChamadasElevador;
std::priority_queue<std::pair<int, int>> heapChamadasElevador1;
std::set<int> hashChamadasElevador1;
std::priority_queue<std::pair<int, int>> heapChamadasElevador2;
std::set<int> hashChamadasElevador2;

Botoes bts_elevador1;
Botoes bts_elevador2;

Elevador elevador1;
Elevador elevador2;
