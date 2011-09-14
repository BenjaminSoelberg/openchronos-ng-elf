#!/usr/bin/env python
###################################################################################################
# ChronosTool.py
# Tool for programming TI eZ430-Chronos watches
# (C) 2011, Christiane Ruetten, cr@23bit.net
#
# Released under GPLv2, see http://www.gnu.org/licenses/gpl-2.0.html
#
# The included ram_based_updater.txt binary blob rfbsl code is
# (C) 2010 Texas Instruments Inc., released under the following license:
#
# *************************************************************************************************
#
#       Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
#
#
#         Redistribution and use in source and binary forms, with or without
#         modification, are permitted provided that the following conditions
#         are met:
#
#           Redistributions of source code must retain the above copyright
#           notice, this list of conditions and the following disclaimer.
#
#           Redistributions in binary form must reproduce the above copyright
#           notice, this list of conditions and the following disclaimer in the
#           documentation and/or other materials provided with the
#           distribution.
#
#           Neither the name of Texas Instruments Incorporated nor the names of
#           its contributors may be used to endorse or promote products derived
#           from this software without specific prior written permission.
#
#         THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#         "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#         LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#         A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#         OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#         SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#         LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#         DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#         THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#         (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#         OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# *************************************************************************************************
#
###################################################################################################
version = "0.3"
# Changelog:
#   0.1 - public preview version
#   0.2 - import rewritten
#       - sync time accuracy fix
#   0.3 - verbosity option
#       - noreset option
#       - acceleration data streaming
###################################################################################################

import sys
import os
import serial
import time
import datetime

###################################################################################################
class CBMcmd():
    "Class for handling Chronos Base Module commands"

    def __init__( self, opcode, payload=[] ):
        self.opcode = opcode
        self.payload = bytearray( payload )
        self.len = len( self.payload ) + 3

    def opcode( self, opcode ):
        self.opcode = opcode

    @staticmethod
    def maxlen():
        return 28

    def len( self ):
        return self.len

    def payload( self, payload ):
        self.payload = bytearray( payload )
        self.len = len( self.payload ) + 3

    def tobytes( self ):
        return bytearray( [0xff, self.opcode, self.len]  ) + bytearray( self.payload )

    def tostr( self ):
        return str( self.tobytes() )

    def tohex( self ):
        return self.tostr().encode('hex')

    def getitem( self ):
        return self.tobytes()

###################################################################################################
class CBMpayload:
    "Class for handling Chronos Base Module command payloads"

    def __init__( self, data ):
        self.data = bytearray( data )

    @staticmethod
    def maxlen():
        return CBMcmd.maxlen()

    def tocmd( self, opcode ):
        return CBMcmd( opcode, self.data )

###################################################################################################
class CBMburst:
    "Class for handling Chronos Base Module bursts"

    max_burst_len = 0xf7

    def __init__( self, type, data ):
        self.type = type
        self.data = bytearray( data )
        self.len = len( self.data ) + 2

    @classmethod
    def maxlen( cls ):
        return cls.max_burst_len

    @classmethod
    def setmaxlen( cls, len ):
        cls.max_burst_len = len
        if opt.verbose:
            print >> sys.stderr, "Maximum CBM burst length set to", hex( len )

    def topayloads( self ):
        #Reshape each burst into payloads
        max_payload_len = CBMpayload.maxlen()
        payloads = []
        burst = bytearray( [self.type, self.len-2] ) + self.data
        while burst:
            payload = CBMpayload( burst[:max_payload_len] )
            burst = burst[max_payload_len:]
            payloads.append( payload )
        return payloads

###################################################################################################
class CBMchunk:
    "Class for Chronos Base Module chunks"

    def __init__( self, address, data ):
        self.address = address
        self.data = bytearray( data )
        self.len = len( self.data ) + 2

    def tobursts( self ):
        #Reshape chunk into burst sequences
        max_burst_len = CBMburst.maxlen()
        bursts = []
        chunk = bytearray( [self.address>>8, self.address&0xff] ) + self.data
        nr = 0
        while chunk:
            if nr == 0:
                #First burst
                burst_id = 0x01
            else:
                #Consecutive bursts
                burst_id = 0x02
            nr += 1
            burst = CBMburst( burst_id, chunk[:max_burst_len] )
            chunk = chunk[max_burst_len:]
            bursts.append( burst )
        return bursts

