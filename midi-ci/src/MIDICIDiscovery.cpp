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
#include "MIDICIDiscovery.h"
#include "MIDICIDevice.h"
#include "Logger.h"


MIDICIDiscovery::MIDICIDiscovery()
{
    // nothing
}

void MIDICIDiscovery::start()
{
    getDevice()->setLocalMUID(nextRandomMUID());
    LOG(" local MUID:%s", MUID2STRING(getDevice()->getLocalMUID()));
}

void MIDICIDiscovery::stop()
{
    if (getDevice() != nullptr && getDevice()->hasSentMessages())
    {
        sendInvalidateMUID(getDevice()->getLocalMUID());
        // give some time to actually send the message
        Thread::sleep(20);
    }
}


bool MIDICIDiscovery::handleMIDI(const MIDIMessage& message)
{
    if (!MIDICIMessage::isMIDICIMessage(message,
        MIDICI_MESSAGE_TYPE_MANAGEMENT_BEGIN,
        MIDICI_MESSAGE_TYPE_MANAGEMENT_END))
    {
        return false;
    }

    MIDICIMessage ciMessage(message);

    // if this message is not addressed to us, ignore
    if (!ciMessage.isAddressedTo(getDevice()->getLocalMUID()))
    {
        return false;
    }

    // if this message comes from us, it's either a loopback from us, or a MUID collision
    if (ciMessage.getSourceMUID() == getDevice()->getLocalMUID())
    {
        // if this is a discovery message, we do additional collision handling
        if (ciMessage.getMessageType() != MIDICI_MESSAGE_DISCOVERY)
        {
            // return true --> assume it originates from us... and ignore.
            return true;
        }
        if (!handleCollision(ciMessage))
        {
            // we don't respond to the message because we've sent an InvalidateMUID message.
            // but return true to not trigger a NAK
            return true;
        }
    }

    // finally handle this message
    switch (ciMessage.getMessageType())
    {
    case MIDICI_MESSAGE_DISCOVERY:
        return handleDiscovery(ciMessage, false/*isReply*/);

    case MIDICI_MESSAGE_DISCOVERY_REPLY:
        return handleDiscovery(ciMessage, true/*isReply*/);

    case MIDICI_MESSAGE_INVALIDATE_MUID:
        return handleInvalidateMUID(ciMessage);

    case MIDICI_MESSAGE_NAK:
        return handleNAK(ciMessage);
    }

    return false;
}


void MIDICIDiscovery::triggerDiscovery()
{
    sendDiscovery(false/*isReply*/);
}


void MIDICIDiscovery::generateNewRandomMUID(bool canSendInvalidateMUID /*= true*/)
{
    MUID oldMUID = getDevice()->getLocalMUID();
    getDevice()->setLocalMUID(nextRandomMUID());
    LOG("  generated new MUID=%s", MUID2STRING(getDevice()->getLocalMUID()));

    if (canSendInvalidateMUID && getDevice()->hasSentMessages())
    {
        sendInvalidateMUID(oldMUID);
    }
    getDevice()->resetSentMessagesFlag();
}


MUID MIDICIDiscovery::nextRandomMUID() const
{
    MUID newMUID;
    Random rng(Time::currentTimeMillis());
    while (true)
    {
        newMUID = (((uint32)(rng.nextInt64() & 0xFFFFFFFF)) % (MIDICI_MUID_MAX_ASSIGNABLE_VALUE + 1));
        if (!getDevice()->hasLocalOrRemoteMUID(newMUID))
        {
            break;
        }
    }
    return newMUID;
}

bool MIDICIDiscovery::handleCollision(const MIDICIMessage& message)
{
    // we have a real collision
    MUID collidedMUID = message.getSourceMUID();
    // invalidate current MUID
    bool hasAlreadyEngagedInIO = getDevice()->hasSentMessages();
    if (hasAlreadyEngagedInIO)
    {
        LOG("RX Discovery: MUID collision! respond with InvalidateMUID %s", MUID2STRING(collidedMUID));
    }
    else
    {
        // how convenient, we have not published our MUID. We can just quietly change it and respond normally with the modified MUID.
        LOG("RX Discovery: MUID collision! old MUID not published, no need to invalidate MUID.");
    }
    generateNewRandomMUID();

    // if this was a real collision, we cannot continue
    return !hasAlreadyEngagedInIO;
}

void MIDICIDiscovery::logDiscoveryMessage(bool isTX, const MIDICIMessage& discoveryMessage, bool isReply, const MIDICIDeviceInfo& info)
{
    LOG("%s Discovery%s from %s to %s:",
        isTX ? "TX" : "RX",
        isReply ? "Reply" : "",
        MUID2STRING(discoveryMessage.getSourceMUID()),
        MUID2STRING(discoveryMessage.getDestinationMUID()));
    if ((info.manufacturerId & 0xFFFF) != 0)
    {
        // 3-byte manufacturer ID
        LOG("  manufacturer=0x%06X", info.manufacturerId);
    }
    else
    {
        // 1-byte manufacturer ID
        LOG("  manufacturer=0x%02X", (info.manufacturerId >> 16));
    }
    LOG("  family,model=0x%04X,0x%04X", info.familyId, info.modelId);
    LOG("  version=0x%08X", info.versionId);
    LOG("  categories=0x%02X", info.categoriesSupported);
    LOG("  maxSysEx=%d bytes", info.maxSysExSize);
}


