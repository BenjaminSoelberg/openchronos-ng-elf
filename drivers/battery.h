/**
    battery.h: battery voltage measurement driver

    Copyright (C) 2012 Matthew Excell <matt@excellclan.com>
    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

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

/***************************************************************************

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

****************************************************************************/

#ifndef __BATTERY_H__
#define __BATTERY_H__

#include "openchronos.h"

void battery_init(void);
void battery_measurement(void);

/* Battery high voltage threshold */
#define BATTERY_HIGH_THRESHOLD          (360u)

/* Battery end of life voltage threshold -> disable radio,
   show "lobatt" message */
#define BATTERY_LOW_THRESHOLD           (240u)

/* Where we consider the battery full - A guess, but a pretty safe one at this point*/
#define BATTERY_FULL_THRESHOLD          (310u)

/* Where we consider the battery empty */
#define BATTERY_EMPTY_THRESHOLD         (220u)

struct {
    /* Battery voltage */
    uint16_t voltage;

    /* Battery voltage offset */
    int16_t offset;
} battery_info;

#endif /* __BATTERY_H__ */
