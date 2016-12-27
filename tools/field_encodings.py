#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: ts=4 noexpandtab

import base64
import time

def b32encode(string, encode):
	if encode:
		key = base64.b32decode(string.upper().replace(" ",""))
		return  '"' + "".join(map(lambda x:"\\x%02x" % ord(x), list(key))) + '"'
	else:
		s =  "".join(map (lambda x: chr(int("0x" + x, 16)), string.replace('"', '').split("\\x")[1:]))
		return  base64.b32encode(s)

def tzget (cfg_offset, tz_set):
        offset = 0
        if cfg_offset == '': 
            offset = -time.timezone/3600
        else: 
            offset = cfg_offset
        return offset   
