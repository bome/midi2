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

#include "midi2_support.h"
#include <assert.h>

// -------------------------- UMPacket --------------------------

/** an implementation of the MIDI 2.0 UMP Packet */
class UMPacket
{
public:

	// Message Type
	typedef enum
	{
		/*0*/ Utility = 0,
		/*1*/ System,
		/*2*/ M1ChannelVoice,
		/*3*/ Data64,
		/*4*/ M2ChannelVoice,
		/*5*/ Data128,
		/*6*/ Reserved6,
		/*7*/ Reserved7,
		/*8*/ Reserved8,
		/*9*/ Reserved9,
		/*A*/ Reserved10,
		/*B*/ Reserved11,
		/*C*/ Reserved12,
		/*D*/ Reserved13,
		/*E*/ Reserved14,
		/*F*/ Reserved15,
		MessageTypeCOUNT
	}
	MessageType;
	
	/** @return the message size in bytes, given the message type */
	static int messageTypeToSize(MessageType mt);
	
	static const char* messageTypeToString(MessageType mt);

	// MIDI 1.0 Channel Voice Messages

	typedef enum
	{
		M1StatusNoteOff = 0x8,
		M1StatusNoteOn = 0x9,
		M1StatusPressure = 0xA,
		M1StatusControlChange = 0xB,
		M1StatusProgramChange = 0xC,
		M1StatusChannelPressure = 0xD,
		M1StatusPitchBend = 0xE
	}
	M1ChannelVoiceStatus;

	static const char* m1ChannelVoiceStatusToString(M1ChannelVoiceStatus status);


	// MIDI 2.0 Channel Voice Messages

	typedef enum
	{
		M2StatusRegisteredPerNoteCC = 0x0,
		M2StatusAssignablePerNoteCC = 0x1,
		M2StatusRegisteredCC = 0x2,
		M2StatusAssignableCC = 0x3,
		M2StatusRelativeRegisteredCC = 0x4,
		M2StatusRelativeAssignableCC = 0x5,
		M2StatusPerNotePitchBend = 0x6,
		M2StatusReserved7 = 0x7,
		M2StatusNoteOff = 0x8,
		M2StatusNoteOn = 0x9,
		M2StatusPressure = 0xA,
		M2StatusControlChange = 0xB,
		M2StatusProgramChange = 0xC,
		M2StatusChannelPressure = 0xD,
		M2StatusPitchBend = 0xE,
		M2StatusPerNoteManagement = 0xF
	}
	M2ChannelVoiceStatus;

	static const char* m2ChannelVoiceStatusToString(M2ChannelVoiceStatus status);

	
	typedef enum
	{
		AttributeNone = 0x00,
		AttributeManufacturerSpecific = 0x01,
		AttributeProfileSpecific = 0x02,
		AttributePitch7_9 = 0x03
	}
	AttributeType;

	
	typedef enum
	{
		BankSelectValidFlag = 1
	}
	ProgramChangeFlag;

    typedef enum
    {
        DetachPerNoteControllers = 0x01,
        ResetPerNoteControllers = 0x02
    }
    PerNoteManagementFlag;

	// ------------------------------------------------------------------

	UMPacket();

	/**
	 * @param sizeInBytes must be 4, 8, 12, or 16 
	 */
	UMPacket(const byte* data, int sizeInBytes);

	/**
	 * The MSB of data will be the first byte of the packet.
	 * @param count must be 1, 2, or 4
	 */
	UMPacket(const uint32* data, int count);
	UMPacket(uint32 data);
	UMPacket(uint32 data1, uint32 data2);
	UMPacket(uint32 data1, uint32 data2, uint32 data3);
	UMPacket(uint32 data1, uint32 data2, uint32 data3, uint32 data4);

	UMPacket& initNoteOff(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity);
	UMPacket& initNoteOff(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity, uint8 attributeType, uint16 attribute);
	UMPacket& initNoteOn(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity);
	UMPacket& initNoteOn(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity, uint8 attributeType, uint16 attribute);
	UMPacket& initPolyPressure(uint4 group, uint4 channel, uint7 noteNumber, uint32 pressure);
	UMPacket& initControlChange(uint4 group, uint4 channel, uint7 controllerIndex, uint32 value);
	UMPacket& initAssignableCC(uint4 group, uint4 channel, uint7 bank, uint7 index, uint32 value);
	UMPacket& initRegisteredCC(uint4 group, uint4 channel, uint7 bank, uint7 index, uint32 value);
	UMPacket& initProgramChange(uint4 group, uint4 channel, uint8 optionFlags, uint7 program, uint7 bankLSB, uint7 bankMSB);
	UMPacket& initChannelPressure(uint4 group, uint4 channel, uint32 value);
	UMPacket& initPitchBend(uint4 group, uint4 channel, uint32 value);

    UMPacket& initPerNoteAssignableCC(uint4 group, uint4 channel, uint7 noteNumber, uint7 index, uint32 value);
    UMPacket& initPerNoteRegisteredCC(uint4 group, uint4 channel, uint7 noteNumber, uint7 index, uint32 value);
    UMPacket& initPerNoteManagement(uint4 group, uint4 channel, uint7 noteNumber, uint8 optionFlags/*PerNoteManagementFlag*/);

	// raw data

