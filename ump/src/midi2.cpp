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
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif //_WIN32

#include "midi2.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//
// MARK: UMPacket
//

int UMPacket::messageTypeToSize(MessageType mt)
{
	switch (mt)
	{
	/*0*/ case Utility:    return 1;
	/*1*/ case System:     return 1;
	/*2*/ case M1ChannelVoice: return 1;
	/*3*/ case Data64:     return 2;
	/*4*/ case M2ChannelVoice: return 2;
	/*5*/ case Data128:    return 4;
	/*6*/ case Reserved6:  return 1;
	/*7*/ case Reserved7:  return 1;
	/*8*/ case Reserved8:  return 2;
	/*9*/ case Reserved9:  return 2;
	/*A*/ case Reserved10: return 2;
	/*B*/ case Reserved11: return 3;
	/*C*/ case Reserved12: return 3;
	/*D*/ case Reserved13: return 4;
	/*E*/ case Reserved14: return 4;
	/*F*/ case Reserved15: return 4;
	case MessageTypeCOUNT:
		break;
	}
	return 0;
}


const char* UMPacket::messageTypeToString(MessageType mt)
{
	switch (mt)
	{
	/*0*/ case Utility:    return "Utility";
	/*1*/ case System:     return "System";
	/*2*/ case M1ChannelVoice: return "MIDI1ChannelVoice";
	/*3*/ case Data64:     return "Data64";
	/*4*/ case M2ChannelVoice: return "MIDI2ChannelVoice";
	/*5*/ case Data128:    return "Data128";
	/*6*/ case Reserved6:  return "Reserved6";
	/*7*/ case Reserved7:  return "Reserved7";
	/*8*/ case Reserved8:  return "Reserved8";
	/*9*/ case Reserved9:  return "Reserved9";
	/*A*/ case Reserved10: return "Reserved10";
	/*B*/ case Reserved11: return "Reserved11";
	/*C*/ case Reserved12: return "Reserved12";
	/*D*/ case Reserved13: return "Reserved13";
	/*E*/ case Reserved14: return "Reserved14";
	/*F*/ case Reserved15: return "Reserved15";
	case MessageTypeCOUNT:
		break;
	}
	return "";
}


const char* UMPacket::m1ChannelVoiceStatusToString(M1ChannelVoiceStatus status)
{
	switch (status)
	{
	case M1StatusNoteOff: return "NoteOff";
	case M1StatusNoteOn: return "NoteOn";
	case M1StatusPressure: return "Pressure";
	case M1StatusControlChange: return "ControlChange";
	case M1StatusProgramChange: return "ProgramChange";
	case M1StatusChannelPressure: return "ChannelPressure";
	case M1StatusPitchBend: return "PitchBend";
	}
	return "";
}


const char* UMPacket::m2ChannelVoiceStatusToString(M2ChannelVoiceStatus status)
{
	switch (status)
	{
	case M2StatusRegisteredPerNoteCC: return "RegisteredPerNoteCC";
	case M2StatusAssignablePerNoteCC: return "AssignablePerNoteCC";
	case M2StatusRegisteredCC: return "RegisteredCC";
	case M2StatusAssignableCC: return "AssignableCC";
	case M2StatusRelativeRegisteredCC: return "RelativeRegisteredCC";
	case M2StatusRelativeAssignableCC: return "RelativeAssignableCC";
	case M2StatusPerNotePitchBend: return "PerNotePitchBend";
	case M2StatusReserved7: return "Reserved7";
	case M2StatusNoteOff: return "NoteOff";
	case M2StatusNoteOn: return "NoteOn";
	case M2StatusPressure: return "Pressure";
	case M2StatusControlChange: return "ControlChange";
	case M2StatusProgramChange: return "ProgramChange";
	case M2StatusChannelPressure: return "ChannelPressure";
	case M2StatusPitchBend: return "PitchBend";
	case M2StatusPerNoteManagement: return "PerNoteManagement";
	}
	return "";
}


