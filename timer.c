#include "avr/io.h"
#include "avr/interrupt.h"



uint16_t timer_timer;

SIGNAL (SIG_OVERFLOW0)
{
	timer_timer++;
}

void timer_init()
{
	TCCR0 = (1<<CS02)|(0<<CS01)|(1<<CS00);		// equals to 0x05
	TIMSK = (1 << TOIE0);						// equals to 0x01
}
