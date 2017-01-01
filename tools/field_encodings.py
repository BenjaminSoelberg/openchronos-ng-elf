#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: ts=4 noexpandtab

import base64
import time

def b32encode_each(string, decode):
	if decode:
		key = base64.b32decode(string.upper().replace(" ",""))
		return  '"' + "".join(map(lambda x:"\\x%02x" % ord(x), list(key))) + '"'
	else:
		s =  "".join(map (lambda x: chr(int("0x" + x, 16)), string.replace('"', '').split("\\x")[1:]))
		return  base64.b32encode(s)

def b32encode(strings, decode):
        strings.strip()
        start_block = '{ '
        end_block= ' }'
        removechars = start_block + end_block + ' '
        if decode:
            encoded_auth_secrets = strings.split(',')
            encoded_auth_list = []
            for encoded_auth_count, secret in enumerate(encoded_auth_secrets):
                colon_split_secret = secret.split(':')

                if len(colon_split_secret) == 1:
                #user did not provide an identifier, use index as an identifier
                    identifier = str(encoded_auth_count) 
                    secret = colon_split_secret[0]
                else:
                    identifier = colon_split_secret[0].upper()
                    identifier.strip()
                    secret = colon_split_secret[1]
                encoded_auth_list.append( start_block + '"' + identifier + '"')
                encoded_auth_list.append(b32encode_each(secret, True)+ end_block) 
            return start_block + ",".join(encoded_auth_list) + end_block

        else:
            for c in removechars:
                strings = strings.replace(c,'')
            strings.strip()
            encoded_auth_list = []
            decoded_auth_secrets = strings.split(',')
            for  identifier, decoded_secret in zip(*[iter(decoded_auth_secrets)]*2):
                identifier = identifier.replace('"', '')
                encoded_secret = b32encode_each(decoded_secret, False)
                if not identifier[0].isdigit():
                    encoded_secret = identifier + ':' + encoded_secret
                encoded_auth_list.append(encoded_secret)
            return ",".join(encoded_auth_list)

def tzget (cfg_offset, tz_set):
        offset = 0
        if cfg_offset == '': 
            offset = -time.timezone/3600
        else: 
            offset = cfg_offset
        return offset   
