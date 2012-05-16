#ifndef BUZZER_H_
#define BUZZER_H_


extern void start_buzzer(uint8_t cycles, uint16_t on_time, uint16_t off_time);
extern void start_buzzer_steps(uint8_t cycles, uint16_t on_time, uint16_t off_time, uint16_t steps);
extern void stop_buzzer(void);
extern void toggle_buzzer(void);

typedef enum {
	BUZZER_OFF,
	BUZZER_ON_OUTPUT_DISABLED,
	BUZZER_ON_OUTPUT_ENABLED
} buzzer_state;


/* Buzzer output signal frequency = 32,768kHz/(BUZZER_TIMER_STEPS+1)/2 = 2.7kHz */
#define BUZZER_TIMER_STEPS	(5u)

/* Buzzer on time */
#define BUZZER_ON_TICKS		(CONV_MS_TO_TICKS(20))

/* Buzzer off time */
#define BUZZER_OFF_TICKS	(CONV_MS_TO_TICKS(200))

struct buzzer {
	// Keep output for "time" seconds
	uint8_t time;

	// On/off duty in ticks
	uint16_t on_time;
	uint16_t off_time;

	// Current buzzer output state
	buzzer_state state;

	// Current steps (~freq)
	// Frequency = f_ACLK/((steps+1) * 2)
	uint16_t steps;

};

#endif /*BUZZER_H_*/
