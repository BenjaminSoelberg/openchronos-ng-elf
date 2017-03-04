/**
    drivers/lpm.h: Low power mode driver

    Copyright (C) 2017 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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

#include "lpm.h"
#include "buzzer.h"

void enter_lpm_gie(uint16_t LPM_bits) {
    if (is_buzzer_playing()) {
        // Override LPM bits to LPM0 because we need SMCLK for tone generation as well as FLL for stability
        LPM_bits = LPM0_bits;
    }

    /* Go to LPMx & wait for interrupts */
    _BIS_SR(LPM_bits | GIE);
    __no_operation();
}
