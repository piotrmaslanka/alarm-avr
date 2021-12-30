#include "avr/io.h"
#include "avr/interrupt.h"
#include "crc.h"
#include "pins.h"	 // for iTimer
#include "registers.h"		// MODBUS Responder

#define ADDRESS 0x0A
#define MAXREGISTER 3

#define SS_ADDRESS 1
#define SS_COMMAND 2
#define SS_REGISTER_H_RW 3
#define SS_REGISTER_L_RW 4
#define SS_VALUE_H_W 5
#define SS_VALUE_L_W 6
#define SS_AMOUNT_H_R 7
#define SS_AMOUNT_L_R 8
#define SS_CRC_H 9
#define SS_CRC_L 10

#define MC_READ 0x03
#define MC_WRITE 0x06

#define MODBUS_HANDLE_OK 0
#define MODBUS_HANDLE_ERR 1

#define UART_TRANSMITTING 0
#define UART_RECEIVING 1

unsigned char bStatus;
unsigned char bScanStatus;
unsigned int iTransmissionTimer;
	// ----- modbus values
unsigned char mbCommand;
unsigned char mbRegisterL;
unsigned char mbValueL;
unsigned char mbAmountL;
unsigned char mbCRCH, mbCRCL;
	// -------- interfacing
unsigned char mbhStatus;


unsigned char buffer[50];
unsigned char buffer_length;
unsigned char buffer_pointer;

inline void buf_reset() { buffer_length = 0; }
inline void buf_append(unsigned char value) { buffer[buffer_length] = value; buffer_length++; }

void uart_switch_receive() { PORTD &= ~16; bStatus = UART_RECEIVING; }
inline void uart_switch_transmit() { PORTD |= 16; bStatus = UART_TRANSMITTING; iTransmissionTimer = iTimer; }

unsigned char uart_send()
{
	if (buffer_pointer == buffer_length) return 0;
	UDR = buffer[buffer_pointer];
	buffer_pointer++;
	return 1;
}

SIGNAL (SIG_USART_TRANS)
{
	if (bStatus == UART_RECEIVING) return;
	if (uart_send() == 0) uart_switch_receive();
}

void analyze(unsigned char data);
inline void reanalyze(unsigned char data)
{
	bScanStatus = SS_ADDRESS;
	analyze(data);
}

void uart_process_input()
{
	if (mbCommand == MC_READ)
	{
		buf_reset();
		buf_append(ADDRESS);
		buf_append(MC_READ);
		unsigned char bytesize = mbAmountL * 2;
		buf_append(bytesize & 0xFF);
		for(unsigned int port=mbRegisterL; port < mbRegisterL+mbAmountL; port++)
		{
			unsigned int value = modbus_read_holding(port);
			if (mbhStatus == MODBUS_HANDLE_ERR) return;
			buf_append((unsigned char)(value >> 8));
			buf_append((unsigned char)(value & 0xFF));
		}
	} else
	if (mbCommand == MC_WRITE)
	{
		buf_reset();
		buf_append(ADDRESS);
		buf_append(MC_WRITE);
		buf_append(0);
		buf_append(mbRegisterL);
		unsigned int value = modbus_write_holding(mbRegisterL, mbValueL);
		if (mbhStatus == MODBUS_HANDLE_ERR) return;
		buf_append((unsigned char)(value >> 8));
		buf_append((unsigned char)(value & 0xFF));
	}
	crc_calculate();
	buf_append(mbCRCH);
	buf_append(mbCRCL);
	buffer_pointer = 0;
	uart_switch_transmit();
	uart_send();		
}

void analyze(unsigned char data)
{
	switch (bScanStatus)
	{
		case (SS_ADDRESS):
			if (data == ADDRESS) bScanStatus = SS_COMMAND;
									break;
		case (SS_COMMAND):
			if ((data == MC_WRITE) || (data == MC_READ))
			{
				bScanStatus = SS_REGISTER_H_RW;
				mbCommand = data;
			} else reanalyze(data);	break;
		case (SS_REGISTER_H_RW):
			if (data == 0)
				bScanStatus = SS_REGISTER_L_RW;	
			else reanalyze(data); break;
		case (SS_REGISTER_L_RW):
			if (data <= MAXREGISTER)
			{
				mbRegisterL = data;
				bScanStatus = ((mbCommand == MC_WRITE) ? SS_VALUE_H_W : SS_AMOUNT_H_R);
			}
			else reanalyze(data); break;
		case (SS_VALUE_H_W):
			if (data == 0) 
				bScanStatus = SS_VALUE_L_W;
			else reanalyze(data); break;
		case (SS_VALUE_L_W):			// this step must calculate CRC
			mbValueL = data;
			buf_reset();
			buf_append(ADDRESS);
			buf_append(MC_WRITE);
			buf_append(0);
			buf_append(mbRegisterL);
			buf_append(0);
			buf_append(mbValueL);
			crc_calculate();
			bScanStatus = SS_CRC_H;
								  break;
		case (SS_AMOUNT_H_R):
			if (data == 0)
				bScanStatus = SS_AMOUNT_L_R;
			else reanalyze(data); break;
		case (SS_AMOUNT_L_R):			// this step must calculate CRC
			if ((data <= MAXREGISTER + 1 - mbRegisterL) && (data > 0))
			{
				mbAmountL = data;
				buf_reset();
				buf_append(ADDRESS);
				buf_append(MC_READ);
				buf_append(0);
				buf_append(mbRegisterL);
				buf_append(0);
				buf_append(mbAmountL);
				crc_calculate();
				bScanStatus = SS_CRC_H;
			}
			else reanalyze(data); break;
		case (SS_CRC_H):
			if (data == mbCRCH)
				bScanStatus = SS_CRC_L;
			else reanalyze(data); break;
		case (SS_CRC_L):
			if (data == mbCRCL)
			{
				uart_process_input();
				bScanStatus = SS_ADDRESS;
			}
			else reanalyze(data); break;
	}
	
}

SIGNAL (SIG_USART_RECV)
{
	unsigned char data = UDR;
	analyze(data);
}

void uart_init()
{
	bScanStatus = SS_ADDRESS;
	UBRRH = 0;
	UBRRL = 11;
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
	UCSRB |= (1<<TXCIE)|(1<<RXCIE)|(1<<RXEN)|(1<<TXEN);
	
	uart_switch_receive();
}
