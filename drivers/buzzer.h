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

#endif /*BUZZER_H_*/