bool MIDICIDiscovery::handleDiscovery(const MIDICIMessage& message, bool isReply)
{
    if (message.getLength() < MIDICI_MESSAGE_DISCOVERY_SIZE)
    {
        LOG("RX corrupt Discovery%s with len=%d --> NAK", isReply ? "Reply" : "", message.getLength());
        return getDevice()->sendNAK(message);
    }

    MIDICIDeviceInfo remoteInfo(message.getSourceMUID());

    const MIDIByte* data = message.getData();
    int index = MIDICI_COMMON_HEADER_LENGTH;
    remoteInfo.manufacturerId = message.extractNumber24_msbfirst(index);
    index += 3;
    remoteInfo.familyId = message.extractNumber16_lsbfirst(index);
    index += 2;
    remoteInfo.modelId = message.extractNumber16_lsbfirst(index);
    index += 2;
    remoteInfo.versionId = message.extractNumber32_lsbfirst(index);
    index += 4;
    remoteInfo.categoriesSupported = data[index];
    index++;
    remoteInfo.maxSysExSize = message.extractNumber28_lsbfirst(index);
    //index += 4;

    logDiscoveryMessage(false/*isTX*/, message, isReply, remoteInfo);

    if (remoteInfo.maxSysExSize < MIDICI_MINIMUM_RECEIVABLE_SYSEX_SIZE)
    {
        LOG("WARNING: remote device reports %d bytes receivable Sys Ex size, which is invalid. Assuming the minimum of %d bytes", remoteInfo.maxSysExSize, MIDICI_MINIMUM_RECEIVABLE_SYSEX_SIZE);
        remoteInfo.maxSysExSize = MIDICI_MINIMUM_RECEIVABLE_SYSEX_SIZE;
    }

    // add or overwrite
    getDevice()->addRemoteInfo(remoteInfo);

    if (!isReply)
    {
        sendDiscovery(true/*isReply*/, remoteInfo.getMUID());
    }

    return true;
}


bool MIDICIDiscovery::handleInvalidateMUID(const MIDICIMessage& message)
{
    if (message.getLength() < MIDICI_MESSAGE_INVALIDATEMUID_SIZE)
    {
        LOG("RX InvalidateMUID with invalid len=%d", message.getLength());
        getDevice()->sendNAK(message);
        return true;
    }

    MUID muidToBeInvalidated = (MUID)message.extractNumber28_lsbfirst(MIDICI_COMMON_HEADER_LENGTH);

    bool isLocalMUID = (muidToBeInvalidated == getDevice()->getLocalMUID());

    if (isLocalMUID)
    {
        // oh! our own MUID gets invalidated!
        LOG("RX InvalidateMUID: %s --> this device's MUID got invalidated.", MUID2STRING(muidToBeInvalidated));
        generateNewRandomMUID(false /*canSendInvalidateMUID*/);
    }
    else
    {
        // maybe it's one of the cached remote devices
        LOG("RX InvalidateMUID: %s", MUID2STRING(muidToBeInvalidated));
        getDevice()->removeRemoteInfo(muidToBeInvalidated);
    }

    return true;
}

bool MIDICIDiscovery::handleNAK(const MIDICIMessage& message)
{
    // cannot really do anything!
    // The spec says: "Receiver response to a NAK message is undefined."
    LOG("RX NAK: %s (ignored)", MUID2STRING(message.getSourceMUID()));
    return true;
}


bool MIDICIDiscovery::sendInvalidateMUID(MUID muidToInvalidate)
{
    MIDIByte msg[MIDICI_MESSAGE_INVALIDATEMUID_SIZE];
    MIDICIMessage message(msg, MIDICI_MESSAGE_INVALIDATEMUID_SIZE);

    message.fillHeader(MIDICI_MESSAGE_INVALIDATE_MUID, getDevice()->getLocalMUID(), MIDICI_MUID_BROADCAST);
    message.writeNumber28_lsbfirst(muidToInvalidate, MIDICI_COMMON_HEADER_LENGTH);
    LOG("TX InvalidateMUID (broadcast) invalidMUID: %s", MUID2STRING(muidToInvalidate));
    return sendMIDI(message);
}


bool MIDICIDiscovery::sendDiscovery(bool isReply, MUID destination /*= MIDICI_MUID_BROADCAST*/)
{
    const MIDICIDeviceInfo& info = getDevice()->getLocalInfo();
    if (!info.isValid())
    {
        LOG("ERROR: cannot send Discovery%s to %s: local info is not valid.", isReply ? "Reply" : "", MUID2STRING(destination));
        return false;
    }

    MIDIByte msg[MIDICI_MESSAGE_DISCOVERY_SIZE];
    MIDICIMessage message(msg, MIDICI_MESSAGE_DISCOVERY_SIZE);
    MIDIByte messageType = isReply ? MIDICI_MESSAGE_DISCOVERY_REPLY : MIDICI_MESSAGE_DISCOVERY;
    message.fillHeader(messageType, getDevice()->getLocalMUID(), destination);

    int index = MIDICI_COMMON_HEADER_LENGTH;
    message.writeNumber24_msbfirst(info.manufacturerId, index);
    index += 3;
    message.writeNumber16_lsbfirst(info.familyId, index);
    index += 2;
    message.writeNumber16_lsbfirst(info.modelId, index);
    index += 2;
    message.writeNumber32_lsbfirst(info.versionId, index);
    index += 4;
    message.getData()[index] = info.categoriesSupported;
    index++;
    message.writeNumber28_lsbfirst(info.maxSysExSize, index);

    logDiscoveryMessage(true/*isTX*/, message, isReply, getDevice()->getLocalInfo());

    return sendMIDI(message);
}
