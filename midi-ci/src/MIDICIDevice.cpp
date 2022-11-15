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
#include "MIDICIDevice.h"
#include "MIDICIHandlerBase.h"
#include "MIDICIMessage.h"
#include "MIDICIConstants.h"
#include "Constants.h"
#include "Logger.h"

MIDICIDevice::MIDICIDevice()
    : _sender(nullptr)
    , messagesSent(false)
{
    localInfo.manufacturerId = MIDICI_DEFAULT_MANUFACTURER_ID;
    localInfo.familyId = MIDICI_DEFAULT_FAMILY_ID;
    localInfo.modelId = MIDICI_DEFAULT_MODEL_ID;
    localInfo.versionId = MIDICI_DEFAULT_VERSION_ID;
}

MIDICIDevice::~MIDICIDevice()
{
    stop();
    for (auto handler : handlers)
    {
        handler->setDevice(nullptr);
    }
}

void MIDICIDevice::addHandler(MIDICIHandlerBase* handler)
{
    if (handler != nullptr)
    {
        handlers.addIfNotAlreadyThere(handler);
        handler->setDevice(this);
    }
}

void MIDICIDevice::removeHandler(MIDICIHandlerBase* handler)
{
    handlers.removeFirstMatchingValue(handler);
    if (handler->getDevice() == this)
    {
        handler->setDevice(nullptr);
    }
}

void MIDICIDevice::start()
{
    messagesSent = false;
    for (auto handler : handlers)
    {
        handler->start();
    }
}

void MIDICIDevice::stop()
{
    for (auto handler : handlers)
    {
        handler->stop();
    }
}

void MIDICIDevice::receiveMIDI(const MIDIMessage& message)
{
    // bail out early on if not a -MIDI-CI message
    if (!MIDICIMessage::isMIDICIMessage(message))
    {
        return;
    }

    // record last receive time
    MIDICIDeviceInfo& remoteInfo = getRemoteInfo(MIDICIMessage(message).getSourceMUID());
    remoteInfo.lastReceiveTime = Time::currentTimeMillis();

    LOG("RX: %s", message.toHex().toRawUTF8());
    bool handled = false;
    for (auto handler : handlers)
    {
        if (handler->handleMIDI(message))
        {
            handled = true;
            break;
        }
    }

    if (!handled)
    {
        LOG("--> MIDI message not handled, return NAK");
        sendNAK(MIDICIMessage(message));
    }
}

bool MIDICIDevice::sendMIDI(const MIDIMessage& message)
{
    LOG("TX: %s", message.toHex().toRawUTF8());
    if (_sender != nullptr)
    {
        if (_sender->sendMIDI(message))
        {
            messagesSent = true;
            return true;
        }
    }
    return false;
}

bool MIDICIDevice::sendNAK(const MIDICIMessage& receivedMessage)
{
    if (receivedMessage.getLength() >= MIDICI_INDEX_SRC_MUID + 4)
    {
        MIDIByte message[MIDICI_MESSAGE_NAK_SIZE];
        MIDICIMessage reply(message, MIDICI_MESSAGE_NAK_SIZE);
        reply.clear();
        reply.fillHeader(MIDICI_MESSAGE_NAK, getLocalMUID(), receivedMessage.getSourceMUID());
        LOG("TX NAK to %s", MUID2STRING(receivedMessage.getSourceMUID()));
        return sendMIDI(reply);
    }

    LOG("Cannot send NAK: incoming message does not specify source MUID (len=%d)", receivedMessage.getLength());
    return false;
}

void MIDICIDevice::addRemoteProfileListener(MIDICIProfileList::Listener* listener)
{
    remoteProfileListeners.add(listener);
    if (remoteProfileListeners.size() == 1)
    {
        subscribeAllRemoteProfiles();
    }
}

void MIDICIDevice::removeRemoteProfileListener(MIDICIProfileList::Listener* listener)
{
    remoteProfileListeners.remove(listener);
    if (remoteProfileListeners.size() == 0)
    {
        unsubscribeAllRemoteProfiles();
    }
}