###################################################################################################
class CBMdata:
    "Class for Chronos Base Module chunked data"

    def __init__( self, data=[] ):
        self.chunks = []
        self.data = bytearray( data )

    def importtxt( self, src ):
        if isinstance( src, str ):
            if os.path.exists( src ):
                file = open( src, "r" )
                input = file.read()
                file.close()
            else:
                input = src
        elif isinstance( src, list ):
            input = src
        elif isinstance( src, bytearray ):
            input = str( src )
        else:
            print >> sys.stderr, "ERROR: unable to handle argument to importtxt"
            sys.exit( 9 )

        # Flatten string by removing spaces and newlines
        input = input.replace( ' ', '' ).replace( '\n', '' )

        # Minimal sanity check
        if input[0] != '@' or input[-1] != 'q':
            print >> sys.stderr, "ERROR: malformed import data"
            sys.exit( 9 )

        # Remove first @ and final q and split into chunks
        chunks = input[1:-1].split( '@' )

        # Iterate chunks, append to instance
        for chunk in chunks:
            # First two bytes are address
            address = int( chunk[:4], 16 )
            data = bytearray( chunk[4:].decode( 'hex' ) )
            if opt.verbose:
                print >> sys.stderr, "Chunk at address @" + hex(address) + ", length", len(data)
            self.chunks.append( CBMchunk( address, data ) )

    def tochunks( self ):
        return self.chunks

