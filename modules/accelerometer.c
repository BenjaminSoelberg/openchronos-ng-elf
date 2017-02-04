/**
    modules/accelerometer.c: Accelerometer for Openchronos

    Copyright (C) 2011-2012 Paolo Di Prodi <paolo@robomotic.com>
    Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

    This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "messagebus.h"
#include "menu.h"

/* drivers */
#include "drivers/rtca.h"
#include "drivers/display.h"
#include "drivers/vti_as.h"
#include "drivers/buzzer.h"

// *************************************************************************************************
// Defines section

#define BIT(x) (1uL << (x))

#define ACCEL_MODE_OFF      (0u)
#define ACCEL_MODE_ON       (1u)
#define ACCEL_MODE_BACKGROUND   (2u)

#define DISPLAY_ACCEL_X     (0u)
#define DISPLAY_ACCEL_Y     (1u)
#define DISPLAY_ACCEL_Z     (2u)

// Stop acceleration measurement after 60 minutes to save battery
// This parameter is ignored if in background mode!
#define ACCEL_MEASUREMENT_TIMEOUT       (60u)

// Conversion values from data to mgrav taken from CMA3000-D0x datasheet (rev 0.4, table 4)
const uint16_t mgrav_per_bit[7] = { 18, 36, 71, 143, 286, 571, 1142 };


// *** Tunes for accelerometer synestesia

static note smb[] = {0x2588, 0x000F};

// *************************************************************************************************
// Global Variable section
struct accel
{
    // ACC_MODE_OFF, ACC_MODE_ON, ACCEL_MODE_BACKGROUND
    uint8_t         mode;

    // Sensor raw data
    uint8_t         xyz[3];



    // Acceleration data in 10 * mgrav
    uint16_t        data;

    // Sensor old data for FIR filter
    uint16_t        data_prev;

    // Timeout: should be decreased with the 1 minute RTC event
    uint16_t            timeout;
    // Display X/Y/Z values
    uint8_t             view_style;
};
extern struct accel sAccel;



static enum {
    VIEW_SET_MODE = 0,
    VIEW_SET_PARAMS,
    VIEW_STATUS,
    VIEW_AXIS,
    VIEWMODE_MAX
} submenu_state;


struct accel sAccel;

// Global flag for proper acceleration sensor operation
extern uint8_t as_ok;

// *************************************************************************************************
// @fn          is_acceleration_measurement
// @brief       Returns 1 if acceleration is currently measured.
// @param       none
// @return      u8      1 = acceleration measurement ongoing
// *************************************************************************************************
uint8_t is_acceleration_measurement(void)
{
    return ((sAccel.mode == ACCEL_MODE_ON) && (sAccel.timeout > 0));
}

// *************************************************************************************************
// @fn          acceleration_value_is_positive
// @brief       Returns 1 if 2's complement number is positive
// @param       u8 value    2's complement number
// @return      u8          1 = number is positive, 0 = number is negavtive
// *************************************************************************************************
uint8_t acceleration_value_is_positive(uint8_t value)
{
    return ((value & (BIT7)) == 0);
}


// *************************************************************************************************
// @fn          convert_acceleration_value_to_mgrav
// @brief       Converts measured value to mgrav units
// @param       u8 value    g data from sensor
// @return      u16         Acceleration (mgrav)
// *************************************************************************************************
uint16_t convert_acceleration_value_to_mgrav(uint8_t value)
{
    uint16_t result;
    uint8_t i;

    if (!acceleration_value_is_positive(value))
    {
        // Convert 2's complement negative number to positive number
        value = ~value;
        value += 1;
    }

    result = 0;
    for (i=0; i<7; i++)
    {

        result += ((value & (BIT(i)))>>i) * mgrav_per_bit[i];
    }

    return (result);
}

void update_menu()
{
    // Depending on the state what do we do?
    switch (submenu_state) {
        case VIEW_SET_MODE:

            if(as_config.mode==FALL_MODE)
                display_chars(0, LCD_SEG_L1_3_0 , "FALL", SEG_SET);
            else if(as_config.mode==MEASUREMENT_MODE)
                display_chars(0, LCD_SEG_L1_3_0 , "MEAS", SEG_SET);
            else if(as_config.mode==ACTIVITY_MODE)
                display_chars(0, LCD_SEG_L1_3_0 , "ACTI", SEG_SET);

            display_chars(0, LCD_SEG_L2_4_0 , "MODE", SEG_SET);
            break;

        case VIEW_SET_PARAMS:

            display_chars(0, LCD_SEG_L2_5_0 , "SETS", SEG_SET);
            break;

        case VIEW_STATUS:

            display_chars(0,LCD_SEG_L2_5_0 , "STAT", SEG_SET);
            break;

        case VIEW_AXIS:

            display_chars(0,LCD_SEG_L2_5_0 , "DATA", SEG_SET);
            break;

        default:
            break;
    }

}

