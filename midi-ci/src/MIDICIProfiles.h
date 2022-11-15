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
#include "MIDICIHandlerBase.h"
#include "MIDICIMessage.h"
#include "MIDICIDataTypes.h"
#include "MIDICIProfileTypes.h"

class MIDICIProfiles
	: public MIDICIHandlerBase
    , protected MIDICIProfileList::Listener
{
public:
	MIDICIProfiles();
    ~MIDICIProfiles() override;

    // MIDICIHandlerBase integration

    void start() override;
    void stop() override;
    bool handleMIDI(const MIDIMessage& message) override;

    // Actions

    void triggerProfileInquiry(MUID destination);

protected:
    void onProfileSpecificDataChange(MIDICIProfileList& list, MIDICIProfileState& state) override;
    void onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel) override;

private:
    /** @return the extracted Profile ID, or an invalid ID on error */
    static MIDICIProfileId getId(const MIDICIMessage& message);
    void startListening();
    void stopListening();

    // handle specific incoming messages
    bool handleProfileInquiry(const MIDICIMessage& message);
    bool handleProfileInquiryReply(const MIDICIMessage& message);
    bool handleProfileSetOnOrOff(const MIDICIMessage& message, bool isOn);
    bool handleProfileReport(const MIDICIMessage& message, bool isEnabled);
    bool handleProfileSpecificData(const MIDICIMessage& message);

    // send messages
    bool sendMessageWithOneProfile(MIDIByte messageType, MIDIByte channel, MUID destination, const MIDICIProfileId& id);
    bool sendProfileInquiry(MUID destination);
    bool sendProfileInquiryReply(MUID destination);
    /** send inquiry reply for the given channel */
    bool sendProfileInquiryReply(MIDIByte channel, MUID destination, bool alsoSendZeroProfiles);
    bool sendProfileSetOnOrOff(MIDIByte channel, MUID destination, const MIDICIProfileId& id, bool isOn);
    bool sendProfileReport(MIDIByte channel, const MIDICIProfileId& id, bool isEnabled);
    bool sendProfileSpecificData(MUID destination, const MIDICIProfileId& id, const MemoryBlock& data);
};

