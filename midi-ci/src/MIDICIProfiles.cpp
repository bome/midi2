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
#include "MIDICIProfiles.h"
#include "MIDICIDevice.h"
#include "MIDICIConstants.h"
#include "Logger.h"


MIDICIProfiles::MIDICIProfiles()
	: MIDICIHandlerBase()
{
	// nothing
}

MIDICIProfiles::~MIDICIProfiles()
{
    stopListening();
}

void MIDICIProfiles::start()
{
    startListening();
}

void MIDICIProfiles::startListening()
{
    if (getDevice() != nullptr)
    {
        getDevice()->getLocalInfo().profiles.addListener(this);
        getDevice()->getLocalInfo().categoriesSupported |= MIDICI_CATEGORY_PROTOCOLNEGOTIATION;
        getDevice()->addRemoteProfileListener(this);
    }
}

void MIDICIProfiles::stop()
{
    stopListening();
}

void MIDICIProfiles::stopListening()
{
    if (getDevice() != nullptr)
    {
        getDevice()->getLocalInfo().profiles.removeListener(this);
        getDevice()->getLocalInfo().categoriesSupported &= ~MIDICI_CATEGORY_PROTOCOLNEGOTIATION;
        getDevice()->removeRemoteProfileListener(this);
    }
}

bool MIDICIProfiles::handleMIDI(const MIDIMessage& message)
{
    if (!MIDICIMessage::isMIDICIMessage(message,
        MIDICI_MESSAGE_TYPE_PROFILES_BEGIN,
        MIDICI_MESSAGE_TYPE_PROFILES_END))
    {
        return false;
    }

    MIDICIMessage ciMessage(message);

    // check channel
    if (!MIDICIProfileState::isValidChannel(ciMessage.getChannel()))
    {
        LOG("RX Profile message 0x%02X from %s: ERROR: channel is invalid=0x%02X", ciMessage.getMessageType(), MUID2STRING(ciMessage.getSourceMUID()), ciMessage.getChannel());
        // send NAK
        return false;
    }

    // Check Source
    // make sure that we have previously discovered the remote device.
    // That will also make sure we're not handling looped back messages from us.
    if (!getDevice()->hasRemoteInfo(ciMessage.getSourceMUID()))
    {
        LOG("RX Profile message 0x%02X: ERROR: comes from unknown remote device: %s", ciMessage.getMessageType(), MUID2STRING(ciMessage.getSourceMUID()));
        // send NAK
        return false;
    }

    // Check Destination
    // If this message is not addressed to us, ignore.
    if (!ciMessage.isAddressedTo(getDevice()->getLocalMUID()))
    {
        // return true so that no NAK is sent
        return true;
    }

    // handle this message
    switch (ciMessage.getMessageType())
    {
    case MIDICI_MESSAGE_TYPE_PROFILES_INQUIRY:
        return handleProfileInquiry(ciMessage);

    case MIDICI_MESSAGE_TYPE_PROFILES_REPLY:
        return handleProfileInquiryReply(ciMessage);

    case MIDICI_MESSAGE_TYPE_PROFILES_SET_ON:
        return handleProfileSetOnOrOff(ciMessage, true/*isOn*/);

    case MIDICI_MESSAGE_TYPE_PROFILES_SET_OFF:
        return handleProfileSetOnOrOff(ciMessage, false/*isOn*/);

    case MIDICI_MESSAGE_TYPE_PROFILES_REPORT_ON:
        return handleProfileReport(ciMessage, true/*isEnabled*/);

    case MIDICI_MESSAGE_TYPE_PROFILES_REPORT_OFF:
        return handleProfileReport(ciMessage, false/*isEnabled*/);

    case MIDICI_MESSAGE_TYPE_PROFILES_SPECIFIC_DATA:
        return handleProfileSpecificData(ciMessage);
    }

    return false;
}

void MIDICIProfiles::triggerProfileInquiry(MUID destination)
{
    sendProfileInquiry(destination);
}


void MIDICIProfiles::onProfileSpecificDataChange(MIDICIProfileList& list, MIDICIProfileState& state)
{
    if (!getDevice()->isLocalMUID(list.getMUID()))
    {
        sendProfileSpecificData(list.getMUID(), state.getId(), state.getSpecificData());
    }
}

