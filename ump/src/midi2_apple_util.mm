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
#include "midi2_apple_util.h"
#include "debug.h"
#include <mach/mach_time.h>

MIDI2AppleUtil::StaticData MIDI2AppleUtil::staticData;

MIDI2AppleUtil::StaticData::~StaticData()
{
    if (sharedMIDIClient != 0)
    {
        //PRINT1("Disposing shared MIDI client");
        MIDIClientDispose(sharedMIDIClient);
        sharedMIDIClient = 0;
    }
}

MIDIObjectRef MIDI2AppleUtil::getSharedMIDIClient()
{
    if (staticData.sharedMIDIClient == 0)
    {
        OSStatus result = MIDIClientCreate(CFSTR("MIDI Client"), &MIDIClientNotification, NULL/*notificationRefCon*/, &staticData.sharedMIDIClient);
        if (result != noErr)
        {
            PRINT1("ERROR: cannot initialize MIDI client.");
        }
    }
    return staticData.sharedMIDIClient;
}


const char* MIDI2AppleUtil::getNotificationTypeString(const MIDINotification *message)
{
    switch (message->messageID)
    {
        case kMIDIMsgSetupChanged: return "Setup Changed";
        case kMIDIMsgObjectAdded: return "Device Added";
        case kMIDIMsgObjectRemoved: return "Device Removed";
        case kMIDIMsgPropertyChanged: return "Property Changed";
        case kMIDIMsgThruConnectionsChanged: return "Thru Connections Changed";
        case kMIDIMsgSerialPortOwnerChanged: return "Serial Port Owner Changed";
        case kMIDIMsgIOError: return "IO Error";
    }
    return "(unknown)";
}


void MIDI2AppleUtil::MIDIClientNotification(const MIDINotification *message, void *refCon)
{
    PRINT("MIDIClientNotification: %s", getNotificationTypeString(message));
}


const char* MIDI2AppleUtil::getMIDIStringProperty(MIDIObjectRef dev, CFStringRef prop, char* outBuffer, uint outBufferSize)
{
    if (outBuffer != nullptr && outBufferSize >= 2)
    {
        CFStringRef cfValue = 0;
        MIDIObjectGetStringProperty(dev, prop, &cfValue);
        outBuffer[0] = 0;
        if (cfValue != 0)
        {
            CFStringGetCString(cfValue, outBuffer, outBufferSize - 1, kCFStringEncodingUTF8);
            // just to be sure
            outBuffer[outBufferSize - 1] = 0;
            CFRelease(cfValue);
            return outBuffer;
        }
    }
    return nullptr;
}

uint64 MIDI2AppleUtil::getTimestamp()
{
    return (MIDITimeStamp)mach_absolute_time();
}
