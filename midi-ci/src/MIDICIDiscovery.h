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
#include "MIDICIConstants.h"
#include "MIDICIDataTypes.h"
#include "MIDICIHandlerBase.h"
#include "MIDICIMessage.h"
#include "MIDICIDeviceInfo.h"


class MIDICIDiscovery
    : public MIDICIHandlerBase
{
public:
    MIDICIDiscovery();

    // MIDICIHandlerBase integration

    void start() override;
    void stop() override;
    bool handleMIDI(const MIDIMessage& message) override;

    // Actions

    void triggerDiscovery();
    /** generate a new random MUID, update MIDICIDevice, and possibly send InvalidateMUID message */
    void generateNewRandomMUID(bool canSendInvalidateMUID = true);

private:
    /** Generate a new random MUID. */
    MUID nextRandomMUID() const;
    bool handleCollision(const MIDICIMessage& message);
    void logDiscoveryMessage(bool isTX, const MIDICIMessage& discoveryMessage, bool isReply, const MIDICIDeviceInfo& info);

    // handle specific incoming messages
    bool handleDiscovery(const MIDICIMessage& message, bool isReply);
    bool handleInvalidateMUID(const MIDICIMessage& message);
    bool handleNAK(const MIDICIMessage& message);

    // send messages
    bool sendInvalidateMUID(MUID muidToInvalidate);
    bool sendDiscovery(bool isReply, MUID destination = MIDICI_MUID_BROADCAST);
};
