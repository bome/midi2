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
#include "MIDICIDataTypes.h"
#include "MIDICIDeviceInfo.h"


// forward declaration to avoid circular dependencies
class MIDICIHandlerBase;
class MIDICIMessage;

/**
 * A representation of a MIDI-CI device
 */
class MIDICIDevice
    : public MIDIReceiver
    , protected MIDICIProfileList::Listener
{
public:
    MIDICIDevice();
    ~MIDICIDevice() override;

    void addHandler(MIDICIHandlerBase* handler);
    void removeHandler(MIDICIHandlerBase* handler);

    /** must be used for this MIDI-CI device to be able to send MIDI messages */
    void setSender(MIDISender* sender) { _sender = sender; }

    /** Start this device. All handlers will be started, too. */
    void start();
    /** Stop this device. All handlers will be stopped, too. */
    void stop();

    /**
     * Distribute the incoming MIDI message to all handlers.
     */
    void receiveMIDI(const MIDIMessage& message) override;

    /** Send a MIDI message via the MIDISender */
    bool sendMIDI(const MIDIMessage& message);
    /** if any messages have been sent at all (for MUID handling) */
    bool hasSentMessages() const { return messagesSent; }
    void resetSentMessagesFlag() { messagesSent = false; }

    /** Send a NAK message, based on the given received CI Message */
    bool sendNAK(const MIDICIMessage& receivedMessage);

    //
    // LocalInfo
    //
    MIDICIDeviceInfo& getLocalInfo() { return localInfo; }
    const MIDICIDeviceInfo& getLocalInfo() const { return localInfo; }

    // convenience accessor
    MUID getLocalMUID() const { return localInfo.getMUID(); }
    void setLocalMUID(MUID muid) { localInfo.setMUID(muid); }
    bool isLocalMUID(MUID muid) const { return getLocalMUID() == muid; }

    //
    // RemoteInfos
    //
    void addRemoteInfo(const MIDICIDeviceInfo& info);
    /** @return true if the given info existed and was removed */
    bool removeRemoteInfo(MUID muid);
    bool hasRemoteInfo(MUID muid) const;

    /**
     * Look up the remote info for the given muid.
     * If not found, return an invalid RemoteInfo.
     */
    MIDICIDeviceInfo& getRemoteInfo(MUID muid);
    const MIDICIDeviceInfo& getRemoteInfo(MUID muid) const;

    int getRemoteInfoCount() const { return remoteInfos.size(); }
    MIDICIDeviceInfo& getRemoteInfoByIndex(int index) { return remoteInfos.getReference(index); }
    const MIDICIDeviceInfo& getRemoteInfoByIndex(int index) const { return remoteInfos.getReference(index); }

    MIDICIDeviceInfo* getMostRecentRemoteInfo();
    const MIDICIDeviceInfo* getMostRecentRemoteInfo() const;

    /** a global listener for any profile changes from remote device */
    void addRemoteProfileListener(MIDICIProfileList::Listener* listener);
    void removeRemoteProfileListener(MIDICIProfileList::Listener* listener);

    /** @return true if either the local MUID or any of the known remote MUIDs matches the given one */
    bool hasLocalOrRemoteMUID(MUID muid) const;

protected:
    void subscribeAllRemoteProfiles();
    void unsubscribeAllRemoteProfiles();
    // MIDICIProfileList::Listener passed through to remoteProfileListeners
    void onProfileAdded(MIDICIProfileList& list, MIDICIProfileState& state);
    void onProfileRemoved(MIDICIProfileList& list, MIDICIProfileState& state);
    void onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel);
    void onProfileAvailableChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel);
    void onProfileSpecificDataChange(MIDICIProfileList& list, MIDICIProfileState& state);

private:
    Array<MIDICIHandlerBase*> handlers;
    MIDISender* _sender;
    bool messagesSent;
    MIDICIDeviceInfo localInfo;
    SortedSet<MIDICIDeviceInfo, CriticalSection> remoteInfos;
    ListenerList<MIDICIProfileList::Listener> remoteProfileListeners;
};
