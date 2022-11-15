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
#include "MIDICIProfileTypes.h"
#include "MIDICIConstants.h"

//
// MIDICIProfileId
//


MIDICIProfileId::MIDICIProfileId()
{
	_data[0] = 0;
	_data[1] = 0;
	_data[2] = 0;
	_data[3] = 0;
	_data[4] = 0;
}

MIDICIProfileId::MIDICIProfileId(const MIDICIProfileId& other)
{
	_data[0] = other._data[0];
	_data[1] = other._data[1];
	_data[2] = other._data[2];
	_data[3] = other._data[3];
	_data[4] = other._data[4];
}

MIDICIProfileId::MIDICIProfileId(const MIDIByte data[MIDICI_PROFILE_ID_SIZE_BYTES])
{
	_data[0] = data[0];
	_data[1] = data[1];
	_data[2] = data[2];
	_data[3] = data[3];
	_data[4] = data[4];
}

MIDICIProfileId::MIDICIProfileId(MIDIByte data1, MIDIByte data2, MIDIByte data3, MIDIByte data4, MIDIByte data5)
{
	_data[0] = data1;
	_data[1] = data2;
	_data[2] = data3;
	_data[3] = data4;
	_data[4] = data5;
}

MIDICIProfileId::MIDICIProfileId(MIDIByte profileBank, MIDIByte profileNumber, MIDIByte profileVersion, MIDIByte profileLevel)
{
	_data[MIDICI_PROFILE_ID_INDEX_ID] = MIDICI_PROFILE_ID_STANDARD_DEFINED;
	_data[MIDICI_PROFILE_ID_INDEX_BANK] = profileBank;
	_data[MIDICI_PROFILE_ID_INDEX_NUMBER] = profileNumber;
	_data[MIDICI_PROFILE_ID_INDEX_VERSION] = profileVersion;
	_data[MIDICI_PROFILE_ID_INDEX_LEVEL] = profileLevel;
}

MIDICIProfileId::MIDICIProfileId(uint32 manufacturerId, MIDIByte info1, MIDIByte info2)
{
	_data[MIDICI_PROFILE_ID_INDEX_MANUID] = (manufacturerId >> 16) & 0x7F;
	_data[MIDICI_PROFILE_ID_INDEX_MANUID + 1] = (manufacturerId >> 8) & 0x7F;
	_data[MIDICI_PROFILE_ID_INDEX_MANUID + 2] = (manufacturerId & 0x7F);
	_data[MIDICI_PROFILE_ID_INDEX_MANU_INFO1] = info1;
	_data[MIDICI_PROFILE_ID_INDEX_MANU_INFO2] = info2;
}


MIDICIProfileId& MIDICIProfileId::operator=(const MIDICIProfileId& f2)
{
	_data[0] = f2._data[0];
	_data[1] = f2._data[1];
	_data[2] = f2._data[2];
	_data[3] = f2._data[3];
	_data[4] = f2._data[4];
	return *this;
}

bool MIDICIProfileId::isValid() const
{
	// one of the first three bytes must be non-zero
	return (_data[0] != 0)
		|| (_data[1] != 0)
		|| (_data[2] != 0);
}

bool MIDICIProfileId::isStandardDefined() const
{
	return (_data[MIDICI_PROFILE_ID_INDEX_ID] == MIDICI_PROFILE_ID_STANDARD_DEFINED);
}

MIDIByte MIDICIProfileId::getProfileBank() const
{
	return _data[MIDICI_PROFILE_ID_INDEX_BANK];
}

MIDIByte MIDICIProfileId::getProfileNumber() const
{
	return _data[MIDICI_PROFILE_ID_INDEX_NUMBER];
}

MIDIByte MIDICIProfileId::getProfileVersion() const
{
	return _data[MIDICI_PROFILE_ID_INDEX_VERSION];
}

MIDIByte MIDICIProfileId::getProfileLevel() const
{
	return _data[MIDICI_PROFILE_ID_INDEX_LEVEL];
}