void MIDICIProfiles::onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel)
{
    if (getDevice()->isLocalMUID(list.getMUID()))
    {
        sendProfileReport(channel, state.getId(), state.isChannelEnabled(channel));
    }
}


MIDICIProfileId MIDICIProfiles::getId(const MIDICIMessage& message)
{
    if (message.getLength() < MIDICI_COMMON_HEADER_LENGTH + MIDICI_PROFILE_ID_SIZE_BYTES + 1 /*EOX*/)
    {
        LOG("ERROR: message 0x%02X from %s is too short: only %d bytes", message.getMessageType(), MUID2STRING(message.getSourceMUID()), message.getLength());
        return MIDICIProfileId();
    }
    return MIDICIProfileId(message.getData(MIDICI_COMMON_HEADER_LENGTH));
}


//
// receiving
//

bool MIDICIProfiles::handleProfileInquiry(const MIDICIMessage& message)
{
    MUID remoteMUID = message.getSourceMUID();
    LOG("RX Profile Inquiry from %s on %s", MUID2STRING(remoteMUID), CHANNEL2STRING(message.getChannel()));
    
    if (message.getChannel() == MIDICI_CHANNEL_PORT)
    {
        return sendProfileInquiryReply(remoteMUID);
    }
    else
    {
        return sendProfileInquiryReply(message.getChannel(), remoteMUID, true/*alsoSendZeroProfiles*/);
    }
}

bool MIDICIProfiles::handleProfileInquiryReply(const MIDICIMessage& message)
{
    MUID remoteMUID = message.getSourceMUID();
    MIDICIDeviceInfo& remoteInfo = getDevice()->getRemoteInfo(remoteMUID);
    //assert(remoteInfo.isValid());

    // verify size
    int enabledProfilesCountIndex = MIDICI_COMMON_HEADER_LENGTH;
    int enabledProfilesCount = 0;
    int enabledProfilesIndex = 0;
    int disabledProfilesCountIndex = 0;
    int disabledProfilesIndex = 0;
    int disabledProfilesCount = 0;
    if (enabledProfilesCountIndex + 2 <= message.getLength())
    {
        enabledProfilesCount = message.extractNumber14_lsbfirst(enabledProfilesCountIndex);
        enabledProfilesIndex = enabledProfilesCountIndex + 2;
        disabledProfilesCountIndex = enabledProfilesIndex + (enabledProfilesCount * MIDICI_PROFILE_ID_SIZE_BYTES);
        if (disabledProfilesCountIndex + 2 <= message.getLength())
        {
            disabledProfilesCount = message.extractNumber14_lsbfirst(disabledProfilesCountIndex);
            disabledProfilesIndex = disabledProfilesCountIndex + 2;
        }
    }
    if (disabledProfilesIndex == 0
        || (disabledProfilesIndex + (disabledProfilesCount * MIDICI_PROFILE_ID_SIZE_BYTES)) > message.getLength())
    {
        LOG("RX Profile Inquiry Reply from %s: ERROR: %d enabled and %d disabled profiles, but length is only %d bytes.", MUID2STRING(remoteMUID), enabledProfilesCount, disabledProfilesCount, message.getLength());
        return false;
    }

    MIDIByte channel = message.getChannel();
    LOG("RX Profile Inquiry Reply from %s on %s: %d enabled and %d disabled profiles", MUID2STRING(remoteMUID), CHANNEL2STRING(channel), enabledProfilesCount, disabledProfilesCount);

    // now add the profiles, one by one

    // enabled profiles
    for (int i = 0; i < enabledProfilesCount; i++)
    {
        MIDICIProfileState& state = remoteInfo.profiles.addProfile(channel, true/*enabled*/, MIDICIProfileId(message.getData(enabledProfilesIndex)));
        LOG("  %d: enabled '%s'", i + 1, state.getId().toString().toRawUTF8());
        enabledProfilesIndex += MIDICI_PROFILE_ID_SIZE_BYTES;
    }
    // disabled profiles
    for (int i = 0; i < disabledProfilesCount; i++)
    {
        MIDICIProfileState& state = remoteInfo.profiles.addProfile(channel, false/*enabled*/, MIDICIProfileId(message.getData(disabledProfilesIndex)));
        LOG("  %d: disabled '%s'", i + 1, state.getId().toString().toRawUTF8());
        disabledProfilesIndex += MIDICI_PROFILE_ID_SIZE_BYTES;
    }

    return true;
}

