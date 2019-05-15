// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/ 
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
// Bosch BMA250 acceleration sensor driver functions
// *************************************************************************************************

// For advanced functions see datasheet: http://www1.futureelectronics.com/doc/BOSCH/BMA250-0273141121.pdf

// *************************************************************************************************
// Include section

// system
#include <string.h>
#include "openchronos.h"

// driver
#include "bmp_as.h"
#include "as.h"
#include "timer.h"
#include "display.h"


// *************************************************************************************************
// Prototypes section

// *************************************************************************************************
// Defines section

// =================================================================================================
// BMA250 acceleration sensor configuration
// =================================================================================================
// DCO frequency division factor determining speed of the acceleration sensor SPI interface
// Speed in Hz = 12MHz / AS_BR_DIVIDER (max. 10MHz)
#define BMP_AS_BR_DIVIDER  (2u)

// *************************************************************************************************
// Global Variable section

// *************************************************************************************************
// Extern section


// *************************************************************************************************
// @fn          bmp_as_start
// @brief       Power-up and initialize acceleration sensor
// @param       none
// @return      none
// *************************************************************************************************
void bmp_as_start(uint8_t bGRange, uint8_t bBwd, uint8_t bSleep, uint8_t filtering)
{	
     // Initialize SPI interface to acceleration sensor
     AS_SPI_CTL0 |= UCSYNC | UCMST | UCMSB        // SPI master, 8 data bits,  MSB first,
	  | UCCKPH;                     //  clock idle low, data output on falling edge
     AS_SPI_CTL1 |= UCSSEL1;                      // SMCLK as clock source
     AS_SPI_BR0   = BMP_AS_BR_DIVIDER;            // Low byte of division factor for baud rate
     AS_SPI_BR1   = 0x00;                         // High byte of division factor for baud rate
     AS_SPI_CTL1 &= ~UCSWRST;                     // Start SPI hardware
  
     // Configure interface pins
     as_start();

     // write sensor configuration
     bmp_as_write_register(BMP_GRANGE, bGRange);  // Set measurement range
     bmp_as_write_register(BMP_BWD, bBwd);        // Set filter bandwidth
     bmp_as_write_register(BMP_PM, bSleep);       // Set powermode


     if (filtering)
	  bmp_as_write_register(BMP_SCR, 0x80);        // acquire unfiltered acceleration data
  
     // configure sensor interrupt
     bmp_as_write_register(BMP_IMR2, 0x01);       // map new data interrupt to INT1 pin
     bmp_as_write_register(BMP_ISR2, 0x10);       // enable new data interrupt
	
     // enable CC430 interrupt pin for data read out from acceleration sensor
     AS_INT_IFG &= ~AS_INT_PIN;                   // Reset flag
     AS_INT_IE  |=  AS_INT_PIN;                   // Enable interrupt
}

// *************************************************************************************************
// @fn          bmp_as_stop
// @brief       Power down acceleration sensor
// @param       none
// @return      none
// *************************************************************************************************
void bmp_as_stop(void)
{
     as_stop();
}


// *************************************************************************************************
// @fn          bmp_as_read_register
// @brief       Read a byte from the acceleration sensor
// @param       uint8_t bAddress		        Register address
// @return      uint8_t					    Register content
// *************************************************************************************************
uint8_t bmp_as_read_register(uint8_t bAddress)
{
     bAddress |= BIT7;                   // set R/W bit for reading
  
     return as_read_register(bAddress);
}

// *************************************************************************************************
// @fn          bmp_as_write_register
// @brief  		Write a byte to the acceleration sensor
// @param       uint8_t bAddress		        Register address
//				uint8_t bData			    Data to write
// @return      uint8_t					
// *************************************************************************************************
uint8_t bmp_as_write_register(uint8_t bAddress, uint8_t bData) // NON SCRIVE!!!
{
     //  bAddress &= ~BIT8;                   // R/W bit to be not set
     bAddress &= ~BIT7;
  
     return as_write_register(bAddress, bData);
}


