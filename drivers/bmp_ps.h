// *************************************************************************************************
//
//      Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//        Redistribution and use in source and binary forms, with or without
//        modification, are permitted provided that the following conditions
//        are met:
//
//          Redistributions of source code must retain the above copyright
//          notice, this list of conditions and the following disclaimer.
//
//          Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the
//          distribution.
//
//          Neither the name of Texas Instruments Incorporated nor the names of
//          its contributors may be used to endorse or promote products derived
//          from this software without specific prior written permission.
//
//        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//        "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//        LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//        A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//        OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//        SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//        LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//        DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//        THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//        OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************

#ifndef BMP_PS_H_
#define BMP_PS_H_

// *************************************************************************************************
// Include section

// *************************************************************************************************
// Prototypes section
extern void bmp_ps_init(void);
extern void bmp_ps_get_cal_param(void);
extern void bmp_ps_start(void);
extern void bmp_ps_stop(void);
extern uint16_t bmp_ps_read_register(uint8_t address, uint8_t mode);
extern uint8_t bmp_ps_write_register(uint8_t address, uint8_t data);
extern uint32_t bmp_ps_get_pa(void);
extern uint16_t bmp_ps_get_temp(void);

// *************************************************************************************************
// Defines section

// Bosch BMP085 defines

#define BMP_085_CHIP_ID	     (0x55)
#define BMP_085_I2C_ADDR     (0xEE >> 1)

#define BMP_085_PROM_START_ADDR (0xAA)
#define BMP_085_CHIP_ID_REG	 (0xD0)
#define BMP_085_VERSION_REG	 (0xD1)

#define BMP_085_CTRL_MEAS_REG (0xF4)
#define BMP_085_ADC_OUT_MSB_REG	(0xF6)
#define BMP_085_ADC_OUT_LSB_REG	(0xF7)

#define BMP_085_SOFT_RESET_REG (0xE0)

#define BMP_085_T_MEASURE    (0x2E)				 // temperature measurent
#define BMP_085_P_MEASURE    (0x34)				 // pressure measurement

#define BMP_085_TEMP_CONVERSION_TIME (5)		 // TO be spec'd by GL or SB

#define BMP_SMD500_PARAM_MG  (3038)              //calibration parameter
#define BMP_SMD500_PARAM_MH  (-7357)             //calibration parameter
#define BMP_SMD500_PARAM_MI  (3791)              //calibration parameter
#define BMP_SMD500_PARAM_MJ  (64385)             //calibration parameter

/** this structure holds all device specific calibration parameters 
 */
typedef struct {
     short ac1;
     short ac2;
     short ac3;
     unsigned short ac4;
     unsigned short ac5;
     unsigned short ac6;
     short b1;
     short b2;
     short mb;
     short mc;
     short md;      		   
} bmp_085_calibration_param_t;

// *************************************************************************************************
// Global Variable section


// *************************************************************************************************
// Extern section

#endif                          /*BMP_PS_H_ */
