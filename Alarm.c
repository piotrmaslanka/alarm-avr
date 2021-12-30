#define F_CPU 1843200
#include "avr/interrupt.h"
#include "avr/wdt.h"
#include "pins.h"
#include "uart.h"


#define SIGNAL_LENGTH 4
#define ALARM_LENGTH 0x04EC
#define VB_ARMED 0x7F
#define VB_UNARMED 0x01

unsigned char bAlarmMask, bAlarmEnabled, bSiren, bDebugStatus = 0, bPresenceSuspend = 0;

void rearm(unsigned char newMask)
{
	bAlarmEnabled = 0;
	bAlarmMask = (newMask | VB_UNARMED) & VB_ARMED;
}

void boot()
{
	rearm(VB_UNARMED);
	pins_init();
	uart_init();
	wdt_enable(WDTO_60MS);
	sei();
}


void loop()
{
	unsigned char presence = 0;		// --------------- do presence
	for(int i=0; i<7; i++) 
		if (baTrigger[i] >= SIGNAL_LENGTH) presence |= 1 << i;


	if ((presence & bAlarmMask) > 0)		// -------------- check alarm
	{
		cli(); iTimer = 0; sei();
		bAlarmEnabled = 1;
	}
								// -------------------- alarm disabler
	if ((bAlarmEnabled == 1) && (iTimer > ALARM_LENGTH)) bAlarmEnabled = 0;

								// -------------- arm-via-casette
	if (baTrigger[7] == SIGNAL_LENGTH)
	{
		cli(); baTrigger[7]++; sei();

		if (bAlarmMask == VB_UNARMED) rearm(VB_ARMED);
		else rearm(VB_UNARMED);
	}
								// -------------- arm led blinker
	if (bAlarmMask == VB_ARMED)
	{
		pins_out(PINS_ARMLED, 1);
	}
	else if (bAlarmMask == VB_UNARMED)
	{
		pins_out(PINS_ARMLED, 0);
	}
	else
	{
		if ((iTimer & 16) == 16) pins_out(PINS_ARMLED, 1);
		else pins_out(PINS_ARMLED, 0);
	}
								// ---------------------- siren
	uint8_t siren_temp = 0;

	siren_temp |= bSiren;
	siren_temp |= bAlarmEnabled;
	pins_out(PINS_SIREN, siren_temp);
								// ---------------------- suspend stopper
	bPresenceSuspend |= presence;

	wdt_reset();
}


int main()
{
	boot();
	for (;;) loop();
}
