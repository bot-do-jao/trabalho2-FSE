#include <string>
#include <queue>
#include <mutex>
#include <set>

#ifndef DEFINE_H_
#define DEFINE_H_

// structs
enum Estado
{
    SUBINDO = 1,
    PARADO = 0,
    DESCENDO = -1
};

typedef struct Mensagem
{
    int pwm;
    float tmp;
    unsigned char id;
    unsigned char cod;
    unsigned char val;
    unsigned char subCod;
    unsigned char endereco;
    unsigned char numBytes;
    unsigned char tamTexto;
    unsigned char texto;
    unsigned char matr[4] = {0x08, 0x01, 0x06, 0x07};
} Mensagem;

typedef struct Elevador
{
    int pos;
    int pwm;
    int terreo = -1;
    int prmAndar = -1;
    int sgdAndar = -1;
    int tcrAndar = -1;
    enum Estado estadoElevador;
} Elevador;

typedef struct Botoes
{
    unsigned char bt_terreo_up = 0;
    unsigned char bt_prm_down = 0;
    unsigned char bt_prm_up = 0;
    unsigned char bt_sgd_down = 0;
    unsigned char bt_sgd_up = 0;
    unsigned char bt_tcr_down = 0;
    unsigned char bt_emerg = 0;
    unsigned char bt_terreo = 0;
    unsigned char bt_prm = 0;
    unsigned char bt_sgd = 0;
    unsigned char bt_tcr = 0;
} Botoes;

extern Botoes bts_elevador1;
extern Botoes bts_elevador2;

// GLOBAL VARIABLES
extern bool parar;
extern bool mover;
extern bool deveLerMensagem; 
extern bool deveLerEncoders;
extern bool devePararMovimentoElevador;
extern enum Estado estado;

// GLOBAL ELEVATORS QUEUES
extern std::queue<int> filaMensagensEnviadas;
extern std::priority_queue<std::pair<int, int>> filaChamadaElevador;
extern std::set<int> hashChamadasElevador;
extern std::priority_queue<std::pair<int, int>> heapChamadasElevador1;
extern std::set<int> hashChamadasElevador1;
extern std::priority_queue<std::pair<int, int>> heapChamadasElevador2;
extern std::set<int> hashChamadasElevador2;
extern std::mutex mtx_envia_mensagem;

// Global Elevators
extern Elevador elevador1;
extern Elevador elevador2;

extern Botoes bts_elevador1;
extern Botoes bts_elevador2;

// Global elevators movement status variables
extern bool moveElevador1;
extern bool moveElevador2;
extern bool altMoveElevador1;
extern bool altMoveElevador2;
extern int direcao1;
extern int direcao2;

// BUFFERS
#define TX_BUFFER_SIZE 256
#define RX_BUFFER_SIZE 255

// TIME CONSTANTS
#define TIME_READ_RX_BUFFER 5
#define TIME_SEND_TEMP 150
#define TIME_SEND_PWM 100
#define TIME_READ_BUTTONS 150
#define TIME_READ_ENCODERS 50
#define TIME_MOVE_ELEVATOR 50

// QUANTITIES
#define QTD_BUTTONS_READ 0x0B

// FUNCTIONALITIES IDENTIFIERS
#define SEND_TEMPERATURE_E1 0
#define SEND_TEMPERATURE_E2 1
#define REQUEST_ENCODER_E1 2
#define REQUEST_ENCODER_E2 3
#define REQUEST_BUTTONS_E1 4
#define REQUEST_BUTTONS_E2 5
#define SEND_BUTTONS_E1 6
#define SEND_BUTTONS_E2 7
#define SEND_PWM_E1 8
#define SEND_PWM_E2 9

// POSITIONAL IDENTIFIERS
#define REQUEST_GROUND_FLOOR_E1 10
#define REQUEST_FIRST_FLOOR_E1 11
#define REQUEST_SECOND_FLOOR_E1 12
#define REQUEST_THIRD_FLOOR_E1 13
#define REQUEST_GROUND_FLOOR_E2 14
#define REQUEST_FIRST_FLOOR_E2 15
#define REQUEST_SECOND_FLOOR_E2 16
#define REQUEST_THIRD_FLOOR_E2 17

// GPIO - Elevador 1
#define E1_DIR1 20
#define E1_DIR2 21
#define E1_POTM 12
#define E1_GRND 18
#define E1_FIRS 23
#define E1_SECN 24
#define E1_THIR 25

// GPIO - Elevador 2
#define E2_DIR1 19
#define E2_DIR2 26
#define E2_POTM 13
#define E2_GRND 17
#define E2_FIRS 27
#define E2_SECN 22
#define E2_THIR 6

// Addresses
#define ADDR_TEMP 0x76
#define ADDR_ESP32 0x01
#define ADDR_ENG1 0x00
#define ADDR_ENG2 0x01
#define ADDR_E1 0x00
#define ADDR_E2 0x01

// Register Adresses - Elevador 1
#define E1_BTN_GND_UP 0x00
#define E1_BTN_FST_DOWN 0x01
#define E1_BTN_FST_UP 0x02
#define E1_BTN_SCD_DOWN 0x03
#define E1_BTN_SCD_UP 0x04
#define E1_BTN_THR_DOWN 0x05
#define E1_BTN_EMERGENC 0x06
#define E1_BTN_GROUND 0x07
#define E1_BTN_FIRST 0x08
#define E1_BTN_SECOND 0x09
#define E1_BTN_THIRD 0x0A

// Register Adresses - Elevador 2
#define E2_BTN_GND_UP 0xA0
#define E2_BTN_FST_DOWN 0xA1
#define E2_BTN_FST_UP 0xA2
#define E2_BTN_SCD_DOWN 0xA3
#define E2_BTN_SCD_UP 0xA4
#define E2_BTN_THR_DOWN 0xA5
#define E2_BTN_EMERGENC 0xA6
#define E2_BTN_GROUND 0xA7
#define E2_BTN_FIRST 0xA8
#define E2_BTN_SECOND 0xA9
#define E2_BTN_THIRD 0xAA

// Defining buttons priorities

// common buttons
#define PRIORITY_BTN_GND_UP      0x00
#define PRIORITY_BTN_FST_DOWN    0x00
#define PRIORITY_BTN_FST_UP      0x00
#define PRIORITY_BTN_SCD_DOWN    0x00
#define PRIORITY_BTN_SCD_UP      0x00
#define PRIORITY_BTN_THR_DOWN    0x00

// elevator internal buttons
#define PRIORITY_BTN_EMERGENC 0x02
#define PRIORITY_BTN_GROUND   0x01
#define PRIORITY_BTN_FIRST    0x01
#define PRIORITY_BTN_SECOND   0x01
#define PRIORITY_BTN_THIRD    0x01

// Display
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64

#endif /* DEFINE_H_ */