UMPacket::UMPacket()
{
	memset(data, 0, sizeof(data));
}


UMPacket::UMPacket(const byte* d, int sizeInBytes)
{
	if (sizeInBytes > (int)sizeof(data))
	{
		sizeInBytes = (int)sizeof(data);
	}
	int word = 0;
	while (sizeInBytes >= 4)
	{
		setWord(word, d[0], d[1], d[2], d[3]);
		word++;
		d += 4;
		sizeInBytes -= 4;
	}
}


UMPacket::UMPacket(const uint32* d, int count)
{
	if (count > 4)
	{
		count = 4;
	}
	memcpy((void*)data, (void*)d, count * 4);
}


UMPacket::UMPacket(uint32 d)
{
	data[0] = d;
}


UMPacket::UMPacket(uint32 data1, uint32 data2)
{
	data[0] = data1;
	data[1] = data2;
}


UMPacket::UMPacket(uint32 data1, uint32 data2, uint32 data3)
{
	data[0] = data1;
	data[1] = data2;
	data[2] = data3;
}


UMPacket::UMPacket(uint32 data1, uint32 data2, uint32 data3, uint32 data4)
{
	data[0] = data1;
	data[1] = data2;
	data[2] = data3;
	data[3] = data4;
}


UMPacket& UMPacket::initNoteOff(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity, uint8 attributeType, uint16 attribute)
{
	setM2ChannelVoice(group, M2StatusNoteOff, channel, (byte)(noteNumber & 0x7F), (byte)(attributeType & 0xFF));
	setWord(1, velocity, attribute);
	return *this;
}


UMPacket& UMPacket::initNoteOff(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity)
{
	setM2ChannelVoice(group, M2StatusNoteOff, channel, (byte)(noteNumber & 0x7F), AttributeNone);
	setWord(1, velocity, 0);
	return *this;
}

UMPacket& UMPacket::initNoteOn(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity, uint8 attributeType, uint16 attribute)
{
	setM2ChannelVoice(group, M2StatusNoteOn, channel, (byte)(noteNumber & 0x7F), (byte)(attributeType & 0xFF));
	setWord(1, velocity, attribute);
	return *this;
}


UMPacket& UMPacket::initNoteOn(uint4 group, uint4 channel, uint7 noteNumber, uint16 velocity)
{
	setM2ChannelVoice(group, M2StatusNoteOn, channel, (byte)(noteNumber & 0x7F), AttributeNone);
	setWord(1, velocity, 0);
	return *this;
}

UMPacket& UMPacket::initPolyPressure(uint4 group, uint4 channel, uint7 noteNumber, uint32 pressure)
{
	setM2ChannelVoice(group, M2StatusPressure, channel, (byte)(noteNumber & 0x7F), 0);
	setWord(1, pressure);
	return *this;
}


UMPacket& UMPacket::initControlChange(uint4 group, uint4 channel, uint7 controllerIndex, uint32 value)
{
	setM2ChannelVoice(group, M2StatusControlChange, channel, (byte)(controllerIndex & 0x7F), 0);
	setWord(1, value);
	return *this;
}

UMPacket& UMPacket::initAssignableCC(uint4 group, uint4 channel, uint7 bank, uint7 index, uint32 value)
{
	setM2ChannelVoice(group, M2StatusAssignableCC, channel, (byte)(bank & 0x7F), (byte)(index & 0x7F));
	setWord(1, value);
	return *this;
}

UMPacket& UMPacket::initRegisteredCC(uint4 group, uint4 channel, uint7 bank, uint7 index, uint32 value)
{
	setM2ChannelVoice(group, M2StatusRegisteredCC, channel, (byte)(bank & 0x7F), (byte)(index & 0x7F));
	setWord(1, value);
	return *this;
}


UMPacket& UMPacket::initProgramChange(uint4 group, uint4 channel, uint8 optionFlags, uint7 program, uint7 bankLSB, uint7 bankMSB)
{
	setM2ChannelVoice(group, M2StatusProgramChange, channel, 0, optionFlags);
	setWord(1, (byte)(program & 0x7F), 0, (byte)(bankMSB & 0x7F), (byte)(bankLSB & 0x7F));
	return *this;
}


