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

#include "midi2_translation.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// ---------------------------------
// Translation
// ---------------------------------
#define TRANSLATION_BANKCHANGE_TIME_THRESHOLD_MILLIS  (500)


//
// MARK: MIDI2Translator
// 

MIDI2Translator::MIDI2Translator()
{
	init();
}


MIDI2Translator::MIDI2Translator(Listener* _listener)
{
	init();
	listener = _listener;
}


void MIDI2Translator::init()
{
	listener = NULL;
	translateToMIDI2Group = 0;
	translateFromMIDI2Group = -1;
	bankChangeLSBtime = 0;
	bankChangeMSBtime = 0;
	memset(runtimeFlags, 0, sizeof(runtimeFlags));
	memset(paramNRPN_MSB, 0, sizeof(paramNRPN_MSB));
	memset(paramNRPN_LSB, 0, sizeof(paramNRPN_LSB));
	memset(valueNRPN_MSB, 0, sizeof(valueNRPN_MSB));
}

void MIDI2Translator::setListener(Listener* _listener)
{
	listener = _listener;
}


MIDI2Translator::Listener* MIDI2Translator::getListener() const
{
	return listener;
}


void MIDI2Translator::setTranslateToMIDI2Group(uint4 group)
{
	translateToMIDI2Group = group;
}


int MIDI2Translator::getTranslateToMIDI2Group() const
{
	return translateToMIDI2Group;
}

void MIDI2Translator::setTranslateFromMIDI2Group(uint4 group)
{
	translateFromMIDI2Group = group;
}


int MIDI2Translator::getTranslateFromMIDI2Group() const
{
	return translateFromMIDI2Group;
}

bool MIDI2Translator::midi1ControlChangeReceived(uint4 channel, uint7 index, uint7 value)
{
	switch (index)
	{
	case MIDI_CC_BANKSELECT_MSB:
	{
		// remember bank changes
		bankChangeMSB = (byte)value;
		bankChangeMSBtime = getMilliTime();
		break;
	}
	case MIDI_CC_BANKSELECT_LSB:
	{
		// remember bank changes
		bankChangeLSB = (byte)value;
		bankChangeLSBtime = getMilliTime();
		break;
	}
	case MIDI_CC_DATA_MSB:
	{
		valueNRPN_MSB[channel] = (byte)(value & 0x7F);
		runtimeFlags[channel] |= receivedNRPNValueMSB;
		break;
	}
	case MIDI_CC_DATA_LSB:
	{
		if (((runtimeFlags[channel] & receivedNRPNParamMSB) == 0)
			|| ((runtimeFlags[channel] & receivedNRPNParamLSB) == 0)
			|| ((runtimeFlags[channel] & receivedNRPNValueMSB) == 0))
		{
			// ignore
		}
		else if ((runtimeFlags[channel] & receivedNRPN) != 0)
		{
			// NRPN / Assignable
			listener->translatedMessage(UMPacket().initAssignableCC(translateToMIDI2Group, channel,
				paramNRPN_MSB[channel], paramNRPN_LSB[channel], convert14to32((byte)value, (byte)valueNRPN_MSB[channel])));
		}
		else if ((runtimeFlags[channel] & receivedRPN) != 0)
		{
			// RPN / Registered
			listener->translatedMessage(UMPacket().initRegisteredCC(translateToMIDI2Group, channel,
				paramNRPN_MSB[channel], paramNRPN_LSB[channel], convert14to32((byte)value, (byte)valueNRPN_MSB[channel])));
		}
		else
		{
			// ignore
		}
		break;
	}
	case MIDI_CC_NRPN_LSB:
	{
		if ((runtimeFlags[channel] & receivedNRPN) != 0)
		{
			//we're doing NRPN
			runtimeFlags[channel] |= receivedNRPNParamLSB;
			paramNRPN_LSB[channel] = (byte)(value & 0x7F);
		}
		else
		{
			// ignore
		}
		break;
	}
	case MIDI_CC_NRPN_MSB:
	{
		// MSB always resets
		runtimeFlags[channel] |= (receivedNRPN | receivedNRPNParamMSB);
		runtimeFlags[channel] &= ~(receivedRPN | receivedNRPNValueMSB | receivedNRPNParamLSB);
		paramNRPN_MSB[channel] = (byte)(value & 0x7F);
		break;
	}
	case MIDI_CC_RPN_LSB:
	{
		if ((runtimeFlags[channel] & receivedRPN) != 0)
		{
			runtimeFlags[channel] |= receivedNRPNParamLSB;
			paramNRPN_LSB[channel] = (byte)(value & 0x7F);
		}
		else
		{
			// ignore
		}
		break;
	}
	case MIDI_CC_RPN_MSB:
	{
		// MSB always resets
		runtimeFlags[channel] |= (receivedRPN | receivedNRPNParamMSB);
		runtimeFlags[channel] &= ~(receivedNRPN | receivedNRPNValueMSB | receivedNRPNParamLSB);
		paramNRPN_MSB[channel] = (byte)(value & 0x7F);
		break;
	}
	}

	// In all cases, send the original message, too.
	listener->translatedMessage(UMPacket().initControlChange(translateToMIDI2Group, channel, index, convert7to32((byte)value)));

	// TODO: optional features can be activated separately:
	// - 14-bit controllers
	// - All Notes Off, All Sound Off
	return TRUE;
}


