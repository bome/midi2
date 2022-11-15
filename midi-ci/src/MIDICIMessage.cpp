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
#include "MIDICIMessage.h"
#include "MIDICIConstants.h"

MIDICIMessage::MIDICIMessage(const MIDIMessage& message)
	: MIDIMessage(message.getData(), message.getLength())
{
	// nothing
}

MIDICIMessage::MIDICIMessage(MIDIByte* data, int length)
	: MIDIMessage(data, length)
{
	// nothing
}

bool MIDICIMessage::isMIDICIMessage(const MIDIMessage& message)
{
	const MIDIByte* data = message.getData();
	return data != nullptr
		&& message.getLength() > MIDICI_COMMON_HEADER_LENGTH
		&& data[MIDICI_INDEX_SYSEX_START] == MIDI_SYSEX_START
		&& data[MIDICI_INDEX_SYSEX_ID] == MIDI_SYSEX_UNIVERSAL_NON_REALTIME
		// Device ID/Channel
		&& (data[MIDICI_INDEX_CHANNEL] == MIDICI_CHANNEL_PORT
			|| data[MIDICI_INDEX_CHANNEL] <= MIDI_CHANNEL_MAX)
		 // Sys Ex ID #1: MIDI-CI
		&& data[MIDICI_INDEX_SUBID1] == MIDI_SYSEX_SUBID1_MIDICI
		// note: accept all future MIDI-CI versions here. Can send NAK later during processing.
		&& data[MIDICI_INDEX_VERSION] >= MIDICI_CURRENT_VERSION
		&& data[message.getLength() - 1] == MIDI_SYSEX_END;
}

bool MIDICIMessage::isMIDICIMessage(const MIDIMessage& message, int minMessageType, int maxMessageType)
{
	return isMIDICIMessage(message)
		&& (message.getData()[MIDICI_INDEX_MESSAGE_TYPE] >= minMessageType)
		&& (message.getData()[MIDICI_INDEX_MESSAGE_TYPE] <= maxMessageType);
}

MIDIByte MIDICIMessage::getChannel() const
{
	//assert(length >= MIDICI_INDEX_CHANNEL);
	return _data[MIDICI_INDEX_CHANNEL];
}

void MIDICIMessage::setChannel(MIDIByte channel)
{
	//jassert(data != nullptr);
	//jassert(length >= MIDICI_INDEX_CHANNEL
	_data[MIDICI_INDEX_CHANNEL] = channel;
}

int MIDICIMessage::getMessageType() const
{
	//assert(length >= MIDICI_INDEX_MESSAGE_TYPE);
	return _data[MIDICI_INDEX_MESSAGE_TYPE];
}

MUID MIDICIMessage::getSourceMUID() const
{
	return (MUID)extractNumber28_lsbfirst(MIDICI_INDEX_SRC_MUID);
}

MUID MIDICIMessage::getDestinationMUID() const
{
	return (MUID)extractNumber28_lsbfirst(MIDICI_INDEX_DST_MUID);
}

bool MIDICIMessage::isAddressedTo(MUID muid) const
{
	MUID destination = getDestinationMUID();
	return (destination == muid)
		|| (destination == MIDICI_MUID_BROADCAST);
}

String MIDICIMessage::fmtMUID(MUID muid)
{
	static String broadcast("broadcast MUID");
	if (muid == MIDICI_MUID_BROADCAST)
	{
		return broadcast;
	}
	String ret;
	ret.preallocateBytes(20);
	ret += "0x";
	for (int shift = 24; shift >= 0; shift -= 4)
	{
		ret += toHexNibble((MIDIByte)(muid >> shift));
	}
	ret += " (";
	ret += String(muid);
	ret += ")";
	return ret;
}

String MIDICIMessage::fmtChannel(MIDIByte channel)
{
	if (channel == MIDICI_CHANNEL_PORT)
	{
		static String portChannel("port channel");
		return portChannel;
	}
	static String channelString("channel ");
	return channelString + String(channel + 1);
}

void MIDICIMessage::clear()
{
	memset(_data, 0, (size_t)_length);
}