// *************************************************************************************************
// @fn          bmp_as_get_data
// @brief       Service routine to read acceleration values.
// @param       int16_t *axes                            array containing the acceleration values
// @return      none
// *************************************************************************************************
void bmp_as_get_data(int16_t *axes)
{
     uint8_t i;
     uint16_t data[3];
     // Exit if sensor is not powered up
     if ((AS_PWR_OUT & AS_PWR_PIN) != AS_PWR_PIN) return;
  
     // Read LSB from LSB acceleration data first to update MSB (shadowing is enabled by default)
     data[0] = ((uint32_t) (bmp_as_read_register(BMP_ACC_X_LSB) & 0xC0)) >> 6;
     data[1] = ((uint32_t) (bmp_as_read_register(BMP_ACC_Y_LSB) & 0xC0)) >> 6;
     data[2] = ((uint32_t) (bmp_as_read_register(BMP_ACC_Z_LSB) & 0xC0)) >> 6;
     // Store X/Y/Z MSB acceleration data in buffer
     data[0] += ((uint32_t) bmp_as_read_register(BMP_ACC_X_MSB)) << 2;
     data[1] += ((uint32_t) bmp_as_read_register(BMP_ACC_Y_MSB)) << 2;
     data[2] += ((uint32_t) bmp_as_read_register(BMP_ACC_Z_MSB)) << 2;


     // Convert the values from 10 bit two's complement uint to 16 bit int
     for (i = 0; i < 3; i++)
	  if (data[i] & 0x0200)
	  {
	       data[i] = (~data[i] & 0x03FF) + 1;
	       axes[i] = -((int32_t) data[i]);
	  }
	  else
	       axes[i] = data[i];
  
}

// *************************************************************************************************
// @fn          bmp_as_enable_interrupts
// @brief       Enables accelerometer's interrupts on interrupt pin 1 (pin 2 is not connected to the
//              ez-Chronos).
// @param       bmp_as_interrupts_t                            Interrupt data structure.
// @return      none
// *************************************************************************************************
void bmp_as_enable_interrupts(bmp_as_interrupts_t interrupts)
{
     uint8_t val = 0;

     // Write the interrupt registers (BMP_ISR1 and BMP_ISR2)
     if (interrupts.flat_interrupt)
	  val |= BIT7;
     if (interrupts.orient_interrupt)
	  val |= BIT6;
     if (interrupts.tap_interrupt == 1)
	  val |= BIT5;
     else if (interrupts.tap_interrupt != 0)
	  val |= BIT4;
     if (interrupts.slope_interrupt.z)
	  val |= BIT2;
     if (interrupts.slope_interrupt.y)
	  val |= BIT1;
     if (interrupts.slope_interrupt.x)
	  val |= BIT0;

     as_write_register(BMP_ISR1, val);

     val = 0;

     if (interrupts.new_interrupt)
	  val |= BIT4;
     if (interrupts.low_interrupt)
	  val |= BIT3;
     if (interrupts.high_interrupt.z)
	  val |= BIT2;
     if (interrupts.high_interrupt.y)
	  val |= BIT1;
     if (interrupts.high_interrupt.x)
	  val |= BIT0;

     as_write_register(BMP_ISR2, val);
  

     // Map every interrupt to pin INT1 (SYS_MSG_AS_INT in messagebus.h). Pin INT2 is NOT connected.
     as_write_register(BMP_IMR1, 0xf7);
     as_write_register(BMP_IMR2, 0x01);
     timer0_delay(100, LPM3_bits);
}

// *************************************************************************************************
// @fn          bmp_as_disable_interrupts
// @brief       Disables accelerometer's interrupts.
// @param       none
// @return      none
// *************************************************************************************************
void bmp_as_disable_interrupts(void)
{
     as_write_register(BMP_ISR1, 0x00);
     as_write_register(BMP_ISR2, 0x00);
     as_write_register(BMP_IMR1, 0x00);
     as_write_register(BMP_IMR2, 0x00);
     as_write_register(BMP_IMR3, 0x00);
     timer0_delay(100, LPM3_bits);
}


