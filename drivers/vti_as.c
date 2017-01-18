/**
    vti_as.c: accelerometer interface

    Copyright (C) 2012 Paolo Di Prodi <paolo@robomotic.com>

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

/*******************************************************************************
//
//  Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions
//    are met:
//
//      Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the
//      distribution.
//
//      Neither the name of Texas Instruments Incorporated nor the names of
//      its contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ****************************************************************************/
/* VTI CMA3000-D0x acceleration sensor driver functions */
/******************************************************************************/

/******************************************************************************/
/* Include section */

/* system */
#include "openchronos.h"
#include "vti_as.h"
#include "timer.h"

#ifndef CONFIG_MOD_ACCELEROMETER
void as_disconnect(void)
{
    AS_PWR_OUT &= ~AS_PWR_PIN; /* Power off */
    AS_INT_OUT &= ~AS_INT_PIN;/* Pin to low to avoid floating pins */
    AS_SPI_OUT &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN);/* Pin to low to avoid floating pins */
    AS_CSN_OUT &= ~AS_CSN_PIN;/* Pin to low to avoid floating pins */
    AS_INT_DIR |= AS_INT_PIN;/* Pin to output to avoid floating pins */
    AS_SPI_DIR |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN;/* Pin to output to avoid floating pins */
    AS_CSN_DIR |= AS_CSN_PIN;/* Pin to output to avoid floating pins */
    AS_PWR_DIR |= AS_PWR_PIN;/* Power pin to output direction */
}
#else


/******************************************************************************/
/* Prototypes section */
void as_start(uint8_t mode);
void as_stop(void);
uint8_t as_read_register(uint8_t bAddress);
uint8_t as_write_register(uint8_t bAddress, uint8_t bData);
uint8_t as_get_x(void);
uint8_t as_get_y(void);
uint8_t as_get_z(void);
uint8_t as_get_status(void);

void write_MDTHR(uint8_t msec);
void write_FFTMR(uint8_t multiplier);
void write_MDTMR(uint8_t multiplier);


/******************************************************************************/
/* Defines section */

/******************************************************************************/
/* CMA3000-D0x acceleration sensor configuration */
/******************************************************************************/
/* DCO frequency division factor determining speed of the acceleration sensor SPI interface */
/* Speed in Hz = 12MHz / AS_BR_DIVIDER (max. 500kHz) */
#define AS_BR_DIVIDER        (30u)

/* Acceleration measurement range in g */
/* Valid ranges are: 2 and 8 */
/* define AS_RANGE             (2u) */

/* Sample rate for acceleration values in Hz */
/* Valid sample rates for 2g range are:     100, 400 */
/* Valid sample rates for 8g range are: 40, 100, 400 */
/* define AS_SAMPLE_RATE       (400u) */

/******************************************************************************/
/* Global Variable section */
/* olatile as_status_register_flags as_status; */

/* Global flag for proper acceleration sensor operation */
uint8_t as_ok;


/******************************************************************************/
/* Extern section */
struct As_Param as_config;

volatile as_status_register_flags as_status;

/******************************************************************************/
/* @fn          as_init */
/* @brief       Setup acceleration sensor connection, do not power up yet */
/* @param       none */
/* @return      none */
/******************************************************************************/
void as_init(void)
{
#ifdef AS_DISCONNECT
    /* Deactivate connection to acceleration sensor */
    AS_PWR_OUT &= ~AS_PWR_PIN; /* Power off */
    AS_INT_OUT &= ~AS_INT_PIN; /* Pin to low to avoid floating pins */
    AS_SPI_OUT &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN); /* Pin to low to avoid floating pins */
    AS_CSN_OUT &= ~AS_CSN_PIN; /* Pin to low to avoid floating pins */
    AS_INT_DIR |= AS_INT_PIN; /* Pin to output to avoid floating pins */
    AS_SPI_DIR |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN; /* Pin to output to avoid floating pins */
    AS_CSN_DIR |= AS_CSN_PIN; /* Pin to output to avoid floating pins */
    AS_PWR_DIR |= AS_PWR_PIN; /* Power pin to output direction */