	/** @return the size in words (1, 2, 3, or 4) */
	int getSizeInWords() const { return messageTypeToSize(getMessageType()); }

	const uint32* getData() const { return data; }

	/** @return the 1st data word */
	uint32 getWord1() const { return data[0]; }
	/** @return the 2nd data word if size is 64, 96, or 128 bit */
	uint32 getWord2() const { return data[1]; }
	/** @return the 3rd data word if size is 96 or 128 bit */
	uint32 getWord3() const { return data[2]; }
	/** @return the 4th data word if size is 128 bit */
	uint32 getWord4() const { return data[3]; }
	
	/** @return the data word with the given index */
	uint32 getWord(uint index) const { assert(index < 4); return data[index]; }
	uint16 getWordUInt16_1(uint index) const { assert(index < 4); return (uint16)(data[index] >> 16); }
	uint16 getWordUInt16_2(uint index) const { assert(index < 4); return (uint16)(data[index] & 0xFFFF); }
	byte getWordByte1(uint index) const { assert(index < 4); return (byte)((data[index] >> 24) & 0xFF); }
	byte getWordByte2(uint index) const { assert(index < 4); return (byte)((data[index] >> 16) & 0xFF); }
	byte getWordByte3(uint index) const { assert(index < 4); return (byte)((data[index] >> 8) & 0xFF); }
	byte getWordByte4(uint index) const { assert(index < 4); return (byte)(data[index] & 0xFF); }

	// set data

	void setWord(uint index, uint32 w) { assert(index < 4); data[index] = w; }
	void setWord(uint index, uint16 a1, uint16 a2)
	    { assert(index < 4); data[index] = (a1 << 16) | a2; }
	void setWord(uint index, byte a1, byte a2, byte a3, byte a4)
	    { assert(index < 4); data[index] = (a1 << 24) | (a2 << 16) | (a3 << 8) | a4; }

	// common fields

	MessageType getMessageType() const { return (MessageType)(data[0] >> 28); }
	
	uint4 getGroup() const { return (uint4)((data[0] >> 24) & 0x0F); }
	void setGroup(uint4 group) { data[0] = (data[0] & 0xF0FFFFFF) | ((group & 0x0F) << 24); }


	// MIDI 1.0 Channel Voice Messages

	void setM1ChannelVoice(uint4 group, M1ChannelVoiceStatus status, uint4 channel, uint7 dataByte1, uint7 dataByte2)
	{
		data[0] = (((uint32)M1ChannelVoice) << 28)
			| (((uint32)group & 0x0F) << 24)
			| (((uint32)status & 0xF) << 20)
			| (((uint32)channel & 0xF) << 16)
			| (((uint32)dataByte1 & 0x7F) << 8)
			| ((uint32)dataByte2 & 0x7F);
	}

	M1ChannelVoiceStatus getM1Status() const { return (M1ChannelVoiceStatus)((data[0] >> 20) & 0x0F); }
	void setM1Status(M1ChannelVoiceStatus status) { data[0] = (data[0] & 0xFF0FFFFF) | ((status & 0x0F) << 20); }

	uint4 getM1Channel() const { return getM2Channel(); }
	void setM1Channel(uint4 channel) { setM2Channel(channel); }

	uint7 getM1NoteNumber() const { return getM2NoteNumber(); }
	void setM1NoteNumber(uint7 noteNumber) { setM2NoteNumber(noteNumber); }


	// MIDI 2.0 Channel Voice Messages

	void setM2ChannelVoice(uint4 group, M2ChannelVoiceStatus status, uint4 channel, uint8 dataByte1, uint8 dataByte2)
	{
		data[0] = (((uint32)M2ChannelVoice) << 28)
			| (((uint32)group & 0x0F) << 24)
			| (((uint32)status & 0x0F) << 20)
			| (((uint32)channel & 0x0F) << 16)
			| (((uint32)dataByte1) << 8)
			| ((uint32)dataByte2);
	}

	M2ChannelVoiceStatus getM2Status() const { return (M2ChannelVoiceStatus)((data[0] >> 20) & 0x0F); }
	void setM2Status(M2ChannelVoiceStatus status) { data[0] = (data[0] & 0xFF0FFFFF) | ((status & 0x0F) << 20); }

	uint4 getM2Channel() const { return (uint4)((data[0] >> 16) & 0x0F); }
	void setM2Channel(uint4 channel) { data[0] = (data[0] & 0xFFF0FFFF) | ((channel & 0x0F) << 16); }

	uint7 getM2NoteNumber() const { return (uint7)((data[0] >> 8) & 0x7F); }
	void setM2NoteNumber(uint7 noteNumber) { data[0] = (data[0] & 0xFFFF00FF) | ((noteNumber & 0x7F) << 8); }

	// Debugging

	const char* toString() const;
private:
	uint32 data[4];
};


//
// ----------- MIDI2Processor (base class) -----------
//
class MIDI2Processor
{
public:
    virtual ~MIDI2Processor() {}
    
    /** abstract process() method, to be implemented by subclasses */
    virtual void process(uint64 timestamp, const UMPacket& packet) = 0;

    /** wrapper for process(), taking a series of raw UMP packet words */
    virtual void processRawUMP(uint64 timestamp, const uint32* rawWords, int sizeInWords);
    
    /** optional error handler */
    virtual void onCorruptRawData(const char* errorMessage) {}
    
};
