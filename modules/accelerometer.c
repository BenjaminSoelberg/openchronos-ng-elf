/*
    modules/accelerometer.c: Accelerometer for Openchronos

    Copyright (C) 2011-2012 Paolo Di Prodi <paolo@robomotic.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <openchronos.h>

/* driver */
#include <drivers/display.h>
#include <drivers/rtca.h>
#include <drivers/vti_as.h>

// *************************************************************************************************
// Defines section
#define DISPLAY_ACCEL_X		(0u)
#define DISPLAY_ACCEL_Y		(1u)
#define DISPLAY_ACCEL_Z		(2u)
#define DISPLAY_STATUS_RG	(3u)

#define ACCEL_MODE_OFF		(0u)
#define ACCEL_MODE_ON		(1u)

// Stop acceleration measurement after 60 minutes to save battery
#define ACCEL_MEASUREMENT_TIMEOUT		(60*60u)


// *************************************************************************************************
// Global Variable section
struct accel
{
	// ACC_MODE_OFF, ACC_MODE_ON
	uint8_t			mode;

	// Sensor raw data
	uint8_t			xyz[3];

	// Acceleration data in 10 * mgrav
	uint16_t		data;

	// Timeout
	uint16_t			timeout;

};
extern struct accel sAccel;

static enum {
	VIEW_CONFIG = 0,
	SET_CONFIG,
	VIEW_STATUS,
	VIEW_X,
	VIEW_Y,
	VIEW_Z
} config_state;

// *************************************************************************************************
// Global Variable section
struct accel sAccel;

// Global flag for proper acceleration sensor operation
extern uint8_t as_ok;

// *************************************************************************************************
// @fn          is_acceleration_measurement
// @brief       Returns 1 if acceleration is currently measured.
// @param       none
// @return      u8		1 = acceleration measurement ongoing
// *************************************************************************************************
uint8_t is_acceleration_measurement(void)
{
	return ((sAccel.mode == ACCEL_MODE_ON) && (sAccel.timeout > 0));
}


/* NUM (#) button pressed callback */
static void num_pressed()
{


}

/* Star button long press callback. */
static void star_long_pressed()
{


}

static void as_event(enum sys_message msg)
{

	//Check the register for status
	as_status.all_flags=as_get_status();

	if(as_status.int_status.falldet || as_status.int_status.motiondet){

		// request.flag.alarm_buzzer=1; BEEEEEEEEEEP
		display_chars(NULL,  LCD_SEG_L2_5_0, _itoa(as_status.all_flags, 3, 0), SEG_SET);

	}

	as_get_data(sAccel.xyz);

}

/* Enter the accelerometer menu */
static void acc_activated()
{
	uint8_t * str;
	uint8_t raw_data;
	uint16_t accel_data;
	uint8_t check;

	sys_messagebus_register(&as_event, SYS_MSG_AS_INT);
	// Show warning if acceleration sensor was not initialised properly
		if (!as_ok)
		{
			display_chars(NULL, LCD_SEG_L1_1_0, (uint8_t*)"ERR", SEG_SET);
		}
		else
		{

			// Clear previous acceleration value
			sAccel.data = 0;
			//now go in free fall mode
			/*
			// 2 g range
			as_config.range=2;
			// 100 Hz sampling rate
			as_config.sampling=SAMPLING_100_HZ;
			//time window is 10 msec for free fall and 100 msec for activity
			//2g multiple 71 mg
			as_config.FFTHR=0x09;
			//as_config.MDTHR=0x0B;
			as_config.MDFFTMR=4;
			// Start sensor in free fall
			as_start(FALL_MODE);
			 */

			// 2 g range
			as_config.range=2;
			// 100 Hz sampling rate
			as_config.sampling=SAMPLING_10_HZ;
			//time window is 10 msec for free fall and 100 msec for activity
			//2g multiple 71 mg: 0F=4 * 71 mg= 1.065 g
			as_config.MDTHR=1;
			as_config.MDFFTMR=1;
			// Start sensor in motion detection mode
			as_start(ACTIVITY_MODE);


			if (!as_ok)
			{
				display_chars(NULL, LCD_SEG_L1_1_0, (uint8_t*)"FAIL", SEG_SET);
			}


			// check if that is really in the mode we set

			display_chars(NULL, LCD_SEG_L1_3_0, _itoa(as_read_register(ADDR_CTRL), 3, 0), SEG_SET);
			display_chars(NULL, LCD_SEG_L2_5_0, _itoa(as_read_register(ADDR_MDFFTMR), 5, 0), SEG_SET);


			// Set timeout counter
			sAccel.timeout = ACCEL_MEASUREMENT_TIMEOUT;

			// Set mode
			sAccel.mode = ACCEL_MODE_ON;

			// Start with register values
			config_state=VIEW_CONFIG;

		}
}

/* Exit the accelerometer menu. */
static void acc_deactivated()
{
	sys_messagebus_unregister(&as_event);
	// Stop acceleration sensor
	as_stop();

	// Clear mode
	sAccel.mode = ACCEL_MODE_OFF;

	/* clean up real screen */
	display_clear(NULL, 1);
	display_clear(NULL, 2);

}

static void up_btn()
{


}

static void down_btn()
{


}

void accelerometer_init()
{

	menu_add_entry(&up_btn, &down_btn,
			&num_pressed,
			&star_long_pressed,
			NULL,NULL,
			&acc_activated,
			&acc_deactivated);

}
