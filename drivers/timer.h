#include <project.h>

void timer0_init(void);
int8_t timer0_create_timer(uint16_t duration, void (*callback_fn)(void));
void timer0_destroy_timer(int8_t tid);
void timer0_start_timer(int8_t tid);
void timer0_stop_timer(int8_t tid);
