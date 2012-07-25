#!/usr/bin/env python2
# encoding: utf-8
# vim: ts=4:

import urwid
import urwid.raw_display
import sys


import re, sys, random
from sorteddict import SortedDict

import modules

DATA = SortedDict()

# GENERAL CONFIG ############################################################

DATA["TEXT_GENERAL"] = {
	"name": "General options",
	"type": "info",
}

DATA["CONFIG_DEBUG"] = {
	"name": "Build debug code",
	"default": False,
	"help": "Sets CFLAGS and LDFLAGS for debugging",
}

DATA["CONFIG_FIXEDPOINT_MATH"] = {
	"name": "Fixedpoint Math (EXPERIMENTAL)",
	"depends": [],
	"default": False,
	"help": "Tries to use fix point aritmetric. If no module is using it, it reduces the code size dramaticly.",
}

DATA["USE_LCD_CHARGE_PUMP"] = {
	"name": "Use LCD Charge Pump (6 bytes)",
	"default": False,
	"help": "Use the internal charge pump to make the display contrast contstant through the whole battery lifetime. As a downside this increases currency and reduces battery lifetime.",
}

DATA["USE_WATCHDOG"] = {
	"name": "Use Watchdog (20 bytes)",
	"default": True,
	"help": "Protects the clock against deadlocks by rebooting it.",
}

# RTC DRIVER #################################################################

DATA["TEXT_RTC"] = {
	"name": "RTC driver",
	"type": "info",
}

DATA["CONFIG_RTC_IRQ"] = {
	"name": "Enable RTC interrupts",
	"default": True,
	"help": "Enables interrupts on the Real Time Clock",
}

# TIMER0 DRIVER ##############################################################

DATA["TEXT_TIMER"] = {
	"name": "Timer0 driver",
	"type": "info",
}

DATA["CONFIG_TIMER_4S_IRQ"] = {
	"name": "Enable 1Hz timer interrupts",
	"default": True,
	"help": "Enables 1Hz interrupts on the hardware timer",
}

DATA["CONFIG_TIMER_20HZ_IRQ"] = {
	"name": "Enable 20Hz timer interrupts",
	"default": True,
	"help": "Enables 20Hz interrupts on the hardware timer",
}

# PORTS DRIVER ###############################################################

DATA["TEXT_PORTS"] = {
	"name": "Ports driver",
	"type": "info",
}

DATA["CONFIG_BUTTONS_LONG_PRESS_TIME"] = {
	"name": "Button short press time",
	"type": "text",
	"default": "6",
	"ifndef": True,
	"help": "Long button press time (in multiples of 1/20 second)",
}

DATA["CONFIG_BUTTONS_SHORT_PRESS_TIME"] = {
	"name": "Button short press time",
	"type": "text",
	"default": "1",
	"ifndef": True,
	"help": "Short button press time (in multiples of 1/20 second)",
}

# BATTERY DRIVER #############################################################

DATA["TEXT_BATTERY"] = {
	"name": "Battery driver",
	"type": "info",
}

DATA["CONFIG_BATTERY_MONITOR"] = {
	"name": "Background Battery Monitor",
	"default": False,
	"help": "Monitors the battery voltage every minute and displays a warning on low battery. Also used by the battery to auto-refresh display, if enabled.",
}

DATA["CONFIG_BATTERY_DISABLE_FILTER"] = {
	"name": "Disables battery filter [FOR TESTING]",
	"default": True,
	"help": "Reports the straight voltage value from measurement instead of the 'smoothed' one.",
}

# TEMPERATURE SENSOR DRIVER ##################################################

DATA["TEXT_TEMPERATURE"] = {
	"name": "Temperature sensor driver",
	"type": "info"
}

DATA["CONFIG_TEMPERATURE_METRIC_ONLY"] = {
	"name": "Only show in degrees C",
	"default": False,
	"help": "Don't build code to convert to F.",
}

# AUTOMATICALLY GENERATED MODULE LIST ########################################

DATA["TEXT_MODULES"] = {
	"name": "Modules",
	"type": "info"
}

for key,field in modules.read_config():
	DATA[key] = field


HEADER = """
#ifndef _CONFIG_H_
#define _CONFIG_H_

"""

FOOTER = """
#endif // _CONFIG_H_
"""

class HelpListWalker(urwid.SimpleListWalker):
	def __init__(self, app, *args, **kwargs):
		self.app = app
		super(HelpListWalker, self).__init__(*args, **kwargs)


	def set_focus(self, focus):
		if hasattr(self[focus], "_datafield") and \
											"help" in self[focus]._datafield:
			self.app.help_widget.set_text(self[focus]._datafield["help"])
		else:
			self.app.help_widget.set_text("")
		return super(HelpListWalker, self).set_focus(focus)

class HelpGridFlow(urwid.GridFlow):
	def __init__(self, app, *args, **kwargs):
		self.app = app
		super(HelpGridFlow, self).__init__(*args, **kwargs)


	def set_focus(self, focus):
		if hasattr(focus, "_datafield") and \
												"help" in focus._datafield:
			self.app.help_widget.set_text(focus._datafield["help"])
		else:
			self.app.help_widget.set_text("")
		return super(HelpGridFlow, self).set_focus(focus)


