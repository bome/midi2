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

#define MIDICI_CURRENT_VERSION                (0x01) // MIDI-CI 1.1

#define MIDICI_CATEGORY_RESERVED              (1<<0)
#define MIDICI_CATEGORY_PROTOCOLNEGOTIATION   (1<<1)
#define MIDICI_CATEGORY_PROFILECONFIGURATION  (1<<2)
#define MIDICI_CATEGORY_PROPERTYEXCHANGE      (1<<3)

/** address the port, not an individual channel */
#define MIDICI_CHANNEL_PORT                (0x7F)
/* for internal use -- an invalid channel */
#define MIDICI_CHANNEL_INVALID             (0xFF)
/** the maximum value of a randomly generated MUID (values above are reserved) */
#define MIDICI_MUID_MAX_ASSIGNABLE_VALUE   (0x0FFFFF00-1)
#define MIDICI_MUID_BROADCAST              (0x0FFFFFFF) // 28-bit
#define MIDICI_MUID_INVALID                (0x0FFFFFFE)
/** Broadcast messages must not be larger than 512 bytes in size. */
#define MIDICI_MUID_BROADCAST_MAX_MESSAGE_SIZE_BYTES  (512)

#define MIDI_CHANNEL_MAX                   (0x0F)
#define MIDI_SYSEX_START                   (0xF0)
#define MIDI_SYSEX_END                     (0xF7)
#define MIDI_SYSEX_UNIVERSAL_NON_REALTIME  (0x7E)
#define MIDI_SYSEX_SUBID1_MIDICI           (0x0D)

#define MIDICI_INDEX_SYSEX_START      (0)
#define MIDICI_INDEX_SYSEX_ID         (1)
#define MIDICI_INDEX_CHANNEL          (2) // Device ID field
#define MIDICI_INDEX_SUBID1           (3)
#define MIDICI_INDEX_MESSAGE_TYPE     (4) // Sub ID#2
#define MIDICI_INDEX_VERSION          (5)
#define MIDICI_INDEX_SRC_MUID         (6)   // LSB first
#define MIDICI_INDEX_DST_MUID         (10)  // LSB first

#define MIDICI_MINIMUM_RECEIVABLE_SYSEX_SIZE  (128)
#define MIDICI_COMMON_HEADER_LENGTH (MIDICI_INDEX_DST_MUID + 4)

// MIDI-CI management message definition
#define MIDICI_MESSAGE_TYPE_MANAGEMENT_BEGIN (0x70)
#define MIDICI_MESSAGE_TYPE_MANAGEMENT_END   (0x7F)
#define MIDICI_MESSAGE_DISCOVERY             (0x70)
#define MIDICI_MESSAGE_DISCOVERY_REPLY       (0x71)
#define MIDICI_MESSAGE_INVALIDATE_MUID       (0x7E)
#define MIDICI_MESSAGE_NAK                   (0x7F)

// message size definitions
#define MIDICI_MESSAGE_DISCOVERY_SIZE        (31)
#define MIDICI_MESSAGE_INVALIDATEMUID_SIZE   (19)
#define MIDICI_MESSAGE_NAK_SIZE              (15)

// MIDI-CI Profiles
#define MIDICI_MESSAGE_TYPE_PROFILES_BEGIN      (0x20)
#define MIDICI_MESSAGE_TYPE_PROFILES_END        (0x2F)
#define MIDICI_MESSAGE_TYPE_PROFILES_INQUIRY    (0x20)
#define MIDICI_MESSAGE_TYPE_PROFILES_REPLY      (0x21)
#define MIDICI_MESSAGE_TYPE_PROFILES_SET_ON     (0x22)
#define MIDICI_MESSAGE_TYPE_PROFILES_SET_OFF    (0x23)
#define MIDICI_MESSAGE_TYPE_PROFILES_REPORT_ON  (0x24)
#define MIDICI_MESSAGE_TYPE_PROFILES_REPORT_OFF (0x25)
#define MIDICI_MESSAGE_TYPE_PROFILES_SPECIFIC_DATA (0x2F)

#define MIDICI_PROFILE_ID_STANDARD_DEFINED   (0x7E)

#define MIDICI_PROFILE_ID_SIZE_BYTES         (5)
#define MIDICI_PROFILE_ID_INDEX_ID           (0)
#define MIDICI_PROFILE_ID_INDEX_BANK         (1)
#define MIDICI_PROFILE_ID_INDEX_NUMBER       (2)
#define MIDICI_PROFILE_ID_INDEX_VERSION      (3)
#define MIDICI_PROFILE_ID_INDEX_LEVEL        (4)
#define MIDICI_PROFILE_ID_INDEX_MANUID       (0)
#define MIDICI_PROFILE_ID_INDEX_MANU_INFO1   (3)
#define MIDICI_PROFILE_ID_INDEX_MANU_INFO2   (4)
