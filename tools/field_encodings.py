#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: ts=4 noexpandtab

import base64
import time


def fix_b32padding(string):  # Fix any missing padding (like a 26 char long DropBox secrets)
    missing_padding = len(string) % 8
    if missing_padding != 0:
        string += '=' * (8 - missing_padding)
    return string


def b32encode_each(string, decode):
    if decode:
        key = base64.b32decode(fix_b32padding(string.upper().replace(" ", "")))
        return '"' + "".join(map(lambda x: "\\x%02x" % ord(x), list(key))) + '"', len(key)
    else:
        s = "".join(map(lambda x: chr(int("0x" + x, 16)), string.replace('"', '').split("\\x")[1:]))
        return base64.b32encode(s), None


def b32encode(strings, decode):
    strings.strip()
    start_block = '{ '
    end_block = ' }'
    removechars = start_block + end_block + ' '
    if decode:
        encoded_auth_secrets = strings.split(',')
        decoded_auth_list = []
        for encoded_auth_count, secret in enumerate(encoded_auth_secrets):
            colon_split_secret = secret.split(':')

            if len(colon_split_secret) == 1:
                #user did not provide an identifier, use index as an identifier
                identifier = chr(encoded_auth_count + ord('A'))
                secret = colon_split_secret[0]
            else:
                identifier = colon_split_secret[0].upper()
                identifier.strip()
                secret = colon_split_secret[1]

            decoded_secret, decoded_secret_len = b32encode_each(secret, True)
            decoded_auth_list.append(start_block + '"' + identifier + '"')
            decoded_auth_list.append(decoded_secret)
            decoded_auth_list.append(str(decoded_secret_len) + end_block)

        return start_block + ",".join(decoded_auth_list) + end_block

    else:
        for c in removechars:
            strings = strings.replace(c, '')
        strings.strip()
        encoded_auth_list = []
        decoded_auth_secrets = strings.split(',')
        for identifier, decoded_secret, secret_len in zip(*[iter(decoded_auth_secrets)] * 3):
            identifier = identifier.replace('"', '')
            encoded_secret, _ = b32encode_each(decoded_secret, False)
            if not identifier[0].isdigit():
                encoded_secret = identifier + ':' + encoded_secret
            encoded_auth_list.append(encoded_secret)
        return ",".join(encoded_auth_list)


def tzget(cfg_offset, tz_set):
    offset = 0
    if cfg_offset == '':
        offset = -time.timezone / 3600
    else:
        offset = cfg_offset
    return offset
