#include "avr/io.h"
#include "avr/interrupt.h"
#include "uart.h"

#define PINS_SIREN 1
#define PINS_ARMLED 2

uint16_t iTimer;
uint8_t baTrigger[8];

uint8_t pins_collect()
{
	uint8_t c, d;
	c = PINC;
	d = PIND;
	return (((c & 0xFC) + (d >> 6)) ^ 128);
}

SIGNAL (SIG_OVERFLOW0)
{
	iTimer++;
	uint8_t pins;
	pins = pins_collect();

	for (uint8_t i = 0; i < 8; i++)
		if (((1 << i) & pins) > 0) baTrigger[i] = baTrigger[i] + 1;
		else baTrigger[i] = 0;

	if (bStatus == UART_TRANSMITTING) // If the device spends too much time on transmitting...
		if ((iTimer - iTransmissionTimer) > 2) // it is invalid - reset transmission, set to listen
			uart_switch_receive();
}

void pins_out(uint8_t what, uint8_t status)
{
	status = 1 - status;		// remember - outputs are negated
								// by output board design
	if (what == PINS_SIREN)
	{
		if (status == 0) 	  PORTD &= ~8;
		else if (status == 1) PORTD |= 8;
	}
	else if (what == PINS_ARMLED)
	{
		if (status == 0) 	  PORTD &= ~4;
		else if (status == 1) PORTD |= 4;
	}
}

void pins_init()
{
	DDRA = 0xFF;
	DDRB = 0xFF;		// PORTA, PORTB unused - outputs
	DDRC = 0x03;
	DDRD = 0x3C;
	pins_out(PINS_SIREN, 0);
	pins_out(PINS_ARMLED, 0);

	TCCR0 = (1<<CS02)|(0<<CS01)|(1<<CS00);	// Prescaler = 1024
	TIMSK = (1 << TOIE0);					// Interrupt on overflow
											//	will happen at 7,03 Hz

	for(uint8_t i=0; i<8; i++) baTrigger[i] = 0;
}