#else
    AS_INT_DIR &= ~AS_INT_PIN; /* Input */
    AS_SPI_DIR &= ~AS_SDI_PIN;/* Input */
    AS_SPI_DIR |= AS_SDO_PIN + AS_SCK_PIN;/* Output */
    AS_SPI_SEL |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN;/* Port pins to SDO, SDI and SCK function */
    AS_CSN_OUT |= AS_CSN_PIN;/* CSN=1 */
    AS_CSN_DIR |= AS_CSN_PIN;
    AS_PWR_OUT |= AS_PWR_PIN;/* VDD=1 */
    AS_PWR_DIR |= AS_PWR_PIN;
#endif

    /* Reset global sensor flag */
    as_ok = 1;
    /* Init configuration parameters to measurment mode */
    as_config.range = 8;
    as_config.sampling = SAMPLING_100_HZ;
}


/******************************************************************************/
/* @fn          change_mode */
/* @brief       This is only called for a "warm" (as_start was already called) mode change */
/* @param       mode can be [FALL_MODE, MEASUREMENT_MODE,ACTIVITY_MODE] */
/* @return      none */
/******************************************************************************/

void change_mode(uint8_t mode)
{
    uint8_t bConfig = 0x00;

/* Configure sensor and start to sample data */
    switch (mode) {
    case FALL_MODE:
        if (as_config.range == 2) {
            bConfig = 0x80;

            if (as_config.sampling == SAMPLING_100_HZ)
                bConfig |= 0x0A;
            else if (as_config.sampling == SAMPLING_400_HZ)
                bConfig |= 0x0C;
        } else if (as_config.range == 8) {
            bConfig = 0x00;

            if (as_config.sampling == SAMPLING_100_HZ)
                bConfig |= 0x0A;
            else if (as_config.sampling == SAMPLING_400_HZ)
                bConfig |= 0x0C;
        }

        /* fall time as long as possible 150 msec at 100 Hz */
        write_FFTMR(as_config.MDFFTMR);
        /* threshold for computation */
        write_FFTHR(as_config.FFTHR);

        break;

    case MEASUREMENT_MODE:

        /* Configure sensor and start to sample data */
        if (as_config.range == 2) {
            bConfig = 0x80;

            if (as_config.sampling == SAMPLING_100_HZ)
                bConfig |= 0x02;
            else if (as_config.sampling == SAMPLING_400_HZ)
                bConfig |= 0x04;
        } else if (as_config.range == 8) {
            bConfig = 0x00;

            if (as_config.sampling == SAMPLING_40_HZ)
                bConfig |= 0x06;
            else if (as_config.sampling == SAMPLING_100_HZ)
                bConfig |= 0x02;
            else if (as_config.sampling == SAMPLING_400_HZ)
                bConfig |= 0x04;
        }

        break;

    case ACTIVITY_MODE:

        /* Configure sensor and start to sample data */
        if (as_config.range == 2) {
            bConfig = 0x80;

            if (as_config.sampling == SAMPLING_10_HZ)
                bConfig |= 0x08;
        } else if (as_config.range == 8) {
            bConfig = 0x00;

            if (as_config.sampling == SAMPLING_10_HZ)
                bConfig |= 0x08;
        }

        bConfig |= MDET_EXIT << 5;
        /* fall time as long as possible 150 msec at 100 Hz */
        write_MDTMR(as_config.MDFFTMR);
        /* check if lower than 571 mgrav */
        /* write_MDTHR(sAccel.MDTHR); */
        write_MDTHR(as_config.MDTHR);
        break;

    default:
        bConfig = 0x80;
        break;

    }

    /* Wait 2 ms before entering modality to settle down */
    timer0_delay(2, LPM3_bits);

    /* write the configuration */
    as_write_register(ADDR_CTRL, bConfig);

    /* Wait 2 ms before entering modality to settle down */
    timer0_delay(2, LPM3_bits);

}
/******************************************************************************/
/* @fn          as_start */
/* @brief       Power-up and initialize acceleration sensor in measurment mode */
/* @param       mode can be [FALL_MODE, MEASUREMENT_MODE,ACTIVITY_MODE] */
/* @return      none */
/******************************************************************************/
void as_start(uint8_t mode)
{

    /* Initialize SPI interface to acceleration sensor */
    AS_SPI_CTL0 |= UCSYNC | UCMST | UCMSB /* SPI master, 8 data bits,  MSB first, */
               | UCCKPH;  /* clock idle low, data output on falling edge */
    AS_SPI_CTL1 |= UCSSEL1; /* SMCLK as clock source */
    AS_SPI_BR0 = AS_BR_DIVIDER; /* Low byte of division factor for baud rate */
    AS_SPI_BR1 = 0x00; /* High byte of division factor for baud rate */
    AS_SPI_CTL1 &= ~UCSWRST; /* Start SPI hardware */

    /* Initialize interrupt pin for data read out from acceleration sensor */
    AS_INT_IES &= ~AS_INT_PIN; /* Interrupt on rising edge */

#ifdef AS_DISCONNECT
    /* Enable interrupt */
    AS_INT_DIR &= ~AS_INT_PIN; /* Switch INT pin to input */
    AS_SPI_DIR &= ~AS_SDI_PIN; /* Switch SDI pin to input */
    AS_SPI_REN |= AS_SDI_PIN; /* Pulldown on SDI pin */
    AS_SPI_SEL |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN; /* Port pins to SDO, SDI and SCK function */
    AS_CSN_OUT |= AS_CSN_PIN; /* Deselect acceleration sensor */
    AS_PWR_OUT |= AS_PWR_PIN; /* Power on active high */
#endif

    /* Delay of >5ms required between switching on power and configuring sensor */
    timer0_delay(10, LPM3_bits);

    /* Initialize interrupt pin for data read out from acceleration sensor */
    AS_INT_IFG &= ~AS_INT_PIN; /* Reset flag */
    AS_INT_IE |= AS_INT_PIN; /* Enable interrupt */


    /* Reset sensor */
    as_write_register(0x04, 0x02);
    as_write_register(0x04, 0x0A);
    as_write_register(0x04, 0x04);

    /* Wait 5 ms before starting sensor output */
    timer0_delay(5, LPM3_bits);

    /* then select modality */
    change_mode(mode);

}

