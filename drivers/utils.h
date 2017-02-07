/**
    util.h: openchronos-ng messaging system

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

/*!
    \file utils.h
    \brief openchronos-ng misc utilities
    \details Misc utilities
*/

#include "openchronos.h"

#ifndef __UTILS_H__
#define __UTILS_H__
// *************************************************************************************************
// Define section
#define st(x)                       do { x } while (__LINE__ == -1)
#define ENTER_CRITICAL_SECTION(x)   st( x = __get_SR_register(); __disable_interrupt(); )
#define EXIT_CRITICAL_SECTION(x)    __set_interrupt_state(x)
#define BIT_IS_SET(F, B) ((F) | (B)) == (F)
#define REBOOT()                    PMMCTL0 = PMMPW | PMMSWBOR; // generate BOR

#endif /* __UTILS_H__ */

