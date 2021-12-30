#define SIGNAL_LENGTH 5
#define ALARM_LENGTH 0x04EC
#define VB_ARMED 0x7F
#define VB_UNARMED 0x01

unsigned char bAlarmMask, bAlarmEnabled, bSiren, bPresenceSuspend, bDebugStatus;


void rearm(unsigned char newMask);