UMPacket& UMPacket::initChannelPressure(uint4 group, uint4 channel, uint32 value)
{
	setM2ChannelVoice(group, M2StatusChannelPressure, channel, 0, 0);
	setWord(1, value);
	return *this;
}


UMPacket& UMPacket::initPitchBend(uint4 group, uint4 channel, uint32 value)
{
	setM2ChannelVoice(group, M2StatusPitchBend, channel, 0, 0);
	setWord(1, value);
	return *this;
}


UMPacket& UMPacket::initPerNoteAssignableCC(uint4 group, uint4 channel, uint7 noteNumber, uint7 index, uint32 value)
{
    setM2ChannelVoice(group, M2StatusAssignablePerNoteCC, channel, (byte)(noteNumber & 0x7F), (byte)(index & 0x7F));
    setWord(1, value);
    return *this;
}

UMPacket& UMPacket::initPerNoteRegisteredCC(uint4 group, uint4 channel, uint7 noteNumber, uint7 index, uint32 value)
{
    setM2ChannelVoice(group, M2StatusRegisteredPerNoteCC, channel, (byte)(noteNumber & 0x7F), (byte)(index & 0x7F));
    setWord(1, value);
    return *this;
}

UMPacket& UMPacket::initPerNoteManagement(uint4 group, uint4 channel, uint7 noteNumber, uint8 optionFlags/*PerNoteManagementFlag*/)
{
    setM2ChannelVoice(group, M2StatusPerNoteManagement, channel, (byte)(noteNumber & 0x7F), optionFlags);
    setWord(1, 0);
    return *this;
}


const char* UMPacket::toString() const
{
	// quick&dirty, not thread safe!
	static char buf[2048];
	static char msg[1024];

	sprintf(msg, "%01X %01X %01X %01X %02X %02X",
		data[0] >> 28, (data[0] >> 24) & 0xF, (data[0] >> 20) & 0xF, (data[0] >> 16) & 0xF, (data[0] >> 8) & 0xFF, data[0] & 0xFF);
	if (getSizeInWords() > 1)
	{
		char* tmp = msg;
		tmp += strlen(msg);
		sprintf(tmp, " %04X %04X", data[1] >> 16, data[1] & 0xFFFF);
	}

	sprintf(buf, "MIDI 2 packet (%d words): %s%s: %s", getSizeInWords(), msg,
		getSizeInWords() > 2 ? "..." : "",
		messageTypeToString(getMessageType()));
	if (getMessageType() == M2ChannelVoice)
	{
		char* tmp = buf;
		tmp += strlen(buf);
		sprintf(tmp, " %s", m2ChannelVoiceStatusToString(getM2Status()));
	}
	return buf;
}


//
// MIDI2Processor base class
//

void MIDI2Processor::processRawUMP(uint64 timestamp, const uint32* rawWords, int sizeInWords)
{
    if (rawWords == nullptr)
    {
        return;
    }
    UMPacket packet;
    while (sizeInWords > 0)
    {
        packet.setWord(0, rawWords[0]);
        int packetSize = packet.getSizeInWords();
        if (sizeInWords < packetSize)
        {
            onCorruptRawData("incomplete UMP received.");
        }
        else
        {
            switch (packetSize)
            {
                case 2:
                    packet.setWord(1, rawWords[1]);
                    break;
                case 3:
                    packet.setWord(1, rawWords[1]);
                    packet.setWord(2, rawWords[2]);
                    break;
                case 4:
                    packet.setWord(1, rawWords[1]);
                    packet.setWord(2, rawWords[2]);
                    packet.setWord(3, rawWords[3]);
                    break;
            }// switch

            process(timestamp, packet);
        }
        
        sizeInWords -= packetSize;
        rawWords += packetSize;
    }
}
