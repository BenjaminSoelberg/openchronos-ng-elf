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
// acceleration sensor driver functions
// *************************************************************************************************


// *************************************************************************************************
// Include section

// system
//#include "project.h"

// driver
#include "openchronos.h"
#include "as.h"
#include "timer.h"


// *************************************************************************************************
// Prototypes section

// *************************************************************************************************
// Defines section

// *************************************************************************************************
// Global Variable section

// Global flag for proper acceleration sensor operation

// *************************************************************************************************
// Extern section








// *************************************************************************************************
// @fn          as_init
// @brief       Setup acceleration sensor connection, do not power up yet
// @param       none
// @return      none
// *************************************************************************************************
void as_init(void)
{
     // Deactivate connection to acceleration sensor
     AS_PWR_OUT &= ~AS_PWR_PIN;         // Power off
     AS_INT_OUT &= ~AS_INT_PIN;         // Pin to low to avoid floating pins
     AS_SPI_OUT &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN); // Pin to low to avoid floating pins
     AS_CSN_OUT &= ~AS_CSN_PIN;         // Pin to low to avoid floating pins
     AS_INT_DIR |= AS_INT_PIN;          // Pin to output to avoid floating pins
     AS_SPI_DIR |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN; // Pin to output to avoid floating pins
     AS_CSN_DIR |= AS_CSN_PIN;          // Pin to output to avoid floating pins
     AS_PWR_DIR |= AS_PWR_PIN;          // Power pin to output direction
}

// *************************************************************************************************
// @fn          as_start
// @brief       Power-up and initialize acceleration sensor
// @param       none
// @return      none
// *************************************************************************************************
void as_start(void)
{
     // Initialize interrupt pin for data read out from acceleration sensor
     AS_INT_IES &= ~AS_INT_PIN;                   // Interrupt on rising edge

     // Enable interrupt
     AS_INT_DIR &= ~AS_INT_PIN;                   // Switch INT pin to input
     AS_SPI_DIR &= ~AS_SDI_PIN;                   // Switch SDI pin to input
     AS_SPI_REN |= AS_SDI_PIN;                    // Pulldown on SDI pin
     AS_SPI_SEL |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN; // Port pins to SDO, SDI and SCK function
     AS_CSN_OUT |= AS_CSN_PIN;                    // Deselect acceleration sensor
     AS_PWR_OUT |= AS_PWR_PIN;                    // Power on active high

     // Delay of >5ms required between switching on power and configuring sensor
     timer0_delay(10, LPM3_bits);

     // Initialize interrupt pin for data read out from acceleration sensor
     AS_INT_IFG &= ~AS_INT_PIN;                   // Reset flag
     AS_INT_IE |= AS_INT_PIN;                     // Enable interrupt
}

// *************************************************************************************************
// @fn          as_stop
// @brief       Power down acceleration sensor
// @param       none
// @return      none
// *************************************************************************************************
void as_stop(void)
{
     // Disable interrupt
     AS_INT_IE &= ~AS_INT_PIN;                    // Disable interrupt

     // Power-down sensor
     AS_PWR_OUT &= ~AS_PWR_PIN;                   // Power off
     AS_INT_OUT &= ~AS_INT_PIN;                   // Pin to low to avoid floating pins
     AS_SPI_OUT &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN); // Pins to low to avoid floating pins
     AS_SPI_SEL &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN); // Port pins to I/O function
     AS_CSN_OUT &= ~AS_CSN_PIN;                   // Pin to low to avoid floating pins
     AS_INT_DIR |= AS_INT_PIN;                    // Pin to output to avoid floating pins
     AS_SPI_DIR |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN; // Pins to output to avoid floating pins
     AS_CSN_DIR |= AS_CSN_PIN;                    // Pin to output to avoid floating pins
}

// *************************************************************************************************
// @fn          as_read_register
// @brief       Read a byte from the acceleration sensor
// @param       uint8_t bAddress                     Register address
// @return      uint8_t bResult                      Register content
//                                                                      If the returned value is 0,
// there was an error.
// *************************************************************************************************
uint8_t as_read_register(uint8_t bAddress)
{
     uint8_t bResult;
     uint16_t timeout;

     AS_SPI_REN &= ~AS_SDI_PIN;                   // Pulldown on SDI pin not required
     AS_CSN_OUT &= ~AS_CSN_PIN;                   // Select acceleration sensor

     bResult = AS_RX_BUFFER;                      // Read RX buffer just to clear
     // interrupt flag

     AS_TX_BUFFER = bAddress;                     // Write address to TX buffer

     timeout = AS_SPI_TIMEOUT;
     while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0)); // Wait until new data was written into
     // RX buffer
     if (timeout == 0)
     {
	  return (0);
     }
     bResult = AS_RX_BUFFER;                      // Read RX buffer just to clear
     // interrupt flag

     AS_TX_BUFFER = 0;                            // Write dummy data to TX buffer

     timeout = AS_SPI_TIMEOUT;
     while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0)); // Wait until new data was written into
     // RX buffer
     if (timeout == 0)
     {
	  return (0);
     }
     bResult = AS_RX_BUFFER;                      // Read RX buffer

     AS_CSN_OUT |= AS_CSN_PIN;                    // Deselect acceleration sensor
     AS_SPI_REN |= AS_SDI_PIN;                    // Pulldown on SDI pin required again

     // Return new data from RX buffer
     return bResult;
}

// *************************************************************************************************
// @fn          as_write_register
// @brief               Write a byte to the acceleration sensor
// @param       uint8_t bAddress                     Register address
//                              uint8_t bData                        Data to write
// @return      uint8_t 0 or bResult         Register content.
//                                                                      If the returned value is 0,
// there was an error.
// *************************************************************************************************
uint8_t as_write_register(uint8_t bAddress, uint8_t bData)
{
     uint8_t bResult;
     uint16_t timeout;

     AS_SPI_REN &= ~AS_SDI_PIN;                   // Pulldown on SDI pin not required
     AS_CSN_OUT &= ~AS_CSN_PIN;                   // Select acceleration sensor

     bResult = AS_RX_BUFFER;                      // Read RX buffer just to clear
     // interrupt flag

     AS_TX_BUFFER = bAddress;                     // Write address to TX buffer

     timeout = AS_SPI_TIMEOUT;
     while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0)); // Wait until new data was written into
     // RX buffer
    
     bResult = AS_RX_BUFFER;                      // Read RX buffer just to clear
     // interrupt flag

     AS_TX_BUFFER = bData;                        // Write data to TX buffer
    

     timeout = AS_SPI_TIMEOUT;
     while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0)); // Wait until new data was written into
     // RX buffer

     bResult = AS_RX_BUFFER;                      // Read RX buffer

     AS_CSN_OUT |= AS_CSN_PIN;                    // Deselect acceleration sensor
     AS_SPI_REN |= AS_SDI_PIN;                    // Pulldown on SDI pin required again

     return bResult;
}