/* NUM (#) button pressed callback */
// This change the sub menu
static void num_pressed()
{
    // Change the sub menu code
    submenu_state++;
    submenu_state %= VIEWMODE_MAX;
    // now update the menu
    update_menu();


}

static void up_btn()
{

    // Depending on the state what do we do?
    switch (submenu_state) {
        case VIEW_SET_MODE:
            as_config.mode++;
            as_config.mode %= 3;
            change_mode(as_config.mode);
            update_menu();

            break;

        case VIEW_SET_PARAMS:
            _printf(0,LCD_SEG_L1_3_0 , "%04x", as_read_register(ADDR_CTRL));
            break;

        case VIEW_STATUS:
            _printf(0,LCD_SEG_L1_3_0, "%1u", as_status.all_flags);

            break;

        case VIEW_AXIS:

            display_chars(0,LCD_SEG_L1_3_0 , "TODO", SEG_SET);
            break;

        default:
            break;
    }
    /* update menu screen */
    lcd_screen_activate(0);
}

static void down_btn()
{
    //not necessary at the moment
    /* update menu screen */
    lcd_screen_activate(0);
}

/* Star button long press callback. */
// This set/unset the background mode
static void star_long_pressed()
{
    if(sAccel.mode==ACCEL_MODE_ON)
    {
        sAccel.mode = ACCEL_MODE_BACKGROUND;
        //set the R symbol on so that is propagated when the user leaves the mode
        display_symbol(0, LCD_ICON_RECORD, SEG_SET | BLINK_ON);
    }
    else if(sAccel.mode==ACCEL_MODE_BACKGROUND)
    {

        sAccel.mode = ACCEL_MODE_ON;
        //unset the R symbol
        display_symbol(0, LCD_ICON_RECORD, SEG_SET | BLINK_OFF);
    }
    /* update menu screen */
    lcd_screen_activate(0);
}

void display_data(uint8_t display_id)
{
    uint8_t raw_data=0;
    uint16_t accel_data=0;

    // Convert X/Y/Z values to mg
    switch (sAccel.view_style)
    {
        case DISPLAY_ACCEL_X:
            raw_data = sAccel.xyz[0];
            display_char(display_id,LCD_SEG_L1_3, 'X', SEG_ON);
            break;
        case DISPLAY_ACCEL_Y:
            raw_data = sAccel.xyz[1];
            display_char(display_id,LCD_SEG_L1_3, 'Y', SEG_ON);
            break;
        case DISPLAY_ACCEL_Z:
            raw_data = sAccel.xyz[2];
            display_char(display_id,LCD_SEG_L1_3, 'Z', SEG_ON);
            break;
    }

    // Filter acceleration
    #ifdef FIXEDPOINT
    accel_data = (uint16_t)(((sAccel.data_prev * 2) + (sAccel.data * 8)) / 10);
    #else
    accel_data = (uint16_t)((sAccel.data_prev * 0.2) + (sAccel.data * 0.8));
    #endif

    // Store average acceleration
    sAccel.data = sAccel.data_prev;

    // Display acceleration in x.xx format in the second screen this is real time!
    display_chars(display_id,LCD_SEG_L1_2_0, _sprintf("%03s", accel_data), SEG_ON);

    // Display sign
    if (acceleration_value_is_positive(raw_data)) {
        display_symbol(display_id,LCD_SYMB_ARROW_UP, SEG_ON);
        display_symbol(display_id,LCD_SYMB_ARROW_DOWN, SEG_OFF);
    } else {
        display_symbol(display_id,LCD_SYMB_ARROW_UP, SEG_OFF);
        display_symbol(display_id,LCD_SYMB_ARROW_DOWN, SEG_ON);
    }
}
static void as_event(enum sys_message msg)
{


    if ( (msg & SYS_MSG_RTC_MINUTE) == SYS_MSG_RTC_MINUTE)
    {
        if(sAccel.mode == ACCEL_MODE_ON) sAccel.timeout--;
        //if timeout is over disable the accelerometer
        if(sAccel.timeout<1)
        {
            //disable accelerometer to save power
            as_stop();
            //update the mode to remember
            sAccel.mode = ACCEL_MODE_OFF;
        }

    }
    if ( (msg & SYS_MSG_AS_INT) == SYS_MSG_AS_INT)
    {
        //Check the vti register for status information
        as_status.all_flags=as_get_status();
        //TODO For debugging only
        _printf(0, LCD_SEG_L1_1_0, "%1u", as_status.all_flags);
        buzzer_play(smb);
        //if we were in free fall or motion detection mode check for the event
        if(as_status.int_status.falldet || as_status.int_status.motiondet){

            //if such an event is detected enable the symbol
            //display_symbol(0, LCD_ICON_ALARM , SEG_SET | BLINK_ON);

            //read the data
            as_get_data(sAccel.xyz);
            //display_data(0);
            /* update menu screen */
            lcd_screen_activate(0);

        }//if we were in measurment mode do a measurement and put it in the virtual screen
        else
        {

            //display_symbol(0, LCD_ICON_ALARM , SEG_SET | BLINK_OFF);
            display_data(1);
            /* refresh to accelerometer screen only if in that modality */
            if (submenu_state== VIEW_AXIS )lcd_screen_activate(1);

        }
    }
    /* The 1 Hz timer is used to refresh the menu screen */
    if ( (msg & SYS_MSG_RTC_SECOND) == SYS_MSG_RTC_SECOND)
    {
    /*check the status register for debugging purposes */
    _printf(0, LCD_SEG_L1_1_0, "%1u", as_read_register(ADDR_INT_STATUS));
    /* update menu screen */
    lcd_screen_activate(0);
    }

}

