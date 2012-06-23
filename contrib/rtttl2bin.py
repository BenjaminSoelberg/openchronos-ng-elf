#!/usr/bin/env python3.2
#
# Copyright (C) 2012 Aljaž Srebrnič <a2piratesoft@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import re

notes_translate = ('p', 'a', 'a#', 'b', 'c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#')


def parse_note(note, default_duration, default_octave):
    """
        This parses a note into a tuple of (duration, pitch, octave)
    """
    note = re.match('(^[0-9]{0,2})([a-gp]#?)([4-9]?)(\.?)', note.strip(), flags=re.IGNORECASE).groups()
    duration = note[0] == '' and default_duration or int(note[0])
    duration *= (note[3] == '' and [1] or [1.5])[0]
    return (duration, note[1].lower(), note[2] == '' and default_octave or int(note[2]))


def parse_ringtone(ringtone):
    """
        This method parses a RTTTL ringtone (as a string) to a dictionary in this form:
        title: the title of the melody
        melody: a list of tuple notes, as parsed by parse_note
        whole: The duration in ms of the "whole" note
    """
    match = re.match('([a-zA-Z]+):\s*d=([0-9]{1,3}),o=([0-9]{1,3}),b=([0-9]{1,3}):(.*)', ringtone)
    default_duration = int(match.group(2))
    default_octave = int(match.group(3))
    bpm = int(match.group(4))
    whole = (60 * 1000 / bpm) * 4
    notes = [parse_note(note, default_duration, default_octave) for note in match.group(5).split(',')]
    return {"title": match.group(1), "melody": notes, "whole": whole}


def generate_binary_ringtone(ringtone):
    """
        This method converts a ringtone as parsed by parse_ringtone in the binary representation,
        converting each note using generate_binary_note()
    """
    melody_real = list()
    melody = ringtone["melody"]
    for note in melody:
        melody_real.append(hex(generate_binary_note(note, ringtone["whole"])))
    melody_real.append("0x000F")
    return "note %s[%d] = {%s};" % (ringtone["title"], len(ringtone["melody"]) + 1, ', '.join(melody_real))


def generate_binary_note(note, whole_note):
    """
        This method converts a tuple note as parsed by parse_note to the binary representation.
    """
    duration = whole_note / note[0]
    if duration > 1023:
        raise Exception("note duration too long")
    tone = notes_translate.index(note[1])
    octave = note[2] - 4
    return (duration << 6) | (octave << 2) | tone
    
    
if __name__ == '__main__':
    import sys
    print(generate_binary_ringtone(parse_ringtone(sys.argv[1])))