bool MIDICIDevice::hasLocalOrRemoteMUID(MUID muid) const
{
    if (localInfo.getMUID() == muid)
    {
        return true;
    }

    if (remoteInfos.contains(MIDICIDeviceInfo(muid)))
    {
        return true;
    }

    return false;
}


bool MIDICIDevice::removeRemoteInfo(MUID muid)
{
    int size = remoteInfos.size();
    if (size > 0 && remoteProfileListeners.size() > 0)
    {
        getRemoteInfo(muid).profiles.removeListener(this);
    }

    remoteInfos.removeValue(MIDICIDeviceInfo(muid));
    if (remoteInfos.size() < size)
    {
        LOG("--> removed remote info for MUID %s", MUID2STRING(muid));
        return true;
    }
    return false;
}

bool MIDICIDevice::hasRemoteInfo(MUID muid) const
{
    return remoteInfos.contains(MIDICIDeviceInfo(muid));
}


void MIDICIDevice::addRemoteInfo(const MIDICIDeviceInfo& info)
{
    remoteInfos.add(info);

    if (remoteProfileListeners.size() > 0)
    {
        getRemoteInfo(info.getMUID()).profiles.addListener(this);
    }
}


MIDICIDeviceInfo& MIDICIDevice::getRemoteInfo(MUID muid)
{
    static MIDICIDeviceInfo invalid;
    int index = remoteInfos.indexOf(MIDICIDeviceInfo(muid));
    if (index >= 0)
    {
        return remoteInfos.getReference(index);
    }
    // invalid info
    invalid.setMUID(MIDICI_MUID_INVALID);
    return invalid;
}

const MIDICIDeviceInfo& MIDICIDevice::getRemoteInfo(MUID muid) const
{
    return const_cast<MIDICIDevice*>(this)->getRemoteInfo(muid);
}


MIDICIDeviceInfo* MIDICIDevice::getMostRecentRemoteInfo()
{
    MIDICIDeviceInfo* last = nullptr;
    for (int i = 0; i < remoteInfos.size(); i++)
    {
        MIDICIDeviceInfo& info = remoteInfos.getReference(i);
        if (last == nullptr || (info.lastReceiveTime - last->lastReceiveTime) > 0)
        {
            last = &info;
        }
    }
    return last;
}

const MIDICIDeviceInfo* MIDICIDevice::getMostRecentRemoteInfo() const
{
    return const_cast<MIDICIDevice*>(this)->getMostRecentRemoteInfo();
}



void MIDICIDevice::subscribeAllRemoteProfiles()
{
    for (auto remoteInfo : remoteInfos)
    {
        remoteInfo.profiles.addListener(this);
    }
}

void MIDICIDevice::unsubscribeAllRemoteProfiles()
{
    for (auto remoteInfo : remoteInfos)
    {
        remoteInfo.profiles.removeListener(this);
    }
}

void MIDICIDevice::onProfileAdded(MIDICIProfileList& list, MIDICIProfileState& state)
{
    // pass on to our listeners
    remoteProfileListeners.call([&](Listener& l) { l.onProfileAdded(list, state); });
}

void MIDICIDevice::onProfileRemoved(MIDICIProfileList& list, MIDICIProfileState& state)
{
    // pass on to our listeners
    remoteProfileListeners.call([&](Listener& l) { l.onProfileRemoved(list, state); });
}

void MIDICIDevice::onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel)
{
    // pass on to our listeners
    remoteProfileListeners.call([&](Listener& l) { l.onProfileEnabledChange(list, state, channel); });
}

void MIDICIDevice::onProfileAvailableChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel)
{
    // pass on to our listeners
    remoteProfileListeners.call([&](Listener& l) { l.onProfileAvailableChange(list, state, channel); });
}

void MIDICIDevice::onProfileSpecificDataChange(MIDICIProfileList& list, MIDICIProfileState& state)
{
    // pass on to our listeners
    remoteProfileListeners.call([&](Listener& l) { l.onProfileSpecificDataChange(list, state); });
}

