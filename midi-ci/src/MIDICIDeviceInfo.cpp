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
#include "MIDICIDeviceInfo.h"
#include "MIDICIConstants.h"


//
// MIDICIDeviceInfo
//

MIDICIDeviceInfo::MIDICIDeviceInfo()
	: profiles(MIDICI_MUID_INVALID)
{
	clear();
}

MIDICIDeviceInfo::MIDICIDeviceInfo(MUID muid_)
	: profiles(muid_)
{
	clear();
	muid = muid_;
}

MIDICIDeviceInfo::MIDICIDeviceInfo(const MIDICIDeviceInfo& other)
	: muid(other.muid)
	, manufacturerId(other.manufacturerId)
	, familyId(other.familyId)
	, modelId(other.modelId)
	, versionId(other.versionId)
	, maxSysExSize(other.maxSysExSize)
	, categoriesSupported(other.categoriesSupported)
	, profiles(other.profiles)
{
	// nothing
}

MIDICIDeviceInfo& MIDICIDeviceInfo::operator=(const MIDICIDeviceInfo& other)
{
	muid = other.muid;
	manufacturerId = other.manufacturerId;
	familyId = other.familyId;
	modelId = other.modelId;
	versionId = other.versionId;
	maxSysExSize = other.maxSysExSize;
	categoriesSupported = other.categoriesSupported;
	profiles = other.profiles;
	return *this;
}

void MIDICIDeviceInfo::setMUID(MUID muid_)
{
	muid = muid_;
	profiles.setMUID(muid_);
}


void MIDICIDeviceInfo::clear()
{
	muid = MIDICI_MUID_INVALID;
	manufacturerId = 0;
	familyId = 0;
	modelId = 0;
	versionId = 0;
	maxSysExSize = MIDICI_MINIMUM_RECEIVABLE_SYSEX_SIZE;
	categoriesSupported = 0;
	profiles.clear();
	lastReceiveTime = 0;
}


bool MIDICIDeviceInfo::isValid() const
{
	return muid != MIDICI_MUID_INVALID
		&& manufacturerId != 0;
}

