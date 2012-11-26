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


import unittest
import rtttl2bin

class RTTTLTests(unittest.TestCase):
	def test_generate_binary_note(self):
		"""Testing if generate_binary_note generates binary notes correctly"""
		self.assertEqual(rtttl2bin.generate_binary_note((8, 'e',  4), 2664), 21320)

	def test_generate_binary_ringtone(self):
		self.assertEqual(rtttl2bin.generate_binary_ringtone(
			{'title': 'welcome', 'melody': [(16, 'a', 4), (16, 'c', 4), (16, 'e', 4)], 'whole': 1600}),
			"note welcome[4] = {0x1901, 0x1904, 0x1908, 0x000F};")

	def test_parse_note(self):
		"""Testing if parse_note parses notes correctly"""
		self.assertEqual(rtttl2bin.parse_note("1e4",   32, 4), (1, 'e',  4))
		self.assertEqual(rtttl2bin.parse_note("32d9",  32, 4), (32, 'd', 9))
		self.assertEqual(rtttl2bin.parse_note("2c#6",  32, 4), (2, 'c#', 6))
		self.assertEqual(rtttl2bin.parse_note("2d#9.", 32, 4), (3, 'd#', 9))
		self.assertEqual(rtttl2bin.parse_note("2A#6.", 32, 4), (3, 'a#', 6))

	def test_parse_note_strip(self):
		"""Testing if parse_note strips whitespace"""
		self.assertEqual(rtttl2bin.parse_note(" 2d#.", 32, 4), (3, 'd#', 4))
		self.assertEqual(rtttl2bin.parse_note("2d#. ", 32, 4), (3, 'd#', 4))
		self.assertEqual(rtttl2bin.parse_note(" 2d#. ", 32, 4), (3, 'd#', 4))
		self.assertEqual(rtttl2bin.parse_note("  2d#. ", 32, 4), (3, 'd#', 4))
		self.assertEqual(rtttl2bin.parse_note("2d#.  ", 32, 4), (3, 'd#', 4))

	def test_parse_note_defaults(self):
		"""Testing if parse_note uses supplied defaults"""
		self.assertEqual(rtttl2bin.parse_note("d#6",   32, 4), (32, 'd#', 6))
		self.assertEqual(rtttl2bin.parse_note("d#8.",  32, 4), (48, 'd#', 8))
		self.assertEqual(rtttl2bin.parse_note("2d#.",  32, 4), (3, 'd#', 4))

	def test_parse_ringtone(self):
		"""Test if parse_tone parses multiple notes separated by commas"""
		self.assertEqual(rtttl2bin.parse_ringtone("Test: d=8,o=4,b=105: 2d#6., 2d8"),
			{'title': 'Test', 'melody': [(3, 'd#', 6), (2, 'd', 8)], "whole": 2284})
		self.assertEqual(rtttl2bin.parse_ringtone("Test: d=8,o=4,b=160: d#6., 2d8"),
			{'title': 'Test', 'melody': [(12, 'd#', 6), (2, 'd', 8)], "whole": 1500})
		self.assertEqual(rtttl2bin.parse_ringtone("Test: d=8,o=4,b=90: 2d#6., 2d"),
			{'title': 'Test', 'melody': [(3, 'd#', 6), (2, 'd', 4)], "whole": 2664})

	def test_ringtone_generation(self):
		"""This tests the complete generation sequence"""
		self.assertEqual(rtttl2bin.generate_binary_ringtone(rtttl2bin.parse_ringtone(
			"welcome: d=16,o=4,b=150: a, c, e")),
			"note welcome[4] = {0x1901, 0x1904, 0x1908, 0x000F};")

if __name__ == '__main__':
	unittest.main()