bool MIDICIProfiles::handleProfileSetOnOrOff(const MIDICIMessage& message, bool isOn)
{
    // request from remote to set our own profile on or off
    MIDICIProfileId id = getId(message);
    if (!id.isValid())
    {
        return false;
    }

    if (!getDevice()->getLocalInfo().profiles.hasProfile(id))
    {
        LOG("RX Set Profile %s from %s: ERROR: profile is not available: %s", isOn ? "On" : "Off", MUID2STRING(message.getSourceMUID()), id.toString().toRawUTF8());
        // reply with NAK
        return false;
    }

    LOG("RX Set Profile %s '%s' from %s ", isOn ? "On" : "Off", id.toString().toRawUTF8(), MUID2STRING(message.getSourceMUID()));

    // change the state. If anything gets changed, a listener callback 
    // will send out the respective report messages.
    MIDICIProfileState& profile(getDevice()->getLocalInfo().profiles.getProfile(id));
    MIDIByte channel = message.getChannel();
    profile.setChannelEnabled(channel, isOn);
    if (profile.isChannelEnabled(channel) != isOn)
    {
        // that didn't work! Send the opposite message
        sendProfileReport(channel, id, !isOn);
    }

	return true;
}

bool MIDICIProfiles::handleProfileReport(const MIDICIMessage& message, bool isEnabled)
{
    // report from remote that their profile is enabled or not
    MIDICIProfileId id = getId(message);
    if (!id.isValid())
    {
        return false;
    }
    MUID remoteMUID = message.getSourceMUID();
    MIDICIDeviceInfo& remoteInfo = getDevice()->getRemoteInfo(remoteMUID);
    //assert(remoteInfo.isValid());

    LOG("RX Profile %s Report '%s' from %s", isEnabled ? "Enabled" : "Disabled", id.toString().toRawUTF8(), MUID2STRING(message.getSourceMUID()));

    remoteInfo.profiles.getProfile(id).setChannelEnabled(message.getChannel(), isEnabled);

    return true;
}

bool MIDICIProfiles::handleProfileSpecificData(const MIDICIMessage& message)
{
    // request to set specific data to a local profile
    MIDICIProfileId id = getId(message);
    if (!id.isValid())
    {
        return false;
    }

    if (!getDevice()->getLocalInfo().profiles.hasProfile(id))
    {
        LOG("RX Profile Specific Data Message from %s: ERROR: profile is not available: '%s'", MUID2STRING(message.getSourceMUID()), id.toString().toRawUTF8());
        // reply with NAK
        return false;
    }


    int index = MIDICI_COMMON_HEADER_LENGTH + MIDICI_PROFILE_ID_SIZE_BYTES;
    if (message.getLength() >= index + 4 + 1/*EOX*/)
    {
        int size = (int)message.extractNumber28_lsbfirst(index);
        index += 4;
        if (message.getLength() >= index + size + 1/*EOX*/)
        {
            LOG("RX Profile Specific Data Message '%s' from %s: %d bytes", id.toString().toRawUTF8(), MUID2STRING(message.getSourceMUID()), message.getLength());
            getDevice()->getLocalInfo().profiles.getProfile(id).setSpecificData(message.getData(index), size);
            return true;
        }
    }
    
    LOG("RX Profile Specific Data Message from %s: ERROR: message is too short: only %d bytes", MUID2STRING(message.getSourceMUID()), message.getLength());
    return false;
}

//
// sending
//

bool MIDICIProfiles::sendMessageWithOneProfile(MIDIByte messageType, MIDIByte channel, MUID destination, const MIDICIProfileId& id)
{
    MIDIByte msg[MIDICI_COMMON_HEADER_LENGTH + MIDICI_PROFILE_ID_SIZE_BYTES + 1/*EOX*/];
    MIDICIMessage message(msg, sizeof(msg));

    message.fillHeader(messageType, getDevice()->getLocalMUID(), destination);
    message.setChannel(channel);
    id.write(msg, MIDICI_COMMON_HEADER_LENGTH);
    return sendMIDI(message);
}

