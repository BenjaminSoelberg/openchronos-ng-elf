/**
    drivers/timer.h: Openchronos TA0 timer driver

    Copyright (C) 2012 Aljaž Srebrnič <a2piratesoft@gmail.com>

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
 * \file buzzer.h
 * \brief Buzzer subsystem functions.
 * \details This file contains all the methods used for \
 * playing tones with buzzer.
 * The buzzer can play a number of different tones, represented as a
 * note array.
 * The buzzer output frequency can be calculated using:
 * \f[ \frac{T_{\mathrm{ACLK}}}{2*\mathrm{TA1CCR0}} \f]
 */
#ifndef BUZZER_H_
#define BUZZER_H_

#include "openchronos.h"

bool is_buzzer_playing();

/*!
 * \brief Note type.
 * \details This is a type representing a note. It is composed by:
 * - The first 4 MSB represent the pitch
 * - The next 2 bits represent the octave
 * - The following 10 bits are the duration in ms of the note.
 *
 * There are two "meta" notes:
 * - The note xxx0 represents no tone (a rest).
 * - The note xxxF represents the "stop note" marking the \
 *   end of a note sequence.
 *
 * \note The stop note is needed in the play loop to determine \
 * when to end the melody.
 */
typedef uint16_t note;

/*!
 * \brief Initialize buzzer subsystem.
 */
void buzzer_init(void);

/*!
 * \brief Play a sequence of notes using the buzzer.
 * \param notes An array of notes to play.
 */
void buzzer_play(note *notes);
extern note welcome[4];
#endif /*BUZZER_H_*/
