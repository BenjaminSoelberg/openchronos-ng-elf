#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Fri Dec 16 16:40:30 2016

@author: menehune
"""

import hashlib
import hmac
import time
import base64
import sys
import struct
import binascii

class totp:
    
    def __init__(self,base32secret):
        self.base32secret = base32secret
        self.decodedsecret = base64.b32decode(self.base32secret,True)
        self.decodedsecretlen = len(self.decodedsecret)
        self.hash_object = hmac.new(self.decodedsecret, None, hashlib.sha1)
        return None
        
    def __repr(self):
        return 'totp({})'.format(self.base32secret)

    def __str__(self):
        return 'totp Secret: {} hash: {} len: {}'.format(self.base32secret, 
                binascii.hexlify(self.decodedsecret), self.decodedsecretlen)

    @staticmethod
    def makebytearray(argint, length, total_len):
        bytelist = []
        for i in range(length):
            bytelist.append(argint & 0xff)
            argint = argint >> 8
        if total_len - length > 0:
            for i in range(0,total_len - length):
                bytelist.append(0)
        if sys.byteorder == 'little':
            bytelist.reverse()
        return bytearray(bytelist)

    @classmethod
    def getepochbytes(cls):
        curtime, remaintime = divmod(time.time(),30)
        return cls.makebytearray(int(curtime), 4, 8), 30-int(remaintime) 
    
    def genotp(self):
        epochbytes, remaintime = self.getepochbytes()
        self.hash_object.update(epochbytes)
        digest = self.hash_object.digest()
        offset = digest[self.hash_object.digest_size - 1] 
        if isinstance(offset, str):
            offset = ord(offset)
        offset = offset & 0xf
        otp = digest[offset:offset + 4]
        otp = struct.unpack('>I', otp)[0]
        otp = otp % 1000000
        return otp, remaintime

if __name__ == '__main__':
    if len(sys.argv) == 2:
        base32secret = sys.argv[1]
    else:
        base32secret = 'ATNIUMOTRUMZPTUZTNZN3O4U7MH67IB7'
        print('provide base32 secret as argv[1], using {}'.format(base32secret))
    totpinst = totp(base32secret)
    print('{:06d} Remaining {} seconds'.format(*totpinst.genotp()))
    print(totpinst)