bool MIDICIProfileId::isManufacturerSpecific() const
{
	return (_data[MIDICI_PROFILE_ID_INDEX_ID] != MIDICI_PROFILE_ID_STANDARD_DEFINED)
		&& isValid();
}

uint32 MIDICIProfileId::getProfileManufacturerId() const
{
	// MSB first!
	return (((uint32)_data[MIDICI_PROFILE_ID_INDEX_MANUID]) << 16)
		| (((uint32)_data[MIDICI_PROFILE_ID_INDEX_MANUID + 1]) << 8)
		| ((uint32)_data[MIDICI_PROFILE_ID_INDEX_MANUID + 2]);
}

MIDIByte MIDICIProfileId::getProfileSpecificInfo1() const
{
	return _data[MIDICI_PROFILE_ID_INDEX_MANU_INFO1];
}

MIDIByte MIDICIProfileId::getProfileSpecificInfo2() const
{
	return _data[MIDICI_PROFILE_ID_INDEX_MANU_INFO2];
}

void MIDICIProfileId::write(MIDIByte* destData, int offset) const
{
	destData += offset;
	for (int i = 0; i < MIDICI_PROFILE_ID_SIZE_BYTES; i++)
	{
		destData[i] = _data[i];
	}
}

bool MIDICIProfileId::operator<(const MIDICIProfileId& f2) const
{
	int count = MIDICI_PROFILE_ID_SIZE_BYTES;
	if (isStandardDefined())
	{
		// the last byte of standard defined profiles is the support level
		count--;
	}
	for (int i = 0; i < count; i++)
	{
		if (_data[i] < f2._data[i])
		{
			return true;
		}
		if (_data[i] > f2._data[i])
		{
			return false;
		}
	}
	return false;
}

bool MIDICIProfileId::operator>(const MIDICIProfileId& f2) const
{
	return !((*this) == f2) && !(f2 < (*this));
}

bool MIDICIProfileId::operator<=(const MIDICIProfileId& f2) const
{
	return ((*this) == f2) || ((*this) < f2);
}

bool MIDICIProfileId::operator>=(const MIDICIProfileId& f2) const
{
	return ((*this) == f2) || ((*this) > f2);
}

bool MIDICIProfileId::operator==(const MIDICIProfileId& f2) const
{
	int count = MIDICI_PROFILE_ID_SIZE_BYTES;
	if (isStandardDefined())
	{
		// the last byte of standard defined profiles is the support level
		count--;
	}
	for (int i = 0; i < count; i++)
	{
		if (_data[i] != f2._data[i])
		{
			return false;
		}
	}
	return true;
}

bool MIDICIProfileId::operator!=(const MIDICIProfileId& f2) const
{
	return !((*this) == f2);
}

String MIDICIProfileId::toString() const
{
	if (isStandardDefined())
	{
		// abuse MIDIMessage's hex dump function...
		return MIDIMessage(&_data[MIDICI_PROFILE_ID_INDEX_BANK], 4).toHex();
	}
	String ret;
	if (_data[MIDICI_PROFILE_ID_INDEX_MANUID + 1] != 0 || _data[MIDICI_PROFILE_ID_INDEX_MANUID + 2] != 0)
	{
		// 3-byte manufacturer ID
		ret = String("Mfr=") + MIDIMessage(&_data[MIDICI_PROFILE_ID_INDEX_MANUID], 3).toHex();
	}
	else
	{
		// 1-byte manufacturer ID
		ret = String("Mfr=") + MIDIMessage(&_data[MIDICI_PROFILE_ID_INDEX_MANUID], 1).toHex();
	}
	return ret + " (" + MIDIMessage(&_data[MIDICI_PROFILE_ID_INDEX_MANU_INFO1], 2).toHex() + ")";
}

String MIDICIProfileId::toRawString() const
{
	return MIDIMessage(_data, MIDICI_PROFILE_ID_SIZE_BYTES).toHex();
}


//
// MIDICIProfileState
//