bool MIDICIProfiles::sendProfileInquiry(MUID destination)
{
    MIDIByte msg[MIDICI_COMMON_HEADER_LENGTH + 1/*EOX*/];
    MIDICIMessage message(msg, sizeof(msg));

    message.fillHeader(MIDICI_MESSAGE_TYPE_PROFILES_INQUIRY, getDevice()->getLocalMUID(), destination);
    LOG("TX Profile Inquiry to %s", MUID2STRING(destination));
    return sendMIDI(message);
}

bool MIDICIProfiles::sendProfileInquiryReply(MUID destination)
{
    // first send possible replies for individual channels
    for (MIDIByte i = 0; i <= MIDI_CHANNEL_MAX; i++)
    {
        bool ok = sendProfileInquiryReply(i, destination, false /*alsoSendZeroProfiles*/);
        if (!ok)
        {
            return false;
        }
    }
    // then, to finalize, send the reply for the CHANNEL_PORT address
    return sendProfileInquiryReply(MIDICI_CHANNEL_PORT, destination, true /*alsoSendZeroProfiles*/);
}



// for easy message allocation, restrict the number of profiles
#define MIDICI_PROFILES_INQUIRY_REPLY_MAX_PROFILES  (100)
#define MIDICI_PROFILES_INQUIRY_REPLY_MAX_MSG_SIZE  \
              MIDICI_COMMON_HEADER_LENGTH           \
            + 2 + 2 /*size fields*/                 \
            + (MIDICI_PROFILES_INQUIRY_REPLY_MAX_PROFILES * MIDICI_PROFILE_ID_SIZE_BYTES)


bool MIDICIProfiles::sendProfileInquiryReply(MIDIByte channel, MUID destination, bool alsoSendZeroProfiles)
{
    const MIDICIProfileList& profiles = getDevice()->getLocalInfo().profiles;

    // calculate required size
    int commonSize = MIDICI_COMMON_HEADER_LENGTH + 2 + 2 /* size fields */ + 1 /*EOX*/;
    int profileCount = profiles.getProfileCountOnSpecificChannel(channel);

    if (profileCount == 0 && !alsoSendZeroProfiles)
    {
        // no error
        return true;
    }

    int size = commonSize + (profileCount * MIDICI_PROFILE_ID_SIZE_BYTES);

    int maxSize = MIDICI_PROFILES_INQUIRY_REPLY_MAX_MSG_SIZE;

    MIDICIDeviceInfo& remoteInfo = getDevice()->getRemoteInfo(destination);
    if (remoteInfo.isValid() && remoteInfo.maxSysExSize < maxSize)
    {
        maxSize = remoteInfo.maxSysExSize;
    }
    if (size > maxSize)
    {
        int adaptedProfileCount = (maxSize - (MIDICI_COMMON_HEADER_LENGTH + 2 + 2 + 1)) / MIDICI_PROFILE_ID_SIZE_BYTES;
        LOG("TX Profile Inquiry Reply: WARNING: max SysEx size (%d bytes) restricts available number of profiles (%d) to %d", maxSize, profileCount, adaptedProfileCount);
        profileCount = adaptedProfileCount;
        size = commonSize + (profileCount * MIDICI_PROFILE_ID_SIZE_BYTES);
    }

    MIDIByte msg[MIDICI_PROFILES_INQUIRY_REPLY_MAX_MSG_SIZE];
    MIDICIMessage message(msg, size);
    message.fillHeader(MIDICI_MESSAGE_TYPE_PROFILES_REPLY, getDevice()->getLocalMUID(), destination);
    message.setChannel(channel);

    LOG("TX Profile Inquiry Reply to %s on %s:", MUID2STRING(destination), CHANNEL2STRING(channel));

    // enabled profiles
    int count = 0;
    int countIndex = MIDICI_COMMON_HEADER_LENGTH;
    int index = countIndex + 2;
    for (int i = 0; i < profiles.getProfileCount(); i++)
    {
        const MIDICIProfileState& profile = profiles.getProfile(i);
        if (profile.isChannelEnabled(channel))
        {
            if (count >= profileCount)
            {
                break;
            }
            profile.getId().write(msg, index);
            index += MIDICI_PROFILE_ID_SIZE_BYTES;
            count++;
            LOG("  %d: enabled '%s'", count, profile.getId().toString().toRawUTF8());
        }
    }
    message.writeNumber14_lsbfirst((uint16)count, countIndex);
    int enabledCount = count;

    // available profiles
    count = 0;
    countIndex = index;
    index = countIndex + 2;
    for (int i = 0; i < profiles.getProfileCount(); i++)
    {
        const MIDICIProfileState& profile = profiles.getProfile(i);
        if (profile.isChannelAvailable(channel) && !profile.isChannelEnabled(channel))
        {
            if (count + enabledCount >= profileCount)
            {
                break;
            }
            profile.getId().write(msg, index);
            index += MIDICI_PROFILE_ID_SIZE_BYTES;
            count++;
            LOG("  %d: disabled '%s'", count, profile.getId().toString().toRawUTF8());
        }
    }
    message.writeNumber14_lsbfirst((uint16)count, countIndex);
    int disabledCount = count;

    jassert((index + 1) == size);

    LOG("  %s: enabled=%d  disabled=%d", CHANNEL2STRING(channel), enabledCount, disabledCount);

    return sendMIDI(message);
}


