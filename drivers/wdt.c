/**
    drivers/wdt.c: Watchdog timer functions

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
#include "wdt.h"

inline void wdt_setup() {
#ifdef USE_WATCHDOG
    WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
#else
    wdt_stop();
#endif
}

/* Stop watchdog timer */
inline void wdt_stop() {
    WDTCTL = WDTPW + WDTHOLD;
}

/* service watchdog on wakeup */
inline void wdt_poll() {
#ifdef USE_WATCHDOG
    WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
#endif
}
