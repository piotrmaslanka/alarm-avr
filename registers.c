#include "Alarm.h"
#include "uart.h"

unsigned int modbus_read_holding(unsigned int port)
{
	mbhStatus = MODBUS_HANDLE_OK;
	switch (port)
	{
		case 0x0000:
			return (bAlarmEnabled << 8) + bPresenceSuspend;
		case 0x0001:
			return bAlarmMask;
		case 0x0002:
			return bSiren;
		case 0x0003:
			return bDebugStatus;
		default:
			mbhStatus = MODBUS_HANDLE_ERR;
	}	
	return 0;
}
unsigned int modbus_write_holding(unsigned int port, unsigned int value)
{
	mbhStatus = MODBUS_HANDLE_OK;
	switch (port)
	{
		case 0x0000:
			bPresenceSuspend = (unsigned char)(value & 0xFF);
			return bPresenceSuspend;
		case 0x0001:
			rearm((unsigned char)(value & 0xFF));
			return bAlarmMask;
		case 0x0002:
			bSiren = (unsigned char)(value & 0xFF);
			return bSiren;
		case 0x0003:
			bDebugStatus = (unsigned char)(value & 0xFF);
			return bDebugStatus;
		default:
			mbhStatus = MODBUS_HANDLE_ERR;
	}
	return 0;
}


