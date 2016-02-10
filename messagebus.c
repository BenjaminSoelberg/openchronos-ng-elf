/*
    openchronos.c: openchronos-ng main loop & user interface

	 Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

	          http://www.openchronos-ng.sourceforge.net

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

#include "messagebus.h"

/* the message bus */
static struct sys_messagebus *messagebus;

/***************************************************************************
 ************************* THE SYSTEM MESSAGE BUS **************************
 **************************************************************************/
void sys_messagebus_register(void (*callback)(enum sys_message),
                             enum sys_message listens)
{
	struct sys_messagebus **p = &messagebus;

	while (*p) {
		/* Set p to address of next pointer */
		p = &(*p)->next;
	}

	*p = malloc(sizeof(struct sys_messagebus));
	(*p)->next = NULL;
	(*p)->fn = callback;
	(*p)->listens = listens;
}

void sys_messagebus_unregister_all(void (*callback)(enum sys_message))
{
        sys_messagebus_unregister(callback, 0);
}

void sys_messagebus_unregister(void (*callback)(enum sys_message),
                               enum sys_message listens)
{
	struct sys_messagebus *p = messagebus, *pp = NULL;

	while (p) {
		if (p->fn == callback && (listens == 0 || p->listens == listens)) {
			if (!pp) { // If 1. element
				// Remove first element by pointing to the next
				messagebus = p->next;
				// Free element
				free(p);
				// Set current pointer to point to new first element
				p = messagebus;
				// Keep pp the same (NULL)
			} else { // If 2. or later element
				// Remove element by pointing previous to the next
				pp->next = p->next;
				// Free element
				free(p);
				// Set current pointer to point to next element
				p = pp->next;
				// Keep pp the same
			}
		} else {
			// Set pp (previous pointer) to current element
			pp = p;
			// Set p (current pointer) to next element
			p = p->next;
		}
	}
}
void send_events(enum sys_message msg)
{
	struct sys_messagebus *p = messagebus;

	while (p) {
		/* notify listener if he registered for any of these messages */
		if (msg & p->listens) {
			p->fn(msg);
		}

		/* move to next */
		p = p->next;
	}
}

