/*
    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __EZCHRONOS_H__
#define __EZCHRONOS_H__

#include <project.h>

typedef void (*menu_item_fn_t)(uint8_t line);
typedef void (*menu_disp_fn_t)(uint8_t line, uint8_t mode);

void menu_add_entry(uint8_t const line,
		menu_item_fn_t const sx_fun,
		menu_item_fn_t const mx_fun,
		menu_disp_fn_t const disp_fun);

void menu_L1_skip_next(void);
void menu_L2_skip_next(void);

#endif /* __EZCHRONOS_H__ */