/** channel number used internally in the available and enabled BitSet32s for MIDICI_CHANNEL_PORT */
#define BITSET_CHANNEL_PORT   (31)


MIDICIProfileState::MIDICIProfileState()
{
	// nothing
}

MIDICIProfileState::MIDICIProfileState(const MIDICIProfileState& other)
	: availableChannels(other.availableChannels)
	, enabledChannels(other.enabledChannels)
	, id(other.id)
	, specificData(other.specificData)
{
	for (auto l : other.listeners.getListeners())
	{
		listeners.add(l);
	}
}

MIDICIProfileState::MIDICIProfileState(const MIDICIProfileId& id_)
	: id(id_)
{
	// nothing
}

MIDICIProfileState::MIDICIProfileState(MIDIByte availableChannel, bool enabled, const MIDICIProfileId& id_)
	: id(id_)
{
	setChannelAvailable(availableChannel, true);
	if (enabled)
	{
		setChannelEnabled(availableChannel, true);
	}
}

MIDICIProfileState::MIDICIProfileState(Listener* listener, MIDIByte availableChannel, bool enabled, const MIDICIProfileId& id_)
	: id(id_)
{
	setChannelAvailable(availableChannel, true);
	if (enabled)
	{
		setChannelEnabled(availableChannel, true);
	}
	if (listener != nullptr)
	{
		listeners.add(listener);
	}
}

MIDICIProfileState& MIDICIProfileState::operator=(const MIDICIProfileState& other)
{
	availableChannels = other.availableChannels;
	enabledChannels = other.enabledChannels;
	id = other.id;
	specificData = other.specificData;
	for (auto l : other.listeners.getListeners())
	{
		listeners.add(l);
	}
	return *this;
}

#define PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel) \
	((channel == MIDICI_CHANNEL_PORT) ? BITSET_CHANNEL_PORT : channel)


int MIDICIProfileState::getAvailableChannelCount() const
{
	int count = 0;
	if (isChannelAvailable(MIDICI_CHANNEL_PORT))
	{
		count++;
	}
	for (MIDIByte i = 0; i < MIDI_CHANNEL_MAX; i++)
	{
		if (isChannelAvailable(i))
		{
			count++;
		}
	}
	return count;
}


bool MIDICIProfileState::isChannelAvailable(MIDIByte channel) const
{
	return availableChannels.isBitSet(PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel));
}


void MIDICIProfileState::setChannelAvailable(MIDIByte channel, bool available)
{
	if (!isValidChannel(channel))
	{
		// nothing to do
		return;
	}
	BitSet32 old = availableChannels;
	if (available)
	{
		availableChannels.setBit(PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel));
	}
	else
	{
		availableChannels.clearBit(PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel));
	}
	if (old != availableChannels)
	{
		listeners.call([&](Listener& l) { l.onProfileAvailableChange(*this, channel); });
	}
}

MIDIByte MIDICIProfileState::getFirstAvailableChannel() const
{
	for (MIDIByte i = 0; i < MIDI_CHANNEL_MAX; i++)
	{
		if (isChannelAvailable(i))
		{
			return i;
		}
	}
	if (isChannelAvailable(MIDICI_CHANNEL_PORT))
	{
		return MIDICI_CHANNEL_PORT;
	}
	return MIDICI_CHANNEL_INVALID;
}


int MIDICIProfileState::getEnabledChannelCount() const
{
	int count = 0;
	if (isChannelEnabled(MIDICI_CHANNEL_PORT))
	{
		count++;
	}
	for (MIDIByte i = 0; i < MIDI_CHANNEL_MAX; i++)
	{
		if (isChannelEnabled(i))
		{
			count++;
		}
	}
	return count;
}

int MIDICIProfileState::getDisabledChannelCount() const
{
	int count = 0;
	if (isChannelAvailable(MIDICI_CHANNEL_PORT) && !isChannelEnabled(MIDICI_CHANNEL_PORT))
	{
		count++;
	}
	for (MIDIByte i = 0; i < MIDI_CHANNEL_MAX; i++)
	{
		if (isChannelAvailable(i) && !isChannelEnabled(i))
		{
			count++;
		}
	}
	return count;
}


