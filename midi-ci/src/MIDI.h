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

#include <JuceHeader.h>

//
// Types
//

typedef unsigned char MIDIByte;


//
// MIDIMessage
//

/**
 * The base MIDIMessage class. It merely stores a reference to a MIDI message,
 * it does not allocate or deallocate the referenced memory block.
 */
class MIDIMessage
{
public:
    /**
     * Create a new MIDIMessage object.
     * NOTE: the data is only referenced, not copied!
     *       For copying the data, use HeapMIDIMessage.
     */
    MIDIMessage(const MIDIByte* data, int length);

    const MIDIByte* getData() const { return _data; }
    MIDIByte* getData() { return _data; }
    const MIDIByte* getData(int offset) const { return &_data[offset]; }
    MIDIByte* getData(int offset) { return &_data[offset]; }
    int getLength() const { return _length; }

    String toHex(int maxBytes = -1) const;

protected:
    static char toHexNibble(MIDIByte b);
    MIDIByte* _data;
    int _length;

private:
    // prevent instantiation without parameters
    MIDIMessage() {}
};


//
// HeapMIDIMessage
//

/**
 * A MIDIMessage which copies the message data in an own memory block.
 */
class HeapMIDIMessage
    : public MIDIMessage
{
public:
    HeapMIDIMessage(const MIDIByte* data, int length);
    HeapMIDIMessage(const MIDIMessage& message);
    HeapMIDIMessage(const HeapMIDIMessage& message);

    HeapMIDIMessage& operator=(const HeapMIDIMessage& message);
    HeapMIDIMessage& operator=(const MIDIMessage& message);

protected:
    void initPointers();
    MemoryBlock heap;
};


//
// MIDISender
//

// a listener type of class for sending MIDI messages to a MIDI OUT port
class MIDISender
{
public:
    virtual ~MIDISender() {}

    virtual bool sendMIDI(const MIDIMessage& message) = 0;
};


//
// MIDIReceiver
//

// a listener type of class for receiving MIDI messages from a MIDI IN port
class MIDIReceiver
{
public:
    virtual ~MIDIReceiver() {}

    virtual void receiveMIDI(const MIDIMessage& message) = 0;
};
