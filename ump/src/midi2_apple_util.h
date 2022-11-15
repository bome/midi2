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


/** Print received UMPackets to stdout. */
class MIDI2AppleUtil
{
public:
    static MIDIObjectRef getSharedMIDIClient();

    /** @return a static buffer containing the property string, or nullptr if property does not exist */
    static const char* getMIDIStringProperty(MIDIObjectRef dev, CFStringRef prop, char* outBuffer, uint outBufferSize);

    static uint64 getTimestamp();
    
protected:

    static const char* getNotificationTypeString(const MIDINotification *message);

    static void MIDIClientNotification(const MIDINotification *message, void *refCon);

private:
    class StaticData
    {
    public:
        ~StaticData();
        MIDIObjectRef sharedMIDIClient = 0;
    };
    static StaticData staticData;
};