void MIDICIMessage::fillHeader(MIDIByte messageType, MUID source, MUID destination)
{
	//jassert(data != nullptr);
	//jassert(length > MIDICI_COMMON_HEADER_LENGTH);
	_data[MIDICI_INDEX_SYSEX_START] = MIDI_SYSEX_START;
	_data[MIDICI_INDEX_SYSEX_ID] = MIDI_SYSEX_UNIVERSAL_NON_REALTIME;
	_data[MIDICI_INDEX_CHANNEL] = MIDICI_CHANNEL_PORT;
	_data[MIDICI_INDEX_SUBID1] = MIDI_SYSEX_SUBID1_MIDICI;
	_data[MIDICI_INDEX_MESSAGE_TYPE] = messageType;
	_data[MIDICI_INDEX_VERSION] = MIDICI_CURRENT_VERSION;
	writeNumber28_lsbfirst(source, MIDICI_INDEX_SRC_MUID);
	writeNumber28_lsbfirst(destination, MIDICI_INDEX_DST_MUID);
	// also write end of Sys Ex
	_data[_length - 1] = MIDI_SYSEX_END;
}

uint16 MIDICIMessage::extractNumber14_lsbfirst(int offset) const
{
	MIDIByte* data = &_data[offset];
	return ((uint32)data[0] & 0x7F) | (((uint32)data[1] & 0x7F) << 7);
}

uint16 MIDICIMessage::extractNumber16_lsbfirst(int offset) const
{
	MIDIByte* data = &_data[offset];
	return ((uint32)data[0] & 0x7F) | (((uint32)data[1] & 0x7F) << 8);
}

uint32 MIDICIMessage::extractNumber24_msbfirst(int offset) const
{
	MIDIByte* data = &_data[offset];
	return (((uint32)data[0] & 0x7F) << 16) | (((uint32)data[1] & 0x7F) << 8) | ((uint32)data[2] & 0x7F);
}

uint32 MIDICIMessage::extractNumber28_lsbfirst(int offset) const
{
	MIDIByte* data = &_data[offset];
	return ((uint32)data[0] & 0x7F)
		| (((uint32)data[1] & 0x7F) << 7)
		| (((uint32)data[2] & 0x7F) << 14)
		| (((uint32)data[3] & 0x7F) << 21);
}

uint32 MIDICIMessage::extractNumber32_lsbfirst(int offset) const
{
	MIDIByte* data = &_data[offset];
	return ((uint32)data[0] & 0x7F) | (((uint32)data[1] & 0x7F) << 8) | (((uint32)data[2] & 0x7F) << 16) | (((uint32)data[3] & 0x7F) << 24);
}

void MIDICIMessage::writeNumber14_lsbfirst(uint16 number, int offset)
{
	MIDIByte* data = &_data[offset];
	*data++ = number & 0x7F;
	*data = (number >> 7) & 0x7F;
}

void MIDICIMessage::writeNumber16_lsbfirst(uint16 number, int offset)
{
	MIDIByte* data = &_data[offset];
	*data++ = number & 0x7F;
	*data = (number >> 8) & 0x7F;
}

void MIDICIMessage::writeNumber24_msbfirst(uint32 number, int offset)
{
	MIDIByte* data = &_data[offset];
	*data++ = (number >> 16) & 0x7F;
	*data++ = (number >> 8) & 0x7F;
	*data = number & 0x7F;
}

void MIDICIMessage::writeNumber28_lsbfirst(uint32 number, int offset /*=0*/)
{
	MIDIByte* data = &_data[offset];
	*data++ = number & 0x7F;
	*data++ = (number >> 7) & 0x7F;
	*data++ = (number >> 14) & 0x7F;
	*data = (number >> 21) & 0x7F;
}

void MIDICIMessage::writeNumber32_lsbfirst(uint32 number, int offset)
{
	MIDIByte* data = &_data[offset];
	*data++ = number & 0x7F;
	*data++ = (number >> 8) & 0x7F;
	*data++ = (number >> 16) & 0x7F;
	*data = (number >> 24) & 0x7F;
}
