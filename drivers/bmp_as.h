// *************************************************************************************************
//
//	Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/ 
//	 
//	 
//	  Redistribution and use in source and binary forms, with or without 
//	  modification, are permitted provided that the following conditions 
//	  are met:
//	
//	    Redistributions of source code must retain the above copyright 
//	    notice, this list of conditions and the following disclaimer.
//	 
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the 
//	    documentation and/or other materials provided with the   
//	    distribution.
//	 
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//	
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************

#ifndef BMP_AS_H_
#define BMP_AS_H_


// *************************************************************************************************
// Include section


// *************************************************************************************************

typedef struct
{
     struct
     {
	  unsigned int x : 1;
	  unsigned int y : 1;
	  unsigned int z : 1;
     } slope_interrupt;
     struct
     {
	  unsigned int x : 1;
	  unsigned int y : 1;
	  unsigned int z : 1;
     } high_interrupt;
     unsigned int flat_interrupt : 1;
     unsigned int orient_interrupt : 1;
     unsigned int tap_interrupt : 2; // 1 single, 2/3 double, 0 none
     unsigned int low_interrupt : 1;
     unsigned int new_interrupt : 1;
} bmp_as_interrupts_t;

typedef struct
{
     bmp_as_interrupts_t int_raised;
     struct
     {
	  unsigned int sign : 1; // 1 -, 0 +
	  unsigned int first_x : 1;
	  unsigned int first_y : 1;
	  unsigned int first_z : 1;
     } high;
     struct
     {
	  unsigned int sign : 1;
	  unsigned int first_x : 1;
	  unsigned int first_y : 1;
	  unsigned int first_z : 1;
     } tap;

     struct
     {
	  unsigned int sign : 1;
	  unsigned int first_x : 1;
	  unsigned int first_y : 1;
	  unsigned int first_z : 1;
     } slope;
     unsigned int orient_z : 1; // 0 upward, 1 downward
     unsigned int orient_xy : 2; // 00 portrait upright, 01 portrait upside-down, 10, landscape left, 11 landscape right

} bmp_as_status_t;

// Prototypes section
extern void as_init(void);
extern void as_start(void);
extern void as_stop(void);
extern uint8_t as_read_register(uint8_t bAddress);
extern uint8_t as_write_register(uint8_t bAddress, uint8_t bData);
extern void bmp_as_start(uint8_t bGRange, uint8_t bBwd, uint8_t bSleep, uint8_t filtering);
extern void bmp_as_stop(void);
extern uint8_t bmp_as_read_register(uint8_t bAddress);
extern uint8_t bmp_as_write_register(uint8_t bAddress, uint8_t bData);
extern void bmp_as_get_data(int16_t * axes);
extern void bmp_as_enable_interrupts(bmp_as_interrupts_t interrupts);
extern void bmp_as_disable_interrupts(void);
extern bmp_as_status_t bmp_as_process_interrupt(void);
extern bmp_as_status_t bmp_as_init_status(void);
extern bmp_as_interrupts_t bmp_as_init_interrupts(void);

// *************************************************************************************************
// Defines section

#define BMP_RSTKEY           (0xB6)

/********************************************************************
 *
 * Bosch BMA250
 * Register Map
 *
 ********************************************************************/

#define BMP_CHIPID           (0x00)
#define BMP_ACC_X_LSB        (0x02)
#define BMP_ACC_X_MSB        (0x03)
#define BMP_ACC_Y_LSB        (0x04)
#define BMP_ACC_Y_MSB        (0x05)
#define BMP_ACC_Z_LSB        (0x06)
#define BMP_ACC_Z_MSB        (0x07)

#define BMP_GRANGE           (0x0F)	   // g Range
#define BMP_BWD              (0x10)	   // Bandwidth
#define BMP_PM               (0x11)	   // Power modes
#define BMP_SCR              (0x13)	   // Special Control Register
#define BMP_RESET            (0x14)	   // Soft reset register (writing 0xB6 causes reset)
#define BMP_ISR1             (0x16)	   // Interrupt settings register 1
#define BMP_ISR2             (0x17)	   // Interrupt settings register 2
#define BMP_IMR1             (0x19)	   // Interrupt mapping register 1
#define BMP_IMR2             (0x1A)	   // Interrupt mapping register 2
#define BMP_IMR3             (0x1B)	   // Interrupt mapping register 3

/* Configuration constants */

#define BMP_GRANGE_2G (0x03) // +- 2g
#define BMP_GRANGE_4G (0x05) // -+ 4g
#define BMP_GRANGE_8G (0x08) // -+ 8g
#define BMP_GRANGE_16G (0x0C) // -+ 16g

#define BMP_SLEEP_NO (0x00)
#define BMP_SLEEP_05MS (0x4A) // Approx. 100uA current consumption
#define BMP_SLEEP_1MS (0x4C)
#define BMP_SLEEP_2MS (0x4E) // Approx. 55uA
#define BMP_SLEEP_4MS (0x50)
#define BMP_SLEEP_6MS (0x52)
#define BMP_SLEEP_10MS (0x54) // Approx. 16uA
#define BMP_SLEEP_25MS (0x56)
#define BMP_SLEEP_50MS (0x58)
#define BMP_SLEEP_100MS (0x5A)
#define BMP_SLEEP_500MS (0x5C)
#define BMP_SLEEP_1000MS (0x5E) // Approx. 0.7uA

#define BMP_BWD_7HZ (0x08) // 7.81 Hz
#define BMP_BWD_15HZ (0x09) // 15.63 Hz
#define BMP_BWD_31HZ (0x0A) // 31.25 Hz
#define BMP_BWD_62HZ (0x0B) // 62.5 Hz
#define BMP_BWD_125HZ (0x0C) // 125 Hz
#define BMP_BWD_250HZ (0x0D) // 250 Hz
#define BMP_BWD_500HZ (0x0E) // 500 Hz
#define BMP_BWD_1000HZ (0x0F) // 1000 Hz

// *************************************************************************************************
// Global Variable section


// *************************************************************************************************
// Extern section


#endif /*BMP_AS_H_*/
