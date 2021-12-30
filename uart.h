#define MODBUS_HANDLE_OK 0
#define MODBUS_HANDLE_ERR 1
#define UART_TRANSMITTING 0
#define UART_RECEIVING 1

unsigned char mbhStatus;
unsigned char mbCRCH, mbCRCL;
unsigned char buffer_length;
unsigned char buffer[50];

unsigned char bStatus;
unsigned int iTransmissionTimer;

void uart_switch_receive();

void uart_init();