###################################################################################################
class CBM:
    "Class for the Chronos Base Module"

    def __init__( self, device_name ):
        if opt.verbose:
            print >> sys.stderr, 'Using Chronos Base Module on', device_name
        #opt will be decomissioned by the time __del__ is called
        self.optverbose = opt.verbose
        self.device = serial.Serial( device_name, 115200, timeout = 1 )
        self.allstatus()
        if opt.reset:
            self.reset()
            self.allstatus()
        #Original Chronos tool reads twice
        response = self._wbsl_getmaxpayload()
        response = self._wbsl_getmaxpayload()
        CBMburst.setmaxlen( response.payload[0] )
        self.allstatus()

    def __del__( self ):
        if self.optverbose:
            print >> sys.stderr, 'Closing Chronos Base Module at', self.device.port
        #self.reset()
        self.device.close

    def send( self, cmd ):
        self.device.write( cmd.tostr() )
        time.sleep( 0.015 )
        if opt.verbose:
            print >> sys.stderr, 'SENT:', cmd.tohex()
        response = bytearray( self.device.read( 3 ) )
        if response[2] > 3:
            response += bytearray( self.device.read( response[2]-3 ) )
        self.response = CBMcmd( response[1], response[3:] )
        if opt.verbose:
            print >> sys.stderr, 'RECV:', self.response.tohex()
        return self.response

    def sendcmd( self, opcode, payload=[] ):
        cmd = CBMcmd( opcode, payload )
        return self.send( cmd )

    def _reset( self ):
        return self.sendcmd( 0x01 )     #BM_Reset

    def _getstatus( self ):
        return self.sendcmd( 0x00, [0x00] ) #BM_GetStatus

    def _br_stop( self ):
        return self.sendcmd( 0x06 )     #BM_BR_Stop

    def _spl_start( self ):
        return self.sendcmd( 0x07 )     #BM_SPL_Start

    def _spl_getdata( self ):
        return self.sendcmd( 0x08, [0x00, 0x00, 0x00, 0x00] )   #BM_GetData

    def _spl_stop( self ):
        return self.sendcmd( 0x09 )     #BM_SPL_Stop

    def _sync_start( self ):
        return self.sendcmd( 0x30 )     #BM_SYNC_Start

    def _sync_getbufferstatus( self ):
        return self.sendcmd( 0x32, [0x00] ) #BM_SYNC_GetBufferStatus

    def _sync_readbuffer( self ):
        return self.sendcmd( 0x33, [0x00] ) #BM_SYNC_ReadBuffer

    def _wbsl_start( self ):
        return self.sendcmd( 0x40 )     #BM_WBSL_Start

    def _wbsl_stop( self ):
        return self.sendcmd( 0x46 )     #BM_WBSL_Stop

    def _wbsl_getstatus( self ):
        return self.sendcmd( 0x41, [0x00] ) #BM_WBSL_GetStatus

    def _wbsl_getmaxpayload( self ):
        return self.sendcmd( 0x49, [0x00] ) #BM_WBSL_GetMaxPayload

    def _wbsl_getpacketstatus( self ):
        return self.sendcmd( 0x48, [0x00] ) #BM_WBSL_GetPacketStatus

    def reset( self ):
        self._reset()
        return self._getstatus()

    def spl_start( self ):
        return self._spl_start()

    def spl_getdata( self ):
        return self._spl_getdata().payload

    def _bit_value( self, val, bit_nr ):
        return ( val >> bit_nr ) & 1

    def _mgrav_accel( self, raw ):
        # Conversion values from data to mgrav taken
        # from CMA3000-D0x datasheet (rev 0.4, table 4)
        mgrav_per_bit = [ 18, 36, 71, 143, 286, 571, 1142 ]

        # fix signedness: uint8 to int8
        if raw > 128:
            sign = -1
            absraw = -raw
        else:
            sign = 1
            absraw = raw

        mgrav = 0
        for n in range( 7 ):
            mgrav += mgrav_per_bit[n] * self._bit_value( absraw, n )

        return sign * mgrav / 1000.0

    def spl_getaccel( self ):
        data = self._spl_getdata().payload
        ret = [ False, 0, 0, 0 ]
        if data[0] == 0x01:
            xval = data[1]
            yval = data[2]
            zval = data[3]
            if opt.raw:
                # just fix signedness
                if xval > 128: xval -= 256
                if yval > 128: yval -= 256
                if zval > 128: zval -= 256
            else:
                # mgrav conversion and g/2 Z axis shift
                xval = self._mgrav_accel( xval )
                yval = self._mgrav_accel( yval )
                zval = self._mgrav_accel( zval ) + 0.42
            ret = [ True, xval, yval, zval ]
        return ret

    def spl_stop( self ):
        self._spl_start()
        return self._spl_stop()

    def sync_start( self ):
        return self._sync_start().payload

    def sync_getbufferstatus( self ):
        return self._sync_getbufferstatus().payload

    def wbsl_start( self ):
        #self.spl_stop()
        #time.sleep( 0.5 )
        #self._br_stop()
        #time.sleep( 1.0 )
        ret = self._wbsl_start().payload
        time.sleep( 0.1 )
        return ret

    def wbsl_stop( self ):
        return self._wbsl_stop().payload

    def getstatus( self ):
        return self._getstatus().payload

    def wbsl_getstatus( self ):
        return self._wbsl_getstatus().payload

    def wbsl_getpacketstatus( self ):
        return self._wbsl_getpacketstatus().payload

    def allstatus( self ):
        return [self.getstatus(), self.wbsl_getstatus(), self.wbsl_getpacketstatus()]

    def sendburst( self, burst ):
        for payload in burst.topayloads():
            ret = self.send( payload.tocmd( 0x47 ) )
        return ret

    def sendburstheader( self, bursts ):
        #Construct initial info block
        nr_of_bursts = len( bursts )
        payload = bytearray( [0x00, nr_of_bursts&0xff, nr_of_bursts>>8] )
        return self.sendcmd( 0x47, payload )

    def spl_sync( self, dt=[], celsius=0, meters=0 ):
        self.spl_start()
        raw_input("Ready to sync. Set your watch in sync mode and press enter.")
        time.sleep(2)
        if not dt:
            dt = datetime.datetime.now()
        print "Syncing Time %s, Temp %s C, and altitude %s m)" % (str(dt), str(celsius), str(meters))

        payload = bytearray( 0x13 )
        payload[0x00] = 0x03
        payload[0x01] = dt.hour+0x80 #assume 24h
        payload[0x02] = dt.minute
        payload[0x03] = dt.second
        payload[0x04] = 0x07
        payload[0x05] = dt.year-0x700
        payload[0x06] = dt.month
        payload[0x07] = dt.day
        payload[0x08] = 0x06
        payload[0x09] = 0x1e
        payload[0x0a] = (celsius*10)>>8
        payload[0x0b] = (celsius*10)&0xff
        payload[0x0c] = meters>>8
        payload[0x0d] = meters&0xff

        self.sendcmd( 0x31, payload ) #BM_SYNC_SendCommand
        time.sleep( 2 )
        self.spl_stop()
        print "Synced!"

    def transmitburst( self, data ):
        self.wbsl_start()
        time.sleep( 0.5 )

        chunklist = data.tochunks()
        burstlist = []
        for chunk in chunklist:
            burstlist += chunk.tobursts()

        for burst in burstlist:
            done = 0
            while not done:
                status = self.wbsl_getpacketstatus()[0]
                if   status == 1:   #WBSL_DISABLED
                    time.sleep( 0.2 )
                elif status == 2:   #WBSL_PROCESSING
                    time.sleep( 0.1 )
                elif status == 4:   #WBSL_WAITFORSIZE
                    self.sendburstheader( burstlist )
                elif status == 8:   #WBSL_WAITFORDATA
                    if burstlist:
                        self.sendburst( burstlist[0] )
                        burstlist = burstlist[1:]
                    else:
                        if opt.verbose:
                            print >> sys.stderr, "WARNING: Burstlist underflow"
                        time.sleep(0.05)
                else:           #WBSL_COMPLETE
                    done = 1
                    break
        self.wbsl_stop()

    def wbsl_download( self, txtdata ):

        #Prepare data for downloading to watch
        updater = CBMdata()
        updater.importtxt(
"""@1D30
31 40 FE 2B 3C 40 88 29 3D 40 1C 01 B0 13 78 27
3C 40 52 29 3D 40 84 1E 3E 40 36 00 B0 13 18 28
B0 13 48 26 B0 13 20 28 20 20 00 20 20 20 20 20
20 00 30 30 00 F5 60 B6 F2 63 D3 D7 70 F7 F3 00
00 00 00 00 86 00 77 C7 95 E6 97 17 F3 67 05 F0
87 85 75 46 C6 37 F5 06 D3 87 C4 C4 02 67 E3 B6
10 08 08 08 F7 F7 F7 F7 20 40 04 80 7F 7F 7F 7F
7F 10 01 80 30 30 30 31 30 32 30 33 30 34 30 35
30 36 30 37 30 38 30 39 31 30 31 31 31 32 31 33
31 34 31 35 31 36 31 37 31 38 31 39 32 30 32 31
32 32 32 33 32 34 32 35 32 36 32 37 32 38 32 39
33 30 33 31 33 32 33 33 33 34 33 35 33 36 33 37
33 38 33 39 34 30 34 31 34 32 34 33 34 34 34 35
34 36 34 37 34 38 34 39 35 30 35 31 35 32 35 33
35 34 35 35 35 36 35 37 35 38 35 39 36 30 36 31
36 32 36 33 36 34 36 35 36 36 36 37 36 38 36 39
37 30 37 31 37 32 37 33 37 34 37 35 37 36 37 37
37 38 37 39 38 30 38 31 38 32 38 33 38 34 38 35
38 36 38 37 38 38 38 39 39 30 39 31 39 32 39 33
39 34 39 35 39 36 39 37 39 38 39 39 20 20 00 20
20 44 4F 4E 45 00 20 20 46 41 49 4C 00 20 52 46
42 53 4C 00 24 0A 24 0A 25 0A 26 0A 21 0A 22 0A
23 0A 25 0A 20 0A 20 0A 24 0A 2B 0A 2B 0A 2A 0A
29 0A 28 0A 27 0A 20 0A 24 0A 28 0A 07 00 AD 00
BA 5E BA 11 05 00 AD 00 00 00 1B 15 7A 40 03 00
4B 4A C2 93 A3 2A 76 20 B0 13 94 26 82 43 34 0F
B0 13 18 27 B0 13 D4 27 6D 43 5C 43 B0 13 CE 26
6D 43 6C 43 B0 13 CE 26 6D 43 B0 13 FC 27 5D 43
B0 13 12 28 D2 43 88 29 D2 43 9D 2A B0 13 BE 24
5C 93 3D 20 E2 43 9D 2A 4D 4A 5C 43 B0 13 CE 26
4D 4A 6C 43 B0 13 CE 26 4D 4A 4C 4A B0 13 CE 26
B0 13 CC 1F 92 92 94 2A 96 2A 03 28 C2 93 A3 2A
26 20 B0 13 B2 20 5C 93 17 20 F2 90 64 00 A0 2A
0A 2C 4F 43 6E 43 5C 42 A0 2A 0D 43 B0 13 76 24
0D 4C 5C 43 07 3C B0 13 F2 27 B0 13 10 28 3D 40
6F 1E 6C 43 B0 13 32 25 F2 90 20 00 9D 2A 07 24
92 92 94 2A 96 2A D6 2B C2 93 A3 2A DA 27 6D 42
5C 43 B0 13 CE 26 6D 42 6C 43 B0 13 CE 26 6D 42
B0 13 FC 27 B0 13 10 28 C2 43 88 29 92 92 94 2A
96 2A 0A 28 C2 93 A3 2A 07 24 B2 40 04 A5 20 01
7B 53 87 23 1A 17 10 01 4B 93 FA 23 B0 13 F2 27
3D 40 76 1E 6C 43 B0 13 32 25 F4 3F B0 13 A0 20
B2 40 00 A5 44 01 B0 13 A0 20 B2 40 06 A5 40 01
C2 43 E0 FF B0 13 A0 20 B2 40 10 A5 44 01 10 01
3B 15 82 93 38 0F B2 C0 00 02 32 0F E2 93 9E 2A
32 20 3E 40 02 01 0D 43 3C 40 90 29 B0 13 54 27
7A 40 3B 00 B0 13 78 20 4C 93 25 24 7C 40 3F 00
B0 13 F2 25 C2 4C 90 29 39 40 91 29 6C 53 C2 4C
9C 2A 7B 90 41 00 05 28 B0 13 02 27 B0 13 8A 27
12 3C E2 93 9C 2A 0B 28 B0 13 78 20 6B 93 F9 2B
B0 13 6A 20 19 53 F2 53 9C 2A 7B 53 F7 3F B0 13
6A 20 D2 43 9F 2A 38 17 10 01 7C 40 3F 00 B0 13
F2 25 C9 4C 00 00 10 01 4C 4A B0 13 F2 25 4B 4C
48 4B 4C 4A B0 13 F2 25 4B 4C 48 9C F9 23 10 01
B0 13 A0 20 8C 4D 00 00 B0 13 A0 20 5C 43 10 01
92 B3 44 01 FD 2F 10 01 B0 13 A0 20 CC 4D 00 00
F3 3F 1B 15 4A 43 D2 53 A2 2A F2 90 05 00 A2 2A
05 28 F2 40 20 00 9D 2A 4C 43 5F 3C B0 13 98 25
3B 40 96 2A D2 93 9F 2A 07 24 C2 93 A3 2A 04 24
2D 4B 3D 53 5C 43 4E 3C D2 93 9F 2A 4D 20 C2 43
9F 2A 5F 42 90 29 5F 4F 92 29 7F F0 80 00 D2 92
A1 2A 92 29 41 20 5E 42 93 29 C2 93 A3 2A 14 20
4F 93 10 24 C2 43 A2 2A 5F 42 94 29 47 18 0F 5F
0E 5F 82 4E 94 2A 3D 43 5C 43 B0 13 FC 24 D2 43
A3 2A 2A 3C 3D 43 25 3C 4F 93 22 24 5F 42 94 29
47 18 0E 5E 0F 5E 2F 9B CB 23 B0 13 9A 22 5C 93
17 20 C2 43 A2 2A 2D 4B 5C 43 B0 13 FC 24 92 53
96 2A 2C 4B 5C 06 0F 4C 5C 0A 0F 5C 0C 5C 0C 5F
1E 42 94 2A B0 13 B6 26 C2 4C A0 2A 5A 43 04 3C
2D 4B 4C 43 B0 13 FC 24 4C 4A 1A 17 10 01 1B 15
0B 4C 4A 4D B0 13 02 27 4E 4A 0D 4B 7C 40 7F 00
B0 13 68 25 6A 42 07 3C 3C 40 0F 00 B0 13 E8 26
B0 13 E8 27 7A 53 7C 40 34 00 B0 13 14 22 A2 B3
30 0F FD 2B 92 C3 32 0F 7C 40 35 00 B0 13 14 22
3C 40 19 00 B0 13 E8 26 B0 13 E8 27 92 B3 32 0F
0A 28 92 C3 32 0F 92 B3 30 0F FD 2B 7C 40 3B 00
B0 13 14 22 0D 3C 7C 40 36 00 B0 13 14 22 7C B0
70 00 F9 23 7C 40 3A 00 B0 13 14 22 4A 93 CC 23
1A 17 10 01 3B 15 4B 4C 4C 43 7B 90 BD 00 06 24
7B 90 31 00 38 28 7B 90 3D 00 35 2C 08 42 32 C2
03 43 B2 C0 40 00 02 0F B0 13 06 28 7B 90 31 00
25 28 7B 90 3D 00 22 2C B0 13 F2 25 4A 4C 7D 40
29 00 4C 43 B0 13 72 26 C2 4B 11 0F A2 B2 30 0F
10 28 7B 90 32 00 0D 24 7B 90 39 00 0A 24 7B 90
38 00 07 24 A2 B2 30 0F FD 2F 3F 40 C1 0C 3F 53
FE 2F 4D 4A 4C 43 B0 13 72 26 02 3C C2 4B 11 0F
5C 42 21 0F 02 48 38 17 10 01 2A 15 5C 43 5A 42
90 29 3A 50 FB FF 7D 40 06 00 D2 93 95 29 0B 20
5E 42 96 29 47 18 0E 5E 5F 42 97 29 0E 5F 82 4E
92 2A 2A 83 7D 42 B0 13 BA 27 0A 93 1A 24 1C 42
92 2A 3C 90 00 80 09 28 08 4C 09 43 08 5A 09 63
19 93 05 28 02 20 18 93 02 28 4C 43 0A 3C 4E 4D
3E 50 90 29 4D 4A B0 13 76 23 5C 93 02 20 82 5A
92 2A B2 40 00 A5 40 01 B2 40 10 A5 44 01 28 17
10 01 0A 12 6C 92 2D 28 7C 90 14 00 2A 2C 7D 90
30 00 07 28 7D 90 5B 00 09 2C 4D 4D 5A 4D 35 1D
06 3C 7D 90 2D 00 02 20 6A 43 01 3C 4A 43 7C 90
0B 00 11 28 4F 4A 43 19 4F 10 43 18 4A 5A 4A DF
7C 90 0B 00 08 20 7D 90 31 00 03 24 7D 90 4C 00
02 20 7A 40 80 00 4C 4C 4F 4E 5E 4C 90 1D B0 13
C8 27 3A 41 10 01 5B 15 08 4C 09 4E 5C 43 0A 48
0B 43 0E 48 0F 43 46 4D 0A 56 0B 63 02 3C 1E 53
0F 63 0F 9B 03 28 18 20 0E 9A 16 2C 0C 4E 58 B3
08 20 06 4A 07 4B 36 53 37 63 0E 96 06 20 0F 97
04 20 7D 49 B0 13 A8 20 05 3C 3D 49 B0 13 90 20
1E 53 0F 63 5C 93 E3 27 56 17 10 01 F2 D0 03 00
4A 02 B2 F0 3E FF 6C 01 B2 D0 0C 00 6C 01 82 43
66 01 B2 40 44 00 68 01 32 D0 40 00 82 43 60 01
B2 40 50 00 62 01 B2 40 6E 11 64 01 32 C0 40 00
3F 40 22 F4 03 43 0E 43 3F 53 3E 63 FD 2F B2 F0
F0 FF 6E 01 A2 C3 02 01 A2 B3 02 01 F8 2F 80 00
1E 26 7E E3 5F 93 05 20 6E FC 4E DD CC 4E 00 00
10 01 4F 93 03 20 CC FE 00 00 10 01 6F 93 09 20
CC FE 00 00 CC FE 20 00 CC DD 00 00 CC DD 20 00
10 01 7F 90 03 00 08 20 4F 4E 6F FC 4F DD CC 4F
00 00 CC FE 20 00 10 01 6F 92 04 20 CC FE 00 00
CC FE 20 00 10 01 3B 15 0A 4C 0B 4D 48 4E 39 40
98 2A 2E 43 3D 40 62 1D 0C 49 B0 13 66 27 48 93
13 24 78 90 03 00 10 2C 0B 93 0E 20 3A 90 64 00
0B 2C 4E 48 4D 48 3D E3 1D 53 0A 5A 0D 5A 3D 50
A6 1D 0C 49 B0 13 66 27 0C 49 38 17 10 01 0A 12
4A 43 7D 42 3C 40 7A 29 B0 13 8E 21 B0 13 98 25
D2 93 9F 2A 10 20 C2 43 9F 2A D2 93 93 29 05 20
5F 42 90 29 CF 93 92 29 02 38 4C 43 05 3C D2 42
92 29 A1 2A 5A 43 4C 4A 3A 41 10 01 4E 4C 3C 40
09 00 B0 13 E8 26 B0 13 E8 27 D2 42 A1 2A 83 29
C2 4E 85 29 0F 4D 47 19 0F 10 7F F0 7F 00 C2 4F
86 29 C2 4D 87 29 7D 40 06 00 3C 40 82 29 80 00
8E 21 3B 15 0A 4D 6C 93 05 20 7B 40 0B 00 78 40
06 00 03 3C 7B 40 06 00 68 43 49 43 09 3C 5E 43
0F 4A 0F 59 6D 4F 4C 4B 4C 59 B0 13 12 23 59 53
49 98 F5 2B 38 17 10 01 0A 12 0A 42 B0 13 40 27
6F 4D 0C 5F 82 4C 10 0F 5F 43 07 3C B0 13 DE 27
0C 4D 0C 5F E2 4C 10 0F 5F 53 4F 9E F7 2B C2 93
20 0F 02 4A 3A 41 10 01 B0 13 8A 27 3C 40 00 40
B0 13 E8 26 06 3C B2 B0 00 02 32 0F 02 28 B0 13
F0 1F 92 B3 44 03 03 2C D2 93 9F 2A F4 23 B0 13
9A 27 80 00 02 27 21 83 82 43 32 0F 7C 40 30 00
B0 13 14 22 81 43 00 00 02 3C 91 53 00 00 B1 90
64 00 00 00 FA 2B B0 13 AA 27 82 43 06 0F 21 53
10 01 0F 42 32 C2 03 43 B0 13 06 28 7C 90 2F 00
03 28 7C 90 3E 00 03 20 7C D0 80 00 02 3C 7C D0
C0 00 C2 4C 13 0F 5C 42 20 0F 02 4F 10 01 B2 D0
06 00 06 0A B2 40 1D 7B 00 0A B2 40 ED 00 04 0A
F2 D0 E0 00 4A 02 F2 D0 E0 00 44 02 B2 43 0A 0A
B2 40 FF 00 0C 0A 10 01 B2 40 80 5A 5C 01 32 C2
03 43 B0 13 CC 23 B0 13 D4 27 3C 40 33 73 B0 13
E8 26 B0 13 E8 27 B0 13 BA 1E B2 40 04 A5 20 01
F4 3F 21 83 0E 42 B0 13 40 27 4D 4D 0C 5D 82 4C
10 0F B0 13 DE 27 5F 42 20 0F 81 4F 00 00 02 4E
21 53 10 01 C2 43 88 29 C2 43 A0 2A 82 43 96 2A
82 43 94 2A C2 43 A2 2A C2 43 A3 2A 82 43 32 0F
10 01 7C F3 7E F3 0F 4E 0D 4C 0E 43 1C 43 0D 5D
0E 6E 0E 9F 01 28 0E 8F 0C 6C F9 2B 10 01 0A 12
7C 90 14 00 07 2C 4C 4C 5A 4C 90 1D 4F 4D 4E 4A
B0 13 C8 27 3A 41 10 01 3C 90 00 80 02 28 3C 40
FF 7F 82 4C 54 03 92 C3 44 03 B2 D0 24 01 40 03
10 01 D2 43 9E 2A B0 13 AA 27 7C 40 3A 00 B0 13
14 22 82 43 32 0F 10 01 3D 40 58 1D 5C 43 B0 13
32 25 3D 40 5B 1D 6C 43 80 00 32 25 0A 12 21 83
0A 4C 81 4A 00 00 0D 41 5C 43 B0 13 24 28 F9 3F
32 C2 03 43 B0 13 06 28 4C 4C 3C D0 40 00 47 18
0C 5C 10 01 0F 4C 04 3C CF 4D 00 00 1F 53 3E 53
0E 93 FA 23 10 01 0F 4C 04 3C FF 4D 00 00 1F 53
3E 53 0E 93 FA 23 10 01 0F 4C 0F 5D 03 3C CC 43
00 00 1C 53 0C 9F FB 23 10 01 82 43 32 0F E2 43
9E 2A 7C 40 34 00 80 00 14 22 92 C3 44 03 B2 F0
CF FF 40 03 82 43 50 03 10 01 7C 40 36 00 B0 13
14 22 7C B0 70 00 F9 23 10 01 B2 40 00 A5 44 01
B2 40 40 A5 40 01 10 01 4D 4A 0C 5C 1C 4C 52 29
80 00 22 24 3D 40 7D 1E 6C 43 80 00 32 25 B2 B0
20 00 02 0F FC 2B 10 01 92 B3 44 03 FD 2B 80 00
9A 27 3D 40 6C 1E 5C 43 80 00 32 25 4C 4A B0 13
CE 26 80 00 C6 25 B2 B0 10 00 02 0F FC 2B 10 01
4D 43 4C 43 80 00 CE 26 80 00 66 27 80 00 2C 27
80 00 1C 28 10 01
@FFFE
30 1D
q""" )
        data = CBMdata()
        print "Reading firmware file"
        data.importtxt( txtdata )

        raw_input("Hit enter to start update process. (or Ctrl+C to exit)")

        print "Ready to update. Set your watch in  rfbsl \"open\" mode."
        self.transmitburst( updater )
        print "Sending new firmware.."
        self.transmitburst( data )
        time.sleep( 1 )
        print "Done!"