bool MIDICIProfileState::isChannelEnabled(MIDIByte channel) const
{
	return enabledChannels.isBitSet(PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel));
}


void MIDICIProfileState::setChannelEnabled(MIDIByte channel, bool enabled)
{
	if (!isValidChannel(channel))
	{
		// nothing to do
		return;
	}
	// any channel that we enable or disable should be available
	setChannelAvailable(channel, true);

	bool previouslyEnabled = isChannelEnabled(channel);

	// can we change this?
	if (previouslyEnabled != enabled)
	{
		listeners.call([&](Listener& l) { l.onProfileCanEnable(*this, channel, enabled); });
		// prevent stale data
		previouslyEnabled = isChannelEnabled(channel);
	}

	if (previouslyEnabled != enabled)
	{
		if (enabled)
		{
			enabledChannels.setBit(PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel));
		}
		else
		{
			enabledChannels.clearBit(PROFILE_CHANNEL_TO_BITSET_CHANNEL(channel));
		}
		listeners.call([&](Listener& l) { l.onProfileEnabledChange(*this, channel); });
	}
}

MIDIByte MIDICIProfileState::getFirstEnabledChannel() const
{
	for (MIDIByte i = 0; i < MIDI_CHANNEL_MAX; i++)
	{
		if (isChannelEnabled(i))
		{
			return i;
		}
	}
	if (isChannelEnabled(MIDICI_CHANNEL_PORT))
	{
		return MIDICI_CHANNEL_PORT;
	}
	return MIDICI_CHANNEL_INVALID;
}


void MIDICIProfileState::setSpecificData(const MIDIByte* data, int size)
{
	specificData.replaceAll(data, (size_t)size);
	listeners.call([&](Listener& l) { l.onProfileSpecificDataChange(*this); });
}

void MIDICIProfileState::setSpecificData(const MemoryBlock& data)
{
	specificData = data;
	listeners.call([&](Listener& l) { l.onProfileSpecificDataChange(*this); });
}

bool MIDICIProfileState::operator<(const MIDICIProfileState& f2) const
{
	return id < f2.id;
}

bool MIDICIProfileState::operator>(const MIDICIProfileState& f2) const
{
	return !((*this) < f2) && !((*this) == f2);
}

bool MIDICIProfileState::operator==(const MIDICIProfileState& f2) const
{
	return id == f2.id;
}

String MIDICIProfileState::toString() const
{
	static String enabled(":enabled ");
	static String disabled(":disabled ");
	String channelString;
	if (isChannelAvailable(MIDICI_CHANNEL_PORT))
	{
		channelString = "entire port";
		if (isChannelEnabled(MIDICI_CHANNEL_PORT))
		{
			channelString += enabled;
		}
		else
		{
			channelString += disabled;
		}
	}

	// individual channels
	int count = 0;
	String ch;
	for (MIDIByte i = 0; i <= MIDI_CHANNEL_MAX; i++)
	{
		if (isChannelAvailable(i))
		{
			ch += String(i + 1);
			if (isChannelEnabled(i))
			{
				ch += enabled;
			}
			else
			{
				ch += disabled;
			}
			count++;
		}
	}
	if (count == 0)
	{
		if (channelString.isEmpty())
		{
			ch = String("no channel");
		}
	}
	else if (count == 1)
	{
		ch = String("channel ") + ch;
	}
	else
	{
		ch = String("channels ") + ch;
	}
	if (channelString.isEmpty())
	{
		channelString = ch;
	}
	else
	{
		channelString += " and " + ch;
	}

	return String("Profile ") + id.toString() + " on " + channelString;
}


bool MIDICIProfileState::isValidChannel(MIDIByte channel)
{
	return (channel == MIDICI_CHANNEL_PORT)
		|| (/*channel >= 0 &&*/ channel <= MIDI_CHANNEL_MAX);
}



