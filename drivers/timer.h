#include <ezchronos.h>

/* Timer0, the main hw timer */
void timer0_init(void);

/* Delay timers */
void timer0_delay(uint16_t duration);

/* Fixed frequency timer queues */
struct cblist *timer0_1hz_queue;
struct cblist *timer0_10hz_queue;