bool MIDICIProfiles::sendProfileSetOnOrOff(MIDIByte channel, MUID destination, const MIDICIProfileId& id, bool isOn)
{
    LOG("TX Profile Set %s '%s' on %s to %s", isOn ? "On" : "Off", id.toString().toRawUTF8(), CHANNEL2STRING(channel), MUID2STRING(destination));
    MIDIByte messageType = isOn ? MIDICI_MESSAGE_TYPE_PROFILES_SET_ON : MIDICI_MESSAGE_TYPE_PROFILES_SET_OFF;
    return sendMessageWithOneProfile(messageType, channel, destination, id);
}


bool MIDICIProfiles::sendProfileReport(MIDIByte channel, const MIDICIProfileId& id, bool isEnabled)
{
    LOG("TX Profile %s Report '%s' on %s", isEnabled ? "Enabled" : "Disabled", id.toString().toRawUTF8(), CHANNEL2STRING(channel));

    MIDIByte messageType = isEnabled ? MIDICI_MESSAGE_TYPE_PROFILES_REPORT_ON : MIDICI_MESSAGE_TYPE_PROFILES_REPORT_OFF;
    return sendMessageWithOneProfile(messageType, channel, MIDICI_MUID_BROADCAST, id);
}


bool MIDICIProfiles::sendProfileSpecificData(MUID destination, const MIDICIProfileId& id, const MemoryBlock& data)
{
    int size = MIDICI_COMMON_HEADER_LENGTH + MIDICI_PROFILE_ID_SIZE_BYTES + 4 + (int)data.getSize() + 1/*EOX*/;

    MIDICIDeviceInfo& remoteInfo = getDevice()->getRemoteInfo(destination);
    if (remoteInfo.isValid() && remoteInfo.maxSysExSize < size)
    {
        LOG("TX Profile Specific Data: ERROR: '%s' requested to send %d bytes to %s, but remote device %s only support max sys ex size of %d bytes.", id.toString().toRawUTF8(), data.getSize(), MUID2STRING(destination), remoteInfo.maxSysExSize);
        return false;
    }

    // play it safe, and allocate the required data
    MemoryBlock msg((size_t)size);

    MIDICIMessage message((MIDIByte*)msg.getData(), size);
    message.fillHeader(MIDICI_MESSAGE_TYPE_PROFILES_SPECIFIC_DATA, getDevice()->getLocalMUID(), destination);
    int index = MIDICI_COMMON_HEADER_LENGTH;
    id.write(message.getData(), index);
    index += MIDICI_PROFILE_ID_SIZE_BYTES;
    message.writeNumber28_lsbfirst((int)data.getSize(), index);
    index += 4;
    data.copyTo(message.getData(index), 0, data.getSize());
    
    LOG("TX Profile Specific Data: '%s' send %d bytes to %s", id.toString().toRawUTF8(), data.getSize(), MUID2STRING(destination));
    return sendMIDI(message);
}
