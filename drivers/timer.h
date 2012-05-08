#include <ezchronos.h>

/* Timer0, the main hw timer */
void timer0_init(void);

/* Programable timer */
void timer0_create_prog_timer(uint16_t duration, void (*callback_fn)(void));
void timer0_destroy_prog_timer();

/* Delay timer */
void timer0_delay(uint16_t duration);

/* Fixed frequency timer queues */
struct cblist *timer0_1hz_queue;
struct cblist *timer0_10hz_queue;