bool MIDI2Translator::midi1Received(const byte* data, int len)
{
	if (!listener) return FALSE;
	if (len <= 0) return FALSE;

	UMPacket packet;
	uint4 channel = *data & 0x0F;

	if (len == 3)
	{
		uint7 data1 = (uint7)data[1];
		uint7 data2 = (uint7)data[2];

		switch (*data & 0xF0)
		{
		case MIDI_NOTEON:
			if (data2 > 0)
			{
				// Note On
				listener->translatedMessage(packet.initNoteOn(translateToMIDI2Group, channel, data1, (int)convert7to16((byte)data2)));
				// TODO: optional features can be activated separately:
				// - high res velocity prefix
				return TRUE;
			}
			// translate Note On with 0 velocity to MIDI 2 Note Off with 50% velocity
			data2 = 0x40;
			// fall through
		case MIDI_NOTEOFF:
			// Note Off
			//data2 = 0;
			listener->translatedMessage(packet.initNoteOff(translateToMIDI2Group, channel, data1, (int)convert7to16((byte)data2)));
			return TRUE;
		case MIDI_KEYAFTERTOUCH:
			// Polyphonic Key Pressure
			listener->translatedMessage(packet.initPolyPressure(translateToMIDI2Group, channel, data1, convert7to32((byte)data2)));
			return TRUE;
		case MIDI_CONTROLCHANGE:
			// Control Change
			return midi1ControlChangeReceived(channel, data1, data2);
		case MIDI_PITCHBEND:
			// Pitch Bend
			listener->translatedMessage(packet.initPitchBend(translateToMIDI2Group, channel, convert14to32((byte)data1, (byte)data2)));
			// TODO: respond to pitch bend range RPN
			return TRUE;
		} // switch
	}
	else if (len == 2)
	{
		uint7 data1 = (uint7)data[1];
		switch (*data & 0xF0)
		{
		case MIDI_PROGRAMCHANGE:
		{
			// Program Change
			// update bank info
			uint32 currTime = getMilliTime();
			uint32 bankLSB = 0;
			uint32 bankMSB = 0;
			uint8 options = 0;
			if (bankChangeMSBtime != 0 && ((currTime - bankChangeMSBtime) < TRANSLATION_BANKCHANGE_TIME_THRESHOLD_MILLIS))
			{
				bankMSB = (int)bankChangeMSB;
				options |= UMPacket::BankSelectValidFlag;
			}
			if (bankChangeLSBtime != 0 && ((currTime - bankChangeLSBtime) < TRANSLATION_BANKCHANGE_TIME_THRESHOLD_MILLIS))
			{
				bankLSB = (int)bankChangeLSB;
				options |= UMPacket::BankSelectValidFlag;
			}
			bankChangeMSBtime = 0;
			bankChangeLSBtime = 0;

			listener->translatedMessage(packet.initProgramChange(translateToMIDI2Group, channel, options, data1, bankLSB, bankMSB));
			return TRUE;
		}
		case MIDI_CHANAFTERTOUCH:
			// Channel Pressure
			listener->translatedMessage(packet.initChannelPressure(translateToMIDI2Group, channel, convert7to32((byte)data1)));
			return TRUE;

		} // switch
	}

	// TODO: system messages
	// TODO: sys ex, Show Control, etc.

	// tunnel non-translated messages
	return FALSE;
}