/******************************************************************************/
/* @fn          as_stop */
/* @brief       Power down acceleration sensor */
/* @param       none */
/* @return      none */
/******************************************************************************/
void as_stop(void)
{
    /* Disable interrupt */
    AS_INT_IE &= ~AS_INT_PIN; /* Disable interrupt */

#ifdef AS_DISCONNECT
    /* Power-down sensor */
    AS_PWR_OUT &= ~AS_PWR_PIN; /* Power off */
    AS_INT_OUT &= ~AS_INT_PIN; /* Pin to low to avoid floating pins */
    AS_SPI_OUT &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN); /* Pins to low to avoid floating pins */
    AS_SPI_SEL &= ~(AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN); /* Port pins to I/O function */
    AS_CSN_OUT &= ~AS_CSN_PIN; /* Pin to low to avoid floating pins */
    AS_INT_DIR |= AS_INT_PIN; /* Pin to output to avoid floating pins */
    AS_SPI_DIR |= AS_SDO_PIN + AS_SDI_PIN + AS_SCK_PIN; /* Pins to output to avoid floating pins */
    AS_CSN_DIR |= AS_CSN_PIN; /* Pin to output to avoid floating pins */
#else
    /* Reset sensor -> sensor to powerdown */
    as_write_register(0x04, 0x02);
    as_write_register(0x04, 0x0A);
    as_write_register(0x04, 0x04);
#endif
}

