/*
 * Copyright © 2021-2022 by Florian Bomers, Bome Software GmbH & Co. KG
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without 
 * restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once

// Windows
#ifdef _WIN32
#define TARGET_WIN
#endif

// Apple
#if defined(__APPLE__) || defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__)
#define TARGET_APPLE
#endif

// Linux
#if defined(__GNUC__) && ( defined(__linux__) || defined(__linux) )
#define TARGET_LINUX
#endif

typedef unsigned char      byte;
typedef unsigned int       uint;
typedef signed char        int8;
typedef unsigned char      uint8;
typedef uint8              uint4; // symbolic nibble
typedef uint8              uint7; // symbolic 7-bit number
typedef short              int16;
typedef unsigned short     uint16;
typedef int                int32;
typedef unsigned int       uint32;
typedef int                int32;
typedef unsigned int       uint32;

#ifdef TARGET_WIN
typedef signed __int64     int64;
typedef unsigned __int64   uint64;
#else
typedef long long          int64;
typedef unsigned long long uint64;
#endif

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

#define MIDI_CHANNEL_COUNT       (16)
#define MIDI_CHANNEL_MAX         (0x0F)

// MIDI Status Bytes
#define MIDI_NOTEOFF         0x80 // 3 bytes
#define MIDI_NOTEON          0x90 // 3 bytes
#define MIDI_KEYAFTERTOUCH   0xa0 // 3 bytes
#define MIDI_CONTROLCHANGE   0xb0 // 3 bytes
#define MIDI_PROGRAMCHANGE   0xc0 // 2 bytes
#define MIDI_CHANAFTERTOUCH  0xd0 // 2 bytes
#define MIDI_PITCHBEND       0xe0 // 3 bytes (little endian)
#define MIDI_SYSTEMMESSAGE   0xf0
#define MIDI_BEGINSYSEX      0xf0
#define MIDI_MTCQUARTERFRAME 0xf1 // 2 bytes
#define MIDI_SONGPOSPTR      0xf2 // 3 bytes
#define MIDI_SONGSELECT      0xf3 // 2 bytes
#define MIDI_TUNEREQUEST     0xF6 // 1 byte
#define MIDI_ENDSYSEX        0xF7
#define MIDI_SYSEX_CONT      0xF7 // Windows
#define MIDI_TIMINGCLOCK     0xF8 // 1 byte
#define MIDI_START           0xFA // 1 byte
#define MIDI_CONTINUE        0xFB // 1 byte
#define MIDI_STOP            0xFC // 1 byte
#define MIDI_ACTIVESENSING   0xFE // 1 byte
#define MIDI_SYSTEMRESET     0xFF // 1 byte
//controllers
#define MIDI_CC_BANKSELECT_MSB   0x00 /*0*/
#define MIDI_CC_BANKSELECT_LSB   0x20 /*32*/

#define MIDI_CC_MODULATION       0x01
#define MIDI_CC_VOLUME           0x07
#define MIDI_CC_BALANCE          0x08
#define MIDI_CC_PAN              0x0A

#define MIDI_CC_DATA_MSB         0x06
#define MIDI_CC_DATA_LSB         0x26 /*38*/
#define MIDI_CC_DATA_INC         0x60 /*96*/
#define MIDI_CC_DATA_DEC         0x61 /*97*/
#define MIDI_CC_NRPN_LSB         0x62 /*98*/
#define MIDI_CC_NRPN_MSB         0x63 /*99*/
#define MIDI_CC_RPN_LSB          0x64 /*100*/
#define MIDI_CC_RPN_MSB          0x65 /*101*/

#define MIDI_CC_ALL_SOUND_OFF       0x78 // 120
#define MIDI_CC_RESET_ALL_CTRLS     0x79 // 121
#define MIDI_CC_LOCAL_MODE          0x7A
#define MIDI_CC_ALL_NOTES_OFF       0x7B
#define MIDI_CC_OMNI_OFF            0x7C
#define MIDI_CC_OMNI_ON             0x7D
#define MIDI_CC_MONO_MODE           0x7E
#define MIDI_CC_POLY_MODE           0x7F

#define MIDI_RPN_PITCH_BEND_RANGE       0x00
#define MIDI_RPN_FINE_TUNING            0x01
#define MIDI_RPN_COARSE_TUNING          0x02
#define MIDI_RPN_TUNING_PROGRAM_CHANGE  0x03
#define MIDI_RPN_TUNING_BANK_SELECT     0x04
#define MIDI_RPN_MOD_DEPTH              0x05
#define MIDI_RPN_MPE_MODE               0x06

#define MIDI_SYSEX_UNIVERSAL_NON_REALTIME  (0x7E)

#define NOTE_OFF_VELOCITY_FOR_NOTE_ON_WITH_ZERO_VELOCITY  (64)

//

uint32 getMilliTime();

// random numbers

int brandom();
int brandom_max();
int brandom(int min, int max);
uint8 brandom8();
uint16 brandom16();
uint32 brandom32();