bool MIDI2Translator::umpReceived(const UMPacket& packet)
{
	if (!listener) return FALSE;

	if (translateFromMIDI2Group >= 0 && packet.getGroup() != translateFromMIDI2Group)
	{
		return FALSE;
	}

	byte data[4];

	if (packet.getMessageType() == UMPacket::M2ChannelVoice)
	{
		switch (packet.getM2Status())
		{
		case UMPacket::M2StatusNoteOn:
		{
			byte velocity = packet.getWordByte1(1) >> 1;
			if (velocity == 0)
			{
				velocity = 1;
			}
			data[0] = (byte)(MIDI_NOTEON | packet.getM2Channel());
			data[1] = packet.getWordByte3(0) & 0x7F;
			data[2] = velocity;
			listener->translatedMessage(data, 3, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusNoteOff:
		{
			data[0] = (byte)(MIDI_NOTEOFF | packet.getM2Channel());
			data[1] = packet.getWordByte3(0) & 0x7F;
			data[2] = packet.getWordByte1(1) >> 1;
			listener->translatedMessage(data, 3, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusProgramChange:
		{
			if (packet.getWordByte4(0) & UMPacket::BankSelectValidFlag)
			{
				// send bank change + program change
				data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
				data[1] = (byte)MIDI_CC_BANKSELECT_MSB;
				data[2] = packet.getWordByte3(1) & 0x7F;
				listener->translatedMessage(data, 3, packet.getGroup());
				data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
				data[1] = (byte)MIDI_CC_BANKSELECT_LSB;
				data[2] = packet.getWordByte4(1) & 0x7F;
				listener->translatedMessage(data, 3, packet.getGroup());
			}
			data[0] = (byte)(MIDI_PROGRAMCHANGE | packet.getM2Channel());
			data[1] = packet.getWordByte1(1) & 0x7F;
			listener->translatedMessage(data, 2, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusControlChange:
		{
			data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
			data[1] = packet.getWordByte3(0) & 0x7F;
			data[2] = packet.getWordByte1(1) >> 1;
			listener->translatedMessage(data, 3, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusPressure:
		{
			data[0] = (byte)(MIDI_KEYAFTERTOUCH | packet.getM2Channel());
			data[1] = packet.getWordByte3(0) & 0x7F;
			data[2] = packet.getWordByte1(1) >> 1;
			listener->translatedMessage(data, 3, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusChannelPressure:
		{
			data[0] = (byte)(MIDI_CHANAFTERTOUCH | packet.getM2Channel());
			data[1] = packet.getWordByte1(1) >> 1;
			listener->translatedMessage(data, 2, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusAssignableCC: // fall through
		case UMPacket::M2StatusRegisteredCC:
		{
			// send 4 MIDI messages
			// 1. Index MSB
			data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
			data[1] = (packet.getM2Status() == UMPacket::M2StatusRegisteredCC) ? MIDI_CC_RPN_MSB : MIDI_CC_NRPN_MSB;
			data[2] = packet.getWordByte3(0) & 0x7F;
			listener->translatedMessage(data, 3, packet.getGroup());
			// 2. Index LSB
			data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
			data[1] = (packet.getM2Status() == UMPacket::M2StatusRegisteredCC) ? MIDI_CC_RPN_LSB : MIDI_CC_NRPN_LSB;
			data[2] = packet.getWordByte4(0) & 0x7F;
			listener->translatedMessage(data, 3, packet.getGroup());
			// 3. Value MSB
			data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
			data[1] = MIDI_CC_DATA_MSB;
			data[2] = convert16to14_MSB(packet.getWordUInt16_1(1));
			listener->translatedMessage(data, 3, packet.getGroup());
			// 4. Value LSB
			data[0] = (byte)(MIDI_CONTROLCHANGE | packet.getM2Channel());
			data[1] = MIDI_CC_DATA_LSB;
			data[2] = convert16to14_LSB(packet.getWordUInt16_1(1));
			listener->translatedMessage(data, 3, packet.getGroup());
			return TRUE;
		}
		case UMPacket::M2StatusPitchBend:
		{
			data[0] = (byte)(MIDI_PITCHBEND | packet.getM2Channel());
			data[1] = convert16to14_LSB(packet.getWordUInt16_1(1));
			data[2] = convert16to14_MSB(packet.getWordUInt16_1(1));
			listener->translatedMessage(data, 3, packet.getGroup());
			return TRUE;
		}
		break;

		default:
			break;
		}
	} // switch
	return FALSE;
}


uint16 MIDI2Translator::convert7to16(byte value7)
{
	uint16 bitShiftedValue = ((uint16)value7) << 9;
	if (value7 <= 64)
	{
		return bitShiftedValue;
	}
	// use bit repeat bits from extended value7
	uint16 repeatValue7 = value7 & 0x3F;
	return bitShiftedValue
		| (repeatValue7 << 3)
		| (repeatValue7 >> 3);
}


uint32 MIDI2Translator::convert7to32(byte value7)
{
	uint32 bitShiftedValue = ((uint32)value7) << 25;
	if (value7 <= 64)
	{
		return bitShiftedValue;
	}
	// use bit repeat bits from extended value7
	uint32 repeatValue7 = value7 & 0x3F;
	return bitShiftedValue
		| (repeatValue7 << 19)
		| (repeatValue7 << 13)
		| (repeatValue7 << 7)
		| (repeatValue7 << 1)
		| (repeatValue7 >> 5);
}


uint32 MIDI2Translator::convert14to32(byte lsb, byte msb)
{
	return convert14to32(((uint16)lsb) | (((uint16)msb) << 7));
}


uint32 MIDI2Translator::convert14to32(uint16 value14)
{
	uint32 bitShiftedValue = ((uint32)value14) << 18;
	if (value14 <= 0x2000)
	{
		return bitShiftedValue;
	}
	// use bit repeat bits from value14
	uint32 repeatValue14 = value14 & 0x1FFF;
	return bitShiftedValue
		| (repeatValue14 << 5)
		| (repeatValue14 >> 8);
}
