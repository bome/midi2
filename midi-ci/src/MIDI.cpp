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
#include "MIDI.h"

//
// MIDIMessage
//

MIDIMessage::MIDIMessage(const MIDIByte* data, int length)
    : _data(const_cast<MIDIByte*>(data))
    , _length(length)
{
    // nothing
}

String MIDIMessage::toHex(int maxBytes) const
{
    int c = (_length < maxBytes || maxBytes < 0) ? _length : maxBytes;
    String ret;
    ret.preallocateBytes((size_t) (c * 3));
    char oneByte[4];
    oneByte[2] = ' ';
    oneByte[3] = '\0';
    for (int i = 0; i < c; i++)
    {
        oneByte[0] = toHexNibble(_data[i] >> 4);
        oneByte[1] = toHexNibble(_data[i] & 0xF);
        if (i == c - 1)
        {
            oneByte[2] = '\0';
        }
        ret += oneByte;
    }
    return ret;
}

char MIDIMessage::toHexNibble(MIDIByte b)
{
    static const char* hexDigits = "0123456789ABCDEF";
    return hexDigits[b & 0x0F];
}


//
// HeapMIDIMessage
//

HeapMIDIMessage::HeapMIDIMessage(const MIDIByte* data, int length)
    : MIDIMessage(nullptr, 0)
    , heap(data, (size_t)length)
{
    initPointers();
}

void HeapMIDIMessage::initPointers()
{
    // adjust the super class pointers
    _data = (MIDIByte*)heap.getData();
    _length = (int)heap.getSize();
}

HeapMIDIMessage::HeapMIDIMessage(const MIDIMessage& message)
    : MIDIMessage(nullptr, 0)
    , heap(message.getData(), (size_t)message.getLength())
{
    initPointers();
}

HeapMIDIMessage::HeapMIDIMessage(const HeapMIDIMessage& message)
    : MIDIMessage(nullptr, 0)
    , heap(message.heap)
{
    initPointers();
}

HeapMIDIMessage& HeapMIDIMessage::operator=(const HeapMIDIMessage& message)
{
    heap = message.heap;
    initPointers();
    return *this;
}

HeapMIDIMessage& HeapMIDIMessage::operator=(const MIDIMessage& message)
{
    heap.replaceAll(message.getData(), (size_t)message.getLength());
    initPointers();
    return *this;
}
