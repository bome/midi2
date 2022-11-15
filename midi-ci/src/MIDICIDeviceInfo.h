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
#include "MIDICIProfileTypes.h"

//
// MIDICIDeviceInfo
//

/** A container for basic MIDI-CI device information */
class MIDICIDeviceInfo
{
public:
	/** create an invalid MIDICIDeviceInfo */
	MIDICIDeviceInfo();
	MIDICIDeviceInfo(MUID muid_);
	MIDICIDeviceInfo(const MIDICIDeviceInfo& other);
	MIDICIDeviceInfo& operator=(const MIDICIDeviceInfo& other);

	MUID getMUID() const { return muid; }
	void setMUID(MUID muid_);

	uint32 manufacturerId;
	uint16 familyId;
	uint16 modelId;
	uint32 versionId;
	int maxSysExSize;
	MIDIByte categoriesSupported;
	MIDICIProfileList profiles;

	int64 lastReceiveTime;

	void clear();
	bool isValid() const;
	
	// sorting support for quick look-up in the SortedSet
	// muid is the primary, unique key
	bool operator<(const MIDICIDeviceInfo& f2) const { return muid < f2.muid; }
	bool operator>(const MIDICIDeviceInfo& f2) const { return muid > f2.muid; }
	bool operator<=(const MIDICIDeviceInfo& f2) const { return muid <= f2.muid; }
	bool operator>=(const MIDICIDeviceInfo& f2) const { return muid >= f2.muid; }
	bool operator==(const MIDICIDeviceInfo& f2) const { return muid == f2.muid; }
	bool operator!=(const MIDICIDeviceInfo& f2) const { return muid != f2.muid; }

private:
	MUID muid;
};
