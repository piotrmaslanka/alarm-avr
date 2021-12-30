#define PINS_SIREN 1
#define PINS_ARMLED 2

extern unsigned char baTrigger[8];
extern unsigned int iTimer;

void pins_out(unsigned char what, unsigned char status);
void pins_init();
