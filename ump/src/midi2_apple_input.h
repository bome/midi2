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

#include "midi2.h"
#include <CoreMIDI/CoreMIDI.h>


/** A MIDI Input device abstraction for CoreMIDI*/
class MIDI2AppleInput
{
public:
    MIDI2AppleInput();
    ~MIDI2AppleInput();

    int getDeviceCount() const;
    
    /** @return the device name using the given buffer, or nullptr on error */
    const char* getDeviceName(int deviceId, char* buffer, uint bufferSize) const;
    
    bool open(int deviceId, MIDI2Processor* _receiver);
    void close();
    
    void handleMIDIEventList(const MIDIEventList* eventList);
    
private:
    MIDI2Processor* receiver = nullptr;
    MIDIPortRef port = 0;
    MIDIEndpointRef endpoint = 0;
};