//
// MIDICIProfileList
//

MIDICIProfileList::MIDICIProfileList(MUID muid_)
	: muid(muid_)
{
	//nothing
}

MIDICIProfileList::MIDICIProfileList(const MIDICIProfileList& other)
	: muid(other.muid)
	, list(other.list)
{
	for (auto l : other.listeners.getListeners())
	{
		listeners.add(l);
	}
}

MIDICIProfileList& MIDICIProfileList::operator=(const MIDICIProfileList& other)
{
	muid = other.muid;
	list = other.list;
	for (auto l : other.listeners.getListeners())
	{
		listeners.add(l);
	}
	return *this;
}

void MIDICIProfileList::setMUID(MUID muid_)
{
	muid = muid_;
}

int MIDICIProfileList::getProfileCountOnSpecificChannel(MIDIByte channel) const
{
	int count = 0;
	for (int i = 0; i < list.size(); i++)
	{
		if (getProfile(i).isChannelAvailable(channel))
		{
			count++;
		}
	}
	return count;
}

bool MIDICIProfileList::hasProfile(const MIDICIProfileId& id) const
{
	return list.contains(MIDICIProfileState(id));
}

MIDICIProfileState& MIDICIProfileList::getProfile(const MIDICIProfileId& id)
{
	int index = list.indexOf(MIDICIProfileState(id));
	if (index >= 0)
	{
		return list.getReference(index);
	}
	static MIDICIProfileState invalid;
	return invalid;
}

const MIDICIProfileState& MIDICIProfileList::getProfile(const MIDICIProfileId& id) const
{
	return (const_cast<MIDICIProfileList*>(this))->getProfile(id);
}


MIDICIProfileState& MIDICIProfileList::addProfile(const MIDICIProfileId& id)
{
	return addProfile(MIDICI_CHANNEL_INVALID, false, id);
}


MIDICIProfileState& MIDICIProfileList::addProfile(MIDIByte initialChannel, bool enabled, const MIDICIProfileId& id)
{
	MIDICIProfileState state(this, initialChannel, enabled, id);
	int index = list.indexOf(state);
	if (index >= 0)
	{
		MIDICIProfileState& existingState(list.getReference(index));
		existingState.setChannelEnabled(initialChannel, enabled);
		return existingState;
	}
	// otherwise, add it to our list
	list.add(state);
	index = list.indexOf(state);
	MIDICIProfileState& addedState(list.getReference(index));
	listeners.call([&](Listener& l) { l.onProfileAdded(*this, addedState); });
	return addedState;
}


void MIDICIProfileList::removeProfile(const MIDICIProfileId& id)
{
	int index = list.indexOf(MIDICIProfileState(id));
	if (index >= 0)
	{
		// copy MIDICIProfileState object
		MIDICIProfileState state(list[index]);
		list.remove(index);
		listeners.call([&](Listener& l) { l.onProfileRemoved(*this, state); });
	}
}

void MIDICIProfileList::clear()
{
	list.clear();
}

void MIDICIProfileList::onProfileCanEnable(MIDICIProfileState& state, MIDIByte channel, bool& newEnabledState)
{
	// pass on to our listeners
	listeners.call([&](Listener& l) { l.onProfileCanEnable(*this, state, channel, newEnabledState); });
}

void MIDICIProfileList::onProfileEnabledChange(MIDICIProfileState& state, MIDIByte channel)
{
	// pass on to our listeners
	listeners.call([&](Listener& l) { l.onProfileEnabledChange(*this, state, channel); });
}

void MIDICIProfileList::onProfileAvailableChange(MIDICIProfileState& state, MIDIByte channel)
{
	// pass on to our listeners
	listeners.call([&](Listener& l) { l.onProfileAvailableChange(*this, state, channel); });
}

void MIDICIProfileList::onProfileSpecificDataChange(MIDICIProfileState& state)
{
	// pass on to our listeners
	listeners.call([&](Listener& l) { l.onProfileSpecificDataChange(*this, state); });
}