###################################################################################################
# main

from optparse import OptionParser

usage = """
       %prog [options] rfbsl|sync|prg|accel [<arguments> ...]

       %prog [options] rfbsl <firmware file>
       %prog [options] sync [temperature] [altitude]
       prg   = rfbsl followed by sync
       %prog [options] prg <firmware file> [temperature] [altitude]
       accel = read accelrometer data"""
parser = OptionParser( usage=usage, version="%prog "+version )
parser.add_option( "-d", "--device", dest="device", metavar="DEVICE",
        help="specify USB device of Base Module, will guess if ommited" )
parser.add_option( "-n", "--noreset", action="store_false", dest="reset", default=True,
        help="skip Base Module reset, for resuming streaming etc." )
parser.add_option( "-r", "--raw", action="store_true", dest="raw", default=False,
        help="output raw sensor data" )
parser.add_option( "-v", "--verbose", action="store_true", dest="verbose", default=False,
        help="show CBM communication" )

(opt, args) = parser.parse_args()

#Command must be given
if len( args ) == 0:
    print >> sys.stderr, "ERROR: you must specify a command"
    sys.exit( 5 )

#If no device option given, try to guess
if not opt.device:
    device_guess = ["/dev/ttyACM0", "/dev/ttyUSB0", "/dev/cu.usbmodem001"]
    for path in device_guess:
        if os.path.exists( path ):
            opt.device = path
            break
