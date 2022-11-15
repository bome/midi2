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

#include "MIDI.h"
#include "MIDICIDataTypes.h"


// a macro for convenience when logging MUID's
#define MUID2STRING(muid)  MIDICIMessage::fmtMUID(muid).toRawUTF8()

#define CHANNEL2STRING(channel)  MIDICIMessage::fmtChannel(channel).toRawUTF8()

/** a static class with utility functions for MIDI-CI */
class MIDICIMessage
    : public MIDIMessage
{
public:
    MIDICIMessage(const MIDIMessage& message);
    MIDICIMessage(MIDIByte* data, int length);

    /** @return true if the given message is a MIDI CI message */
    static bool isMIDICIMessage(const MIDIMessage& message);
    /** @return true if the given message is a MIDI CI message and the message type is in the given range */
    static bool isMIDICIMessage(const MIDIMessage& message, int minMessageType, int maxMessageType);

    MIDIByte getChannel() const;
    void setChannel(MIDIByte channel);
    int getMessageType() const;
    MUID getSourceMUID() const;
    MUID getDestinationMUID() const;
    /**
     * @return true if this CI message is addressed to the given muid, i.e. if this message's
     * destination MUID is BROADCAST or equals the given muid.
     */
    bool isAddressedTo(MUID muid) const;

    /** @return a human readable representation of this MUID */
    static String fmtMUID(MUID muid);

    /** @return a human readable representation of the given channel */
    static String fmtChannel(MIDIByte channel);

    /** initialize all bytes with 0 */
    void clear();

    /**
     * Fill a MIDI-CI header with the given message type, source and destination fields. 
     * @param message the MIDI message to be filled. The length must have at least 15 bytes
     */
    void fillHeader(MIDIByte messageType, MUID source, MUID destination);


    /** Read 2 bytes at the given offset, with a 2x7-bit representation of the given number. */
    uint16 extractNumber14_lsbfirst(int offset) const;
    /** Read 2 bytes at the given offset, with a 2x8-bit representation of the given number. */
    uint16 extractNumber16_lsbfirst(int offset) const;
    /** Read 3 bytes at the given offset, with a 3x8-bit representation of the given number. */
    uint32 extractNumber24_msbfirst(int offset) const;
    /** Read 4 bytes at the given offset, with a 4x7-bit representation of the given number. */
    uint32 extractNumber28_lsbfirst(int offset) const;
    /** Read 4 bytes at the given offset, with a 4x8-bit representation of the given number. */
    uint32 extractNumber32_lsbfirst(int offset) const;

    void writeNumber14_lsbfirst(uint16 number, int offset);
    void writeNumber16_lsbfirst(uint16 number, int offset);
    /** Write 3 bytes at the given offset MSB first, with a 4x8-bit representation of the given number. */
    void writeNumber24_msbfirst(uint32 number, int offset);
    /** Write 4 bytes at the given offset, with a 4x7-bit representation of the given number. */
    void writeNumber28_lsbfirst(uint32 number, int offset);
    /** Write 4 bytes at the given offset, with a 4x8-bit representation of the given number. */
    void writeNumber32_lsbfirst(uint32 number, int offset);
};
