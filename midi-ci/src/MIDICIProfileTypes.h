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
#include "MIDICIConstants.h"

/*
Architectural problems with the current solution:
1) Because we use a SortedSet, instances of MIDICIProfileId must be allowed to be copied, 
   i.e. the = operator overridden. That conflicts with the invariant nature.
2) For the same reason, instances of MIDICIProfileState and MIDICIProfileList are getting copied,
   including their listeners.
   This is just asking for problems when the instance that you added yourself as listener might not 
   the instance from which you have to remove yourself when you're getting deleted. Here, it works
   because you should always use an accessor to retrieve the instance for removing yourself as listener.
3) The encapsulation of the Profiles implementation is minimal: all data is stored in fields belonging
   to a MIDICIDevice instance, no matter if Profiles are used or not. Only the relevant MIDI-CI
   i/o implementation is cleanly separated in form of a handler. The problem to solve here is that
   maybe other handlers must access Profiles and listen to Profile changes. So we cannot just put all
   Profile handling into the Profiles class.
4) Nicer would be usage of ValueTree for the MIDIDeviceInfo information, especially all MIDICIProfileState
   fields. But that would require a lot of unintitive type casting.

Ideas:
- singleton class to map Profile Id to profile name -- especially for Standard Profiles, and vice versa.
*/

//
// MIDICIProfileId
//

#define PROFILEID2STRING(id)  id.toRawString().toRawUTF8()


/** A container for one MIDI-CI profile ID (5 bytes) */
class MIDICIProfileId
{
public:
	/** create an invalid Profile ID */
	MIDICIProfileId();
	MIDICIProfileId(const MIDICIProfileId& other);
	/** create a Profile ID from the given byte array */
	MIDICIProfileId(const MIDIByte data[MIDICI_PROFILE_ID_SIZE_BYTES]);
	/** create a Profile ID given the 5 bytes */
	MIDICIProfileId(MIDIByte data1, MIDIByte data2, MIDIByte data3, MIDIByte info1, MIDIByte info2);
	/** create a standard defined Profile ID */
	MIDICIProfileId(MIDIByte profileBank, MIDIByte profileNumber, MIDIByte profileVersion, MIDIByte profileLevel);
	/** create a manufacturer-specific Profile ID */
	MIDICIProfileId(uint32 manufacturerId, MIDIByte data4, MIDIByte data5);

	MIDICIProfileId& operator=(const MIDICIProfileId& f2);

	bool isValid() const;

	/** @return true if this is a standard defined Profile ID defined by the MIDI Association */
	bool isStandardDefined() const;
	/** For standard-defined Profile IDs, return the bank */
	MIDIByte getProfileBank() const;
	/** For standard-defined Profile IDs, return the number */
	MIDIByte getProfileNumber() const;
	/** For standard-defined Profile IDs, return the version */
	MIDIByte getProfileVersion() const;
	/** For standard-defined Profile IDs, return the level */
	MIDIByte getProfileLevel() const;

	/** @return true if this is a manufacturer specific Profile ID */
	bool isManufacturerSpecific() const;
	/** For manufacturer specific Profile IDs, return the manufacturer ID */
	uint32 getProfileManufacturerId() const;
	/** For manufacturer specific Profile IDs, return the info1 byte */
	MIDIByte getProfileSpecificInfo1() const;
	/** For manufacturer specific Profile IDs, return the info2 byte */
	MIDIByte getProfileSpecificInfo2() const;

	/** r/o access to the raw data of this Profile ID */
	const MIDIByte* getData() const { return _data; }

	/** write the 5 profile bytes to the given array at the given offset */
	void write(MIDIByte* destData, int offset = 0) const;

	bool operator<(const MIDICIProfileId& f2) const;
	bool operator>(const MIDICIProfileId& f2) const;
	bool operator<=(const MIDICIProfileId& f2) const;
	bool operator>=(const MIDICIProfileId& f2) const;
	bool operator==(const MIDICIProfileId& f2) const;
	bool operator!=(const MIDICIProfileId& f2) const;

	String toString() const;
	String toRawString() const;

private:
	MIDIByte _data[MIDICI_PROFILE_ID_SIZE_BYTES];
};


//
// BitSet32
//

/** a class for accessing individual bits in a 32-bit uint */
class BitSet32
{
public:
	BitSet32()
		: data(0) {}
	BitSet32(uint32 bits)
		: data(bits) {}
	BitSet32(const BitSet32& other)
		: data(other.data) {}
	BitSet32& operator=(const BitSet32& other)
		{ data = other.data; return *this; }
	BitSet32& operator=(const uint32& other)
		{ data = other; return *this; }

	void clear()
		{ data = 0; }
	bool isBitSet(int bit) const
		{ return (data & (1 << bit)) != 0; }
	void setBit(int bit)
		{ data |= (1 << bit); }
	void clearBit(int bit)
		{ data &= ~((uint32)(1 << bit)); }

