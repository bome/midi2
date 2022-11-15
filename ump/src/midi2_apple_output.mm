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
#include "midi2_apple_output.h"
#include "midi2_apple_util.h"
#include "debug.h"
#include <Cocoa/Cocoa.h>

MIDI2AppleOutput::MIDI2AppleOutput()
{
    // nothing
}


MIDI2AppleOutput::~MIDI2AppleOutput()
{
    close();
}


int MIDI2AppleOutput::getDeviceCount() const
{
    return (int)MIDIGetNumberOfDestinations();
}


const char* MIDI2AppleOutput::getDeviceName(int _deviceId, char* buffer, uint bufferSize) const
{
    MIDIEndpointRef endpoint = MIDIGetDestination(_deviceId);
    if (endpoint != 0)
    {
        return MIDI2AppleUtil::getMIDIStringProperty(endpoint, kMIDIPropertyDisplayName, buffer, bufferSize);
    }
    return nullptr;
}


bool MIDI2AppleOutput::open(int deviceId)
{
    // if currently open, close first
    close();

    // open port
    endpoint = MIDIGetDestination(deviceId);
    if (endpoint == 0)
    {
        PRINT("ERROR: cannot get endpoint #%d", deviceId);
        return false;
    }

    OSStatus result = MIDIOutputPortCreate(MIDI2AppleUtil::getSharedMIDIClient(), CFSTR("output port"), /*OUT*/ &port);
    if (result != noErr)
    {
        PRINT("ERROR: cannot create output port #%d.", deviceId);
        close();
        return false;
    }

    isVirtual = false;

    return true;
}


void MIDI2AppleOutput::close()
{
    if (port != 0)
    {
        MIDIPortDispose(port);
        port = 0;
    }
    
}


bool MIDI2AppleOutput::openVirtualPort(const char* name)
{
    close();
    
    CFStringRef cfName = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    
    OSStatus result =     MIDISourceCreateWithProtocol(MIDI2AppleUtil::getSharedMIDIClient(), cfName,
                                 kMIDIProtocol_2_0, /*OUT*/ &endpoint);

    CFRelease(cfName);

    if (result != noErr)
    {
        PRINT("ERROR: cannot create virtual output port \"%s\".", name);
        close();
        return false;
    }

    isVirtual = true;
    
    return true;
}


bool MIDI2AppleOutput::send(const UMPacket& packet, uint64 timestamp /* = 0 */)
{
    OSStatus result;
    
    // eventList contains space for up to 64 words
    MIDIEventList eventList;
    MIDIEventPacket* curPacket = MIDIEventListInit(&eventList,
                                                   kMIDIProtocol_2_0);

    curPacket = MIDIEventListAdd(&eventList,
                                 sizeof(eventList),
                                 curPacket,
                                 (MIDITimeStamp)timestamp,
                                 packet.getSizeInWords(),
                                 (const UInt32*)packet.getData());
    
    if (isVirtual)
    {
        result = MIDIReceivedEventList(endpoint, &eventList);
    }
    else
    {
        result = MIDISendEventList(port, endpoint, &eventList);
    }

    if (result != noErr)
    {
        PRINT1("ERROR: cannot send UMP to endpoint.");
        return false;
    }
    return true;
}
