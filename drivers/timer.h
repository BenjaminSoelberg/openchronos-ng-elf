#include <ezchronos.h>

/* Timer0, the main hw timer */
void timer0_init(void);

/* Delay timers */
void timer0_delay(uint16_t duration);

/* Fixed frequency timers */
void timer0_1hz_register(void (*callback)(void));
void timer0_1hz_unregister(void (*callback)(void));
