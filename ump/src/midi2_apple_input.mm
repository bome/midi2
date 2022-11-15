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
#include "midi2_apple_input.h"
#include "midi2_apple_util.h"
#include "debug.h"


MIDI2AppleInput::MIDI2AppleInput()
{
    // nothing
}

MIDI2AppleInput::~MIDI2AppleInput()
{
    close();
}

int MIDI2AppleInput::getDeviceCount() const
{
    return (int)MIDIGetNumberOfSources();
}


const char* MIDI2AppleInput::getDeviceName(int _deviceId, char* buffer, uint bufferSize) const
{
    MIDIEndpointRef endpoint = MIDIGetSource(_deviceId);
    if (endpoint != 0)
    {
        return MIDI2AppleUtil::getMIDIStringProperty(endpoint, kMIDIPropertyDisplayName, buffer, bufferSize);
    }
    return nullptr;
}

bool MIDI2AppleInput::open(int deviceId, MIDI2Processor* _receiver)
{
    // if currently open, close first
    close();

    receiver = _receiver;

    // open port
    endpoint = MIDIGetSource(deviceId);
    if (endpoint == 0)
    {
        PRINT("ERROR: cannot get endpoint #%d", deviceId);
        return false;
    }

    MIDIReceiveBlock receiveBlock = ^(const MIDIEventList* eventList,
                                      void* srcConnRefCon)
    {
        handleMIDIEventList(eventList);
    };
    
    // we're doing MIDI 2.0 protocol now.
    // If we're connected to a MIDI 1.0 device, macOS will translate on its own.
    OSStatus result = MIDIInputPortCreateWithProtocol(MIDI2AppleUtil::getSharedMIDIClient(), CFSTR("input port"), kMIDIProtocol_2_0, &port, receiveBlock);
    if (result != noErr)
    {
        PRINT("ERROR: cannot create input port #%d.", deviceId);
        close();
        return false;
    }
    
    result = MIDIPortConnectSource(port, endpoint, nullptr);
    
    if (result != noErr)
    {
        PRINT("ERROR: cannot connect endpoint to input port #%d.", deviceId);
        close();
        return false;
    }
    
    return true;
}

void MIDI2AppleInput::close()
{
    receiver = nullptr;
    
    if (port != 0)
    {
        MIDIPortDispose(port);
        port = 0;
    }
    
}


void MIDI2AppleInput::handleMIDIEventList(const MIDIEventList* eventList)
{
    if (receiver == nullptr)
    {
        // no need to process anything...
        return;
    }

    const MIDIEventPacket* packet = eventList->packet;
    for (int index = 0; index < (int)eventList->numPackets; index++)
    {
        if (packet != NULL)
        {
            receiver->processRawUMP((uint64)packet->timeStamp, (const uint32*)packet->words, (int)packet->wordCount);
        }
        packet = MIDIEventPacketNext(packet);
    }
}