/* Enter the accelerometer menu */
static void acc_activated()
{


    //register to the system bus for vti events as well as the RTC minute events
    sys_messagebus_register(&as_event, SYS_MSG_AS_INT | SYS_MSG_RTC_MINUTE | SYS_MSG_RTC_SECOND);


    /* create two screens, the first is always the active one */
    lcd_screens_create(2);

    /* screen 0 will contain the menu structure and screen 1 the raw accelerometer data */

    // Show warning if acceleration sensor was not initialised properly
    if (!as_ok)
    {
        display_chars(0, LCD_SEG_L1_3_0, "ERR", SEG_SET);
    }
    else
    {

        /* Initialization is required only if not in background mode! */
        if (sAccel.mode != ACCEL_MODE_BACKGROUND)
        {
            // Clear previous acceleration value
            sAccel.data = 0;
            // 2 g range
            as_config.range=2;
            // 100 Hz sampling rate
            as_config.sampling=SAMPLING_10_HZ;
            // keep mode
            as_config.mode=ACTIVITY_MODE;
            //time window is 10 msec for free fall and 100 msec for activity
            //2g multiple 71 mg: 0F=4 * 71 mg= 1.065 g
            as_config.MDTHR=2;
            as_config.MDFFTMR=1;

            // Set timeout counter
            sAccel.timeout = ACCEL_MEASUREMENT_TIMEOUT;

            // Set mode for screen
            sAccel.mode = ACCEL_MODE_ON;

            // Start with mode selection
            submenu_state=VIEW_SET_MODE;

            // Select Axis X
            sAccel.view_style=DISPLAY_ACCEL_Z;

            // Start sensor in motion detection mode
            as_start(ACTIVITY_MODE);
            // After this call interrupts will be generated
        }

        if (!as_ok)
        {
            display_chars(0, LCD_SEG_L1_3_0, "FAIL", SEG_SET);
        }
        display_chars(0, LCD_SEG_L1_3_0 , "ACTI", SEG_SET);
        display_chars(0, LCD_SEG_L2_4_0 , "MODE", SEG_SET);


    }
    /* return to main screen */
    lcd_screen_activate(0);
}

void print_debug()
{
        // check if that is really in the mode we set

        _printf(0, LCD_SEG_L1_3_0, "%03x", as_read_register(ADDR_CTRL));
        _printf(0, LCD_SEG_L2_5_0, "%05x", as_read_register(ADDR_MDFFTMR));

}


/* Exit the accelerometer menu. */
/* Here we could decide to keep it in the background or not and
** also check when activated if it was in background */

static void acc_deactivated()
{


    /* destroy virtual screens */
    lcd_screens_destroy();

    /* clean up screen */

    display_clear(0, 1);
    display_clear(0, 2);


    /* do not disable anything if in background mode */
    if  (sAccel.mode == ACCEL_MODE_BACKGROUND) return;
    else    /* clear symbols only if not in backround mode*/
    //display_symbol(0, LCD_ICON_ALARM , SEG_SET | BLINK_OFF);

    /* otherwise shutdown all the stuff
    ** deregister from the message bus */
    sys_messagebus_unregister_all(&as_event);
    /* Stop acceleration sensor */
    as_stop();

    /* Clear mode */
    sAccel.mode = ACCEL_MODE_OFF;
}



void mod_accelerometer_init()
{

    //if this is called only one time after reboot there are some important things to initialise
    //Initialise sAccel struct?
    sAccel.data=0;
    sAccel.data_prev=0;
    // Set timeout counter
    sAccel.timeout = ACCEL_MEASUREMENT_TIMEOUT;
    /* Clear mode */
    sAccel.mode = ACCEL_MODE_OFF;

    menu_add_entry("ACC",
                   &up_btn,
                   &down_btn,
                   &num_pressed,
                   &star_long_pressed,
                   NULL,
                   NULL,
                   &acc_activated,
                   &acc_deactivated);

}
