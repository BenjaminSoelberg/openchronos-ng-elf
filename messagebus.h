/**
    messagebus.h: openchronos-ng messaging system

    Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>
    Copyright (C) 2016 Benjamin Sølberg <benjamin.soelberg@gmail.com>

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
    \file messagebus.h
    \brief Messagebus include file
*/

#ifndef __MESSAGEBUS_H__
#define __MESSAGEBUS_H__

#include <msp430.h>

#include <stdlib.h>

#include <stdint.h>

/*!
    \brief List of possible message types for the message bus.
    \sa sys_messagebus_register()
*/
/* WARNING: The enum values are optimized to work with some drivers.
            If you need to add a new entry, append it to the end! */
enum sys_message {
    /* drivers/rtca */
    SYS_MSG_NONE       = 0,    /*!< Empty event. */
    SYS_MSG_RTC_ALARM  = BIT0, /*!< alarm event from the hardware RTC. */
    SYS_MSG_RTC_SECOND = BIT1, /*!< second event from the hardware RTC. */
    SYS_MSG_RTC_MINUTE = BIT2, /*!< minute event from the hardware RTC. */
    SYS_MSG_RTC_HOUR   = BIT3, /*!< hour event from the hardware RTC. */
    SYS_MSG_RTC_DAY    = BIT4, /*!< day event from the hardware RTC. */
    SYS_MSG_RTC_MONTH  = BIT5, /*!< month event from the hardware RTC. */
    SYS_MSG_RTC_YEAR   = BIT6, /*!< year event from the hardware RTC. */
    /* drivers/timer */
    SYS_MSG_TIMER_4S   = BIT7, /*!< 4s (period) event from the hardware TIMER_0. */
    SYS_MSG_TIMER_20HZ = BIT8, /*!< 20HZ event from the hardware TIMER_0. */
    SYS_MSG_TIMER_PROG = BIT9, /*!< programmable event from TIMER_0. */
    /* sensor/interrups */
    SYS_MSG_AS_INT     = BITA,
    SYS_MSG_PS_INT     = BITB,
    SYS_MSG_BATT       = BITC,
    SYS_MSG_BUTTON     = BITD,
};

/*!
    \brief Linked list of nodes listening to the message bus.
*/
struct sys_messagebus {
    /*! callback for receiving messages from the system bus */
    void (*fn)(enum sys_message);
    /*! bitfield of message types that the node wishes to receive */
    enum sys_message listens;
    /*! pointer to the next node in the list */
    struct sys_messagebus *next;
};

/*!
    \brief Registers a node in the message bus.
    \details Registers (add) a node to the message bus. A node can filter what message(s) are to be received by setting the bitfield \b listens.
    \sa sys_message, sys_messagebus, sys_messagebus_unregister
*/
void sys_messagebus_register(
    /*! callback to receive messages from the message bus */
    void (*callback)(enum sys_message),
    /*! only receive messages of this type */
    enum sys_message listens
);

/*!
    \brief Unregisters a node from the message bus.
    \sa sys_messagebus_register
*/
void sys_messagebus_unregister_all(
    /*! the same callback used on sys_messagebus_register() */
    void (*callback)(enum sys_message)
);

/*!
    \brief Unregisters a node from the message bus.
    \sa sys_messagebus_register
*/
void sys_messagebus_unregister(
    /*! the same callback used on sys_messagebus_register() */
    void (*callback)(enum sys_message),
    /*! the same message used on sys_messagebus_register() */
    enum sys_message listens
);

/*!
    \brief Send a message to all listening nodes on the message bus.
    \sa sys_messagebus_register, sys_messagebus_unregister
*/
void send_events(
    enum sys_message msg
);

#endif /* __MESSAGEBUS_H__ */
