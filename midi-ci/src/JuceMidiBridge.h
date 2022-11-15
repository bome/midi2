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
#include "MIDI.h"

// a simple class to receive and send MIDI from a given pair of MIDI devices

class JuceMidiBridge
    : public MIDISender
    , public MidiInputCallback
{
public:
    JuceMidiBridge();
    
    /** must be called by the owner of the bridge where MIDI messages will be received. */
    void setReceiver(MIDIReceiver* receiver) { _receiver = receiver; }
    /** must be called by the owner of the bridge to define the MIDI input and output ports */
    void setMIDIPortNames(String inputPort, String outputPort);

    bool start();
    void stop();

    // MIDISender
    bool sendMIDI(const MIDIMessage& message) override;

    // MidiInputCallback
    void handleIncomingMidiMessage(MidiInput*, const MidiMessage& message) override;

private:
    MIDIReceiver* _receiver;
    String _inputPort;
    String _outputPort;

    std::unique_ptr<MidiInput> input;
    std::unique_ptr<MidiOutput> output;
};