	bool operator>(const BitSet32& other) const
		{ return data > other.data; }
	bool operator<(const BitSet32& other) const
		{ return data < other.data; }
	bool operator>=(const BitSet32& other) const
		{ return data >= other.data; }
	bool operator<=(const BitSet32& other) const
		{ return data <= other.data; }
	bool operator==(const BitSet32& other) const
		{ return data == other.data; }
	bool operator!=(const BitSet32& other) const
		{ return data != other.data; }

private:
	uint32 data;
};


//
// MIDICIProfileState
//

/**
 * A current state of a Profile
 * 
 * Channel handling:
 * A profile can be available on any combination of the 16 MIDI channels and/or on the port (MIDICI_CHANNEL_PORT).
 * If your profile is available on all channels, just set MIDICI_CHANNEL_PORT, and it will only be reported to
 * remote devices on the port (not a specific channel).
 * If your profile is not available on all channels, mark the available channels and not MIDICI_CHANNEL_PORT.
 * Certain profiles might have different semantics for enabling it on the port rather than on a specific channel.
 * For those profiles, it might make sense to mark them available on certain (or all) channels, and additionally
 * on MIDICI_CHANNEL_PORT.
 * 
 * Calling setChannelEnabled(...) will always mark that channel as available.
 */
class MIDICIProfileState
{
public:
	class Listener;
	
	/** create an invalid MIDICIProfileState */
	MIDICIProfileState();

	/** copy constructor */
	MIDICIProfileState(const MIDICIProfileState& other);

	/** Create a profile state with no available channels */
	MIDICIProfileState(const MIDICIProfileId& id_);

	/**
	 * Create a state with the given ID and one available channel.
	 * @param availableChannel - the initial available channel: 0..15, or MIDICI_CHANNEL_PORT for the entire port, or MIDICI_CHANNEL_INVALID to not set an initial channel
	 * @param enabled - if true, the given channel is initialized as being available and enabled, otherwise only available.
	 */
	MIDICIProfileState(MIDIByte availableChannel, bool enabled, const MIDICIProfileId& id_);

	/**
	 * Create a state with the given listener, given ID and one available channel.
	 * The listener will not be notified from the constructor.
	 * @param availableChannel - the initial available channel: 0..15, or MIDICI_CHANNEL_PORT for the entire port, or MIDICI_CHANNEL_INVALID to not set an initial channel
	 * @param enabled - if true, the given channel is initialized as being available and enabled, otherwise only available.
	 */
	MIDICIProfileState(Listener* listener, MIDIByte availableChannel, bool enabled, const MIDICIProfileId& id_);

	MIDICIProfileState& operator=(const MIDICIProfileState& other);

	bool isValid() const { return id.isValid(); }

	const MIDICIProfileId& getId() const { return id; }


	// -------- available channels ---------

	/** @return the number of distinct available channels: if MIDICI_CHANNEL_PORT is available, it counts as one */
	int getAvailableChannelCount() const;

	/**
	 * @param channel - either a specific channel 0..15, or MIDICI_CHANNEL_PORT for the entire port.
	 * @return true if that given channel is marked available
	 */
	bool isChannelAvailable(MIDIByte channel) const;

	/** Add (available==true) or remove (available==false) the given channel from the availableChannels set */
	void setChannelAvailable(MIDIByte channel, bool available);

	/** @return the first available MIDI channel (0..15) or, if available, MIDICI_CHANNEL_PORT, or if none available, MIDICI_CHANNEL_INVALID */
	MIDIByte getFirstAvailableChannel() const;


	// -------- enabled channels ---------

	/** @return the number of distinct enabled channels: if MIDICI_CHANNEL_PORT is available, it counts as one */
	int getEnabledChannelCount() const;

	/**
	 * @param channel - either a specific channel 0..15, or MIDICI_CHANNEL_PORT for the entire port.
	 * @return true if the given channel is marked as enabled
	 */
	bool isChannelEnabled(MIDIByte channel) const;

	/**
	 * Add (enabled==true) or remove (enabled==false) the given channel from the enabledChannels set.
	 * If the given channel is not yet in the availableChannels set, it is added to it, too.
	 */
	void setChannelEnabled(MIDIByte channel, bool enabled);

	/** @return the first available MIDI channel (0..15) or, if available, MIDICI_CHANNEL_PORT, or if none available, MIDICI_CHANNEL_INVALID */
	MIDIByte getFirstEnabledChannel() const;


	// -------- disabled channels ---------

	/** @return the number of distinct disabled channels: if MIDICI_CHANNEL_PORT is available, it counts as one */
	int getDisabledChannelCount() const;


	// -------- profile specific data ---------

