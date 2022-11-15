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
#include "JuceMidiBridge.h"
#include "Logger.h"

JuceMidiBridge::JuceMidiBridge()
    : _receiver(nullptr)
{
    // nothing
}

void JuceMidiBridge::setMIDIPortNames(String inputPort, String outputPort)
{
    _inputPort = inputPort;
    _outputPort = outputPort;
}

bool JuceMidiBridge::start()
{
    auto midiInputs = MidiInput::getAvailableDevices();
    for (auto thisInput : midiInputs)
    {
        if (thisInput.name == _inputPort)
        {
            input = MidiInput::openDevice(thisInput.identifier, this);
            if (input.get() != nullptr)
            {
                input->start();
            }
        }
    }

    auto midiOutputs = MidiOutput::getAvailableDevices();
    for (auto thisOutput : midiOutputs)
    {
        if (thisOutput.name == _outputPort)
        {
            output = MidiOutput::openDevice(thisOutput.identifier);
        }
    }

    bool errorsOccurred = false;
    if (input.get() == nullptr)
    {
        LOG("ERROR: could not find or open MIDI INPUT: %s", _inputPort.toRawUTF8());
        errorsOccurred = true;
    }
    else
    {
        LOG("Started MIDI INPUT: %s", _inputPort.toRawUTF8());
    }
    if (output.get() == nullptr)
    {
        LOG("ERROR: could not find or open MIDI OUTPUT: %s", _outputPort.toRawUTF8());
        errorsOccurred = true;
    }
    else
    {
        LOG("Started MIDI OUTPUT: %s", _outputPort.toRawUTF8());
    }

    if (errorsOccurred)
    {
        LOG("note: the MIDI ports are set up in Constants.h");
    }

    return (input.get() != nullptr) && (output.get() != nullptr);
}

void JuceMidiBridge::stop()
{
    if (input.get() != nullptr)
    {
        LOG("Closing MIDI INPUT: %s", _inputPort.toRawUTF8());
        input->stop();
        input = nullptr;
    }

    if (output.get() != nullptr)
    {
        LOG("Closing MIDI OUTPUT: %s", _outputPort.toRawUTF8());
        output = nullptr;
    }
}


bool JuceMidiBridge::sendMIDI(const MIDIMessage& message)
{
    if (output.get() != nullptr)
    {
        output->sendMessageNow(MidiMessage(message.getData(), message.getLength()));
        return true;
    }
    return false;
}


void JuceMidiBridge::handleIncomingMidiMessage(MidiInput*, const MidiMessage& message)
{
    if (_receiver != nullptr)
    {
        _receiver->receiveMIDI(MIDIMessage((MIDIByte*)message.getRawData(), message.getRawDataSize()));
    }
}
