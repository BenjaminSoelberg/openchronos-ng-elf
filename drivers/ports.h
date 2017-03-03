/**
    drivers/ports.c: Openchronos ports driver

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2013 Martin AusChemnitz <MartinAusChemnitz@gmx.net>
    Copyright (C) 2016-2017 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

/*
    Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/


      Redistribution and use in source and binary forms, with or without
      modification, are permitted provided that the following conditions
      are met:

        Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the
        distribution.

        Neither the name of Texas Instruments Incorporated nor the names of
        its contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __PORTS_H__
#define __PORTS_H__

#include <stdbool.h>
#include "config.h"

/* Button ports */
#define PORTS_BTN_DOWN_PIN      (BIT0)
#define PORTS_BTN_NUM_PIN       (BIT1)
#define PORTS_BTN_STAR_PIN      (BIT2)
#define PORTS_BTN_BL_PIN        (BIT3)
#define PORTS_BTN_UP_PIN        (BIT4)

enum ports_buttons {
#ifdef CONFIG_BUTTONS_SWAP_UP_AND_DOWN
    PORTS_BTN_DOWN      = PORTS_BTN_UP_PIN,
    PORTS_BTN_UP        = PORTS_BTN_DOWN_PIN,
#else
    PORTS_BTN_DOWN      = PORTS_BTN_DOWN_PIN,
    PORTS_BTN_UP        = PORTS_BTN_UP_PIN,
#endif
    PORTS_BTN_NUM       = PORTS_BTN_NUM_PIN,
    PORTS_BTN_STAR      = PORTS_BTN_STAR_PIN,
    PORTS_BTN_BL        = PORTS_BTN_BL_PIN,
    PORTS_BTN_LDOWN     = BIT5,
    PORTS_BTN_LNUM      = BIT6,
    PORTS_BTN_LSTAR     = BIT7,
    PORTS_BTN_LBL       = BIT8,
    PORTS_BTN_LUP       = BIT9,
};

/* Global keypress peek, should normally NOT use be used, unless a global hook is needed */
uint8_t ports_button_pressed_peek(uint8_t btn, uint8_t with_longpress);
bool is_ports_button_pressed();

/* Below functions are exclusive for openchronos.c & menu.c, do NOT use them directly */
uint8_t ports_button_pressed(uint8_t btn, uint8_t with_longpress);
void ports_buttons_clear(void);
void ports_buttons_poll(void);
void init_buttons(void);

#endif /* __PORTS_H__ */