class OpenChronosApp(object):
	def main(self):
		self.fields = {}
		text_header = (u"OpenChronos config  \u2503  "
					   u"UP / DOWN / PAGE UP / PAGE DOWN scroll.  F8 aborts.")

		self.list_content = list_content = []

		for key,field in DATA.iteritems():
			# generate gui forms depending on type
			self.generate_widget(key,field)

		def ok_pressed(*args, **kwargs):
			raise urwid.ExitMainLoop()

		def abort_pressed(*args, **kwargs):
			sys.exit(0)

		list_content.append(urwid.Divider(div_char=u"\u2550", top=1, bottom=1))
		list_content.append(
		urwid.Padding(urwid.GridFlow(
			[urwid.AttrWrap(urwid.Button("Save", ok_pressed), 'opt','optsel'),
			 urwid.AttrWrap(urwid.Button("Abort", abort_pressed), 'opt','optsel')],
			15, 4, 4, 'center'),
			('fixed left',4), ('fixed right',3)))


		header = urwid.AttrWrap(urwid.Text(text_header), 'header')
		#header = urwid.Padding(urwid.BigText("OpenChronos", urwid.HalfBlock5x4Font()))
		walker = HelpListWalker(self, list_content)
		listbox = urwid.ListBox(walker)
		self.help_widget = urwid.Text("")
		footer = urwid.AttrWrap(self.help_widget, 'footer')
		frame = urwid.Frame(urwid.AttrWrap(listbox, 'body'), header=header, footer=footer)

		screen = urwid.raw_display.Screen()
		palette = [
			('header',	'white',	'dark red', 	'bold'),
			('info',	'black',	'dark cyan'),
			('body',	'black',	'dark gray'),
			('optsel',	'white',	'dark blue'),
			('opt',		'black',	'light gray'),
			]


		def unhandled(key):
			if key == 'f8':
				#raise urwid.ExitMainLoop()
				sys.exit(0)

		urwid.MainLoop(frame, palette, screen,
			unhandled_input=unhandled).run()

	def generate_widget(self, key, field):
		if field.get("type", "bool") == "bool":
			f = urwid.AttrWrap(urwid.CheckBox(field["name"],
				state=field["value"]),'opt','optsel')
			if field.has_key('ischild') and field['ischild']:
				f = urwid.Padding(f, width=77, left=3)
			f._datafield = field
			self.fields[key] = f
			self.list_content.append(f)

		elif field["type"] == "choices":
			try:
				value = field["values"].index(field["value"])
			except ValueError:
				value = field["default"]
			field["radio_button_group"] = []
			f = urwid.Text(field["name"])
			f._datafield = field
			choice_items = [f]
			for dat in field["values"]:
				txt = value = dat
				if isinstance(dat, tuple):
					value, txt = dat
				f = urwid.AttrWrap(urwid.RadioButton(
					field["radio_button_group"],
					unicode(txt), state=value==field["value"]),
					'opt','optsel')
				f._datafield = field
				f.value = value
				choice_items.append(f)
			hgf = HelpGridFlow(self, choice_items, 20, 3, 1, 'left')
			self.fields[key] = choice_items
			hgf.focus_cell = hgf.cells[1]
			self.list_content.append(hgf)

		elif field["type"] == "text":
			f = urwid.AttrWrap(urwid.Edit("%s: "%field["name"],
					str(field["value"])), 
					'opt', 'optsel')
			f._datafield = field
			self.fields[key] = f
			self.list_content.append(f)

		elif field["type"] == "info":
			f = urwid.AttrWrap(urwid.Text(field["name"]),
					'info', 'info')
			f._datafield = field
			self.fields[key] = f
			self.list_content.append(f)

	def get_config(self):
		return DATA

	def save_config(self):
		for key,field in self.fields.iteritems():
			while isinstance(field, urwid.AttrMap) \
			or isinstance(field, urwid.Padding):
				field = field.original_widget

			if isinstance(field, (tuple, list)):
				for item in field:
					if hasattr(item, "get_state"):
						if item.get_state():
							# found the set radio button
							DATA[key]["value"] = item.value
			elif isinstance(field, urwid.Edit):
				DATA[key]["value"] = field.get_edit_text()
			elif isinstance(field, urwid.CheckBox):
				DATA[key]["value"] = field.get_state()

		fp = open("config.h", "w")
		fp.write("// !!!! DO NOT EDIT !!!, use: make config\n")
		fp.write(HEADER)
		for key,dat in DATA.iteritems():
			if not "value" in dat:
				continue
			if "type" in dat and dat["type"] == "info":
				continue
			if DATA[key].get("ifndef", False):
				fp.write("#ifndef %s\n" %key)
			if isinstance(dat["value"], bool):
				if dat["value"]:
					fp.write("#define %s\n" %key)
				else:
					fp.write("// %s is not set\n" %key)
			else:
				fp.write("#define %s %s\n" %(key, dat["value"]))
			if DATA[key].get("ifndef", False):
				fp.write("#endif // %s\n" %key)
		fp.write(FOOTER)


	def load_config(self):
		def set_default():
			for key,dat in DATA.iteritems():
				#print dat
				if not "value" in dat and "default" in dat:
					dat["value"] = dat["default"]

		try:
			fp = open("config.h")
		except (OSError, IOError):
			set_default()
			return
		match = re.compile('^[\t ]*#[\t ]*define[\t ]+([a-zA-Z0-9_]+)[\t ]*(.*)$')
		match2 = re.compile('^// ([a-zA-Z0-9_]+) is not set$')
		for line in fp:
			m = match.search(line)
			if m:
				m = m.groups()
				if not m[0] in DATA:
					continue
				if m[1] == "":
					DATA[m[0]]["value"] = True
				else:
					try:
						value = int(m[1])
					except ValueError:
						value = m[1]
					DATA[m[0]]["value"] = value
			else:
				m = match2.search(line)
				if m and m.groups()[0] in DATA:
					m = m.groups()
					DATA[m[0]]["value"] = False

		set_default()

if __name__ == "__main__":
	App = OpenChronosApp()
	App.load_config()
	App.main()
	App.save_config()