/******************************************************************************/
/* @fn          as_read_register */
/* @brief       Read a byte from the acceleration sensor */
/* @param       uint8_t bAddres Register address */
/* @return      uint8_t Register content */
/******************************************************************************/
uint8_t as_read_register(uint8_t bAddress)
{
    uint8_t bResult;
    uint16_t timeout;

    /* Exit function if an error was detected previously */
    if (!as_ok)
        return (0);

    bAddress <<= 2; /* Address to be shifted left by 2 and RW bit to be reset */

    AS_SPI_REN &= ~AS_SDI_PIN; /* Pulldown on SDI pin not required */
    AS_CSN_OUT &= ~AS_CSN_PIN; /* Select acceleration sensor */

    bResult = AS_RX_BUFFER; /* Read RX buffer just to clear interrupt flag */

    AS_TX_BUFFER = bAddress; /* Write address to TX buffer */

    timeout = SPI_TIMEOUT;

    while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0))
        ; /* Wait until new data was written into RX buffer */

    if (timeout == 0) {
        as_ok = 0;
        return (0);
    }

    bResult = AS_RX_BUFFER; /* Read RX buffer just to clear interrupt flag */

    AS_TX_BUFFER = 0; /* Write dummy data to TX buffer */

    timeout = SPI_TIMEOUT;

    while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0))
        ; /* Wait until new data was written into RX buffer */

    if (timeout == 0) {
        as_ok = 0;
        return (0);
    }

    bResult = AS_RX_BUFFER; /* Read RX buffer */

    AS_CSN_OUT |= AS_CSN_PIN; /* Deselect acceleration sensor */
    AS_SPI_REN |= AS_SDI_PIN; /* Pulldown on SDI pin required again */

    /* Return new data from RX buffer */
    return bResult;
}

/******************************************************************************/
/* @fn          as_write_register */
/* @brief   Write a byte to the acceleration sensor */
/* @param       uint8_t bAddress    Register address */
/*      uint8_t bData       Data to write */
/* @return      uint8_t */
/******************************************************************************/
uint8_t as_write_register(uint8_t bAddress, uint8_t bData)
{
    uint8_t bResult;
    uint16_t timeout;

    /* Exit function if an error was detected previously */
    if (!as_ok)
        return (0);

    bAddress <<= 2; /* Address to be shifted left by 1 */
    bAddress |= BIT1; /* RW bit to be set */

    AS_SPI_REN &= ~AS_SDI_PIN; /* Pulldown on SDI pin not required */
    AS_CSN_OUT &= ~AS_CSN_PIN; /* Select acceleration sensor */

    bResult = AS_RX_BUFFER; /* Read RX buffer just to clear interrupt flag */

    AS_TX_BUFFER = bAddress; /* Write address to TX buffer */

    timeout = SPI_TIMEOUT;

    while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0))
        ; /* Wait until new data was written into RX buffer */

    if (timeout == 0) {
        as_ok = 0;
        return (0);
    }

    bResult = AS_RX_BUFFER; /* Read RX buffer just to clear interrupt flag */

    AS_TX_BUFFER = bData; /* Write data to TX buffer */

    timeout = SPI_TIMEOUT;

    while (!(AS_IRQ_REG & AS_RX_IFG) && (--timeout > 0))
        ; /* Wait until new data was written into RX buffer */

    if (timeout == 0) {
        as_ok = 0;
        return (0);
    }

    bResult = AS_RX_BUFFER; /* Read RX buffer */

    AS_CSN_OUT |= AS_CSN_PIN; /* Deselect acceleration sensor */
    AS_SPI_REN |= AS_SDI_PIN; /* Pulldown on SDI pin required again */

    return bResult;
}

/******************************************************************************/
/* @fn          as_get_data */
/* @brief       Service routine to read acceleration values. */
/* @param       none */
/* @return      none */
/******************************************************************************/
void as_get_data(uint8_t *data)
{
    /* Exit if sensor is not powered up */
    if ((AS_PWR_OUT & AS_PWR_PIN) != AS_PWR_PIN)
        return;

    /* Store X/Y/Z acceleration data in buffer */
    *(data + 0) = as_read_register(0x06);
    *(data + 1) = as_read_register(0x07);
    *(data + 2) = as_read_register(0x08);
}