// *************************************************************************************************
// @fn          bmp_as_process_interrupt
// @brief       Processes a generated interrupt.
// @param       none
// @return      bmp_as_status_t                                Status data structure.
// *************************************************************************************************
bmp_as_status_t bmp_as_process_interrupt(void)
{
     bmp_as_status_t ret = bmp_as_init_status();
     uint8_t reg, tmp;
  
     reg = bmp_as_read_register(0x09);

     // Process tap and slope interrupts.
     tmp = bmp_as_read_register(0x0b);
     if (reg & BIT2)
     {
	  ret.int_raised.slope_interrupt.x = 1;
	  ret.slope.sign = (tmp & BIT3)? 1: 0;
	  ret.slope.first_z = (tmp & BIT2)? 1: 0;
	  ret.slope.first_y = (tmp & BIT1)? 1: 0;
	  ret.slope.first_x = (tmp & BIT0)? 1: 0;
     }
     if (reg & BIT4)
     {
	  ret.int_raised.tap_interrupt = 2;
	  ret.tap.sign = (tmp & BIT7)? 1: 0;
	  ret.tap.first_z = (tmp & BIT6)? 1: 0;
	  ret.tap.first_y = (tmp & BIT5)? 1: 0;
	  ret.tap.first_x = (tmp & BIT4)? 1: 0;
     }
     if (reg & BIT5)
     {
	  ret.int_raised.tap_interrupt = 1;
	  ret.tap.sign = (tmp & BIT7)? 1: 0;
	  ret.tap.first_z = (tmp & BIT6)? 1: 0;
	  ret.tap.first_y = (tmp & BIT5)? 1: 0;
	  ret.tap.first_x = (tmp & BIT4)? 1: 0;
     }

     // Process flat, orient and high-g interrupt.
     tmp = bmp_as_read_register(0x0c);
     if (reg & BIT7)
	  ret.int_raised.flat_interrupt = (tmp & BIT7)? 1: 0;
     if (reg & BIT1)
     {
	  ret.high.sign = (tmp & BIT3)? 1: 0;
	  ret.high.first_z = (tmp & BIT2)? 1: 0;
	  ret.high.first_y = (tmp & BIT1)? 1: 0;
	  ret.high.first_x = (tmp & BIT0)? 1: 0;
	  ret.int_raised.high_interrupt.x = 1;
     }
     if (reg & BIT6)
     {
	  ret.int_raised.orient_interrupt = 1;
	  ret.orient_z = (tmp & BIT6)? 1: 0;
	  ret.orient_xy = ((tmp & BIT5)? 2 : 0) + ((tmp & BIT4)? 1 : 0);
     }

     // Process low-g interrupt.
     if (reg & BIT0)
	  ret.int_raised.low_interrupt = 1;

     // Process new data interrupt.
     reg = bmp_as_read_register(0x0a);
     if (reg & BIT7)
	  ret.int_raised.new_interrupt = 1;
    
     return ret;
}

// *************************************************************************************************
// @fn          bmp_as_init_interrupts
// @brief       Creates an empty interrupt data structure.
// @param       none
// @return      bmp_as_interrupts_t                            Empty interrupt data structure.
// *************************************************************************************************
bmp_as_interrupts_t bmp_as_init_interrupts(void)
{
     bmp_as_interrupts_t ret;
     memset(&ret, 0, sizeof(bmp_as_interrupts_t));

     return ret;
}

// *************************************************************************************************
// @fn          bmp_as_init_status
// @brief       Creates an empty status data structure.
// @param       none
// @return      bmp_as_status_t                            Empty status data structure.
// *************************************************************************************************
bmp_as_status_t bmp_as_init_status(void)
{
     bmp_as_status_t ret;
     memset(&ret, 0, sizeof(bmp_as_status_t));
  
     return ret;
}