#Check for device
if (not opt.device) or (not os.path.exists( opt.device )):
    print >> sys.stderr, "ERROR: no Base Module device found, please specify as option"
    sys.exit( 6 )

command = args[0]
if command == "rfbsl":
    if len( args ) < 2:
            print >> sys.stderr, "ERROR: rfbsl requires file name as argument"
            sys.exit( 5 )
    file = args[1]
    if not os.path.isfile( file ):
        print >> sys.stderr, "ERROR: cannot open", file
        sys.exit( 7 )
    bm = CBM( opt.device )
    bm.wbsl_download( file )
elif command == "sync":
    bm = CBM( opt.device )
    temp = 0
    alt = 0
    if len(args) >= 2 and args[1].isdigit():
        temp = int(args[1])
    if len(args) == 3  and args[2].isdigit():
        alt = int(args[2])
    bm.spl_sync(celsius=temp, meters=alt)
elif command == "prg":
    if len( args ) < 2:
            print >> sys.stderr, "ERROR: prg requires file name as argument"
            sys.exit( 5 )
    file = args[1]
    if not os.path.isfile( file ):
        print >> sys.stderr, "ERROR: cannot open", file
        sys.exit( 7 )
    temp = 0
    alt = 0
    if len(args) >= 3 and args[2].isdigit():
        temp = int(args[2])
    if len(args) == 4  and args[3].isdigit():
        alt = int(args[3])
    bm = CBM( opt.device )
    bm.wbsl_download( file )
    bm.spl_sync(celsius=temp, meters=alt)
elif command == "accel":
        bm = CBM( opt.device )
        bm.spl_start()
        while True:
                data = bm.spl_getaccel()
                if data[0]:
                        print str( data[1] ) + " " + str( data[2] ) + " " + str( data[3] )
else:
    print >> sys.stderr, "ERROR: invalid command:", command
    sys.exit( 4 )