uint8_t as_get_x(void)
{
    if ((AS_PWR_OUT & AS_PWR_PIN) != AS_PWR_PIN)
        return 0;

    return as_read_register(0x06);
}

uint8_t as_get_y(void)
{
    if ((AS_PWR_OUT & AS_PWR_PIN) != AS_PWR_PIN)
        return 0;

    return as_read_register(0x07);
}

uint8_t as_get_z(void)
{
    if ((AS_PWR_OUT & AS_PWR_PIN) != AS_PWR_PIN)
        return 0;

    return as_read_register(0x08);
}



uint8_t as_get_status()
{
    volatile uint8_t status;

    if ((AS_PWR_OUT & AS_PWR_PIN) != AS_PWR_PIN)
        return 0;

    status = as_read_register(ADDR_INT_STATUS);
    return status;

}

/* Set the MDTHR motion detection threshold value register */
/* Default value is 8h=1000 that amounts to 143 mg at 400 Hz and 2g range */
void write_MDTHR(uint8_t multiplier)
{
    uint8_t bData = 0x00;

    if (as_config.range == 2) {
        bData = multiplier << 2;
        /* for safety only takes B5-B1 */
        /* this is also the max allowed value */
        bData &= (0x3C);

    } else if (as_config.range == 8) {
        bData = multiplier;
        /* for safety only takes B5-B1 */
        bData &= (0x7F);
        /* max range is 0x3C */
    }

    /* TODO force it to 0x39 for some tests */
    as_write_register(ADDR_MDTHR, bData);

}

/* Set the free fall threshold for the accelerometer values */
/* Range=2g -> [B5 B4 B3 B2] 143 71 36 1/56=18 mg */
/* Range=8g -> [B3 B2 B1 B0] 571 286143 1/14 = 71 mg */

void write_FFTHR(uint8_t multiplier)
{
    /* bData = 1100 will set the center bits for both cases */
    uint8_t bData = 0x0C;

    if (as_config.range == 2) {
        bData = multiplier << 2;


    } else if (as_config.range == 8) {
        bData = multiplier;

    }

    /* Take only the 6 LSB: 111111 */
    bData &= (0x3F);
    as_write_register(ADDR_FFTHR, bData);

}

/* Set the motion detection timer bits B7-B4 */
/* B6= 400 msec */
/* B5= 200 msec */
/* B4= 100 msec */
void write_MDTMR(uint8_t multiplier)
{
    /* 0x50=400+100 msec=500 msec */
    /* mask the B6:B4 bits */
    uint8_t bData = (multiplier << 4) & 0x70;
    as_write_register(ADDR_MDFFTMR, bData);
}

/* Set the FFTMR timer bits */
/* Default value is 3h=11 that: */
/* 400 Hz -> 7.5 msec */
/* 100 Hz -> 30  msec */
void write_FFTMR(uint8_t multiplier)
{
    uint8_t bData = multiplier;
    /* The address of the register name MDFFTMR is 0Ah */
    /* Bits FFTMR [3:0] RW Initial Value 3h */
    /* FFTMRLSB[sec]=1/ODR[HZ] */
    /* ODR is the current output data rate defined by Mode bits */
    /* First conver the seconds in the register value with the bit weighting scheme */
    /* in manual page 15 of cm3000-dx */

    /*
    if (sAccel.sampling == 100) { / * in this sampling mode: 1/100 s=10 msec * /
                           / * min: 10 msec -> b0 max: 80+40+20+10=150 msec b2 b1 b0 * /
        bData=multiplier;
    } else if (sAccel.sampling == 400) {
        / * in this sampling mode: 1/400 s=2.5 msec * /
        / * min: 10 msec -> b0 max: 20+10+5+2.5=37.5 msec b2 b1 b0 * /
        bData=multiplier;
    } else return;
    */
    /* Take only the 4 LSB */
    bData &= (0x0F);
    as_write_register(ADDR_MDFFTMR, bData);

}

#endif
