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

// forward declaration to avoid circular dependencies
class MIDICIDevice;

// base class for MIDI CI handlers: receiving, processing, and sending MIDI data

class MIDICIHandlerBase
{
public:
    MIDICIHandlerBase();
    virtual ~MIDICIHandlerBase();

    /** called by MIDICIDevice when this handler is added to it */
    void setDevice(MIDICIDevice* device) { _device = device; }
    MIDICIDevice* getDevice() const { return _device; }

    /** Called by the MIDI-CI Device when it starts */
    virtual void start() {}
    /** Called by the MIDI-CI Device when it stops */
    virtual void stop() {}

    /**
     * Called by MIDICIDevice on an incoming MIDI message
     * @return TRUE if this message was handled
     */
    virtual bool handleMIDI(const MIDIMessage& message) = 0;

    bool sendMIDI(const MIDIMessage& message);

private:
    MIDICIDevice* _device;
};