	bool hasSpecificData() const { return specificData.getSize() > 0; }
	const MemoryBlock& getSpecificData() const { return specificData; }
	void setSpecificData(const MIDIByte* data, int size);
	void setSpecificData(const MemoryBlock& data);


	class Listener
	{
	public:
		virtual ~Listener() {}

		/**
		 * This callback is called right before enabling or disabling a profile on the given channel.
		 * The listener can flip the newEnabledState variable to prevent changing the enabled state
		 * (if not possible for some reason), or it can disable other mutually exclusive channels or profiles
		 */
		virtual void onProfileCanEnable(MIDICIProfileState& /*state*/, MIDIByte /*channel*/, /*IN OUT*/bool& /*newEnabledState*/ ) { /*optional*/ };
		virtual void onProfileEnabledChange(MIDICIProfileState& state, MIDIByte channel) = 0;
		virtual void onProfileAvailableChange(MIDICIProfileState& /*state*/, MIDIByte /*channel*/) { /*optional*/ };
		virtual void onProfileSpecificDataChange(MIDICIProfileState& /*state*/) { /*optional*/ };
	};

	void addListener(Listener* listener) { listeners.add(listener); }
	void removeListener(Listener* listener) { listeners.remove(listener); }

	// the primary key of this class is the profile ID
	bool operator<(const MIDICIProfileState& f2) const;
	bool operator>(const MIDICIProfileState& f2) const;
	bool operator<=(const MIDICIProfileState& f2) const { return ((*this) == f2) || ((*this) < f2); }
	bool operator>=(const MIDICIProfileState& f2) const { return ((*this) == f2) || ((*this) > f2); }
	bool operator==(const MIDICIProfileState& f2) const;
	bool operator!=(const MIDICIProfileState& f2) const { return !((*this) == f2); }

	static bool isValidChannel(MIDIByte channel);

	String toString() const;

private:
	BitSet32 availableChannels;
	BitSet32 enabledChannels;
	MIDICIProfileId id;
	MemoryBlock specificData;
	ListenerList<Listener> listeners;
};



//
// MIDICIProfileList
//

class MIDICIProfileList
	: protected MIDICIProfileState::Listener
{
public:
	MIDICIProfileList(MUID muid_);
	MIDICIProfileList(const MIDICIProfileList& other);
	MIDICIProfileList& operator=(const MIDICIProfileList& other);

	MUID getMUID() const { return muid; }
	/** update the muid */
	void setMUID(MUID muid_);

	/** get total number of defined profiles on any channel */
	int getProfileCount() const { return list.size(); }
	MIDICIProfileState& getProfile(int index) { return list.getReference(index); }
	const MIDICIProfileState& getProfile(int index) const { return list.getReference(index); }
	/** get number of defined profiles on that exact channel */
	int getProfileCountOnSpecificChannel(MIDIByte channel) const;

	bool hasProfile(const MIDICIProfileId& id) const;
	/** Retrieve a profile by id. If it does not exist, return an invalid profile */
	MIDICIProfileState& getProfile(const MIDICIProfileId& id);
	const MIDICIProfileState& getProfile(const MIDICIProfileId& id) const;

	/** @return the just-added profile with no available channels, or if it already existed, the existing one */
	MIDICIProfileState& addProfile(const MIDICIProfileId& id);
	/** @return the just-added profile, or if it already existed, the existing, overwritten one */
	MIDICIProfileState& addProfile(MIDIByte channel, bool enabled, const MIDICIProfileId& id);

	void removeProfile(const MIDICIProfileId& id);

	void clear();

	class Listener
	{
	public:
		virtual ~Listener() {}

		virtual void onProfileAdded(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/) { /*optional*/ };
		virtual void onProfileRemoved(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/) { /*optional*/ };
		virtual void onProfileCanEnable(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/, MIDIByte /*channel*/, /*IN OUT*/bool& /*newEnabledState*/) { /*optional*/ };
		virtual void onProfileEnabledChange(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/, MIDIByte /*channel*/) { /*optional*/ };
		virtual void onProfileAvailableChange(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/, MIDIByte /*channel*/) { /*optional*/ };
		virtual void onProfileSpecificDataChange(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/) { /*optional*/ };
	};

	void addListener(Listener* listener) { listeners.add(listener); }
	void removeListener(Listener* listener) { listeners.remove(listener); }

protected:
	// MIDICIProfileState::Listener
	void onProfileCanEnable(MIDICIProfileState& state, MIDIByte channel, /*IN OUT*/bool& newEnabledState) override;
	void onProfileEnabledChange(MIDICIProfileState& state, MIDIByte channel) override;
	void onProfileAvailableChange(MIDICIProfileState& state, MIDIByte channel) override;
	void onProfileSpecificDataChange(MIDICIProfileState& state) override;

private:
	MUID muid;
	SortedSet<MIDICIProfileState, CriticalSection> list;
	ListenerList<Listener> listeners;
};
