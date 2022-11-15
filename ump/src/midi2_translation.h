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

#include "midi2.h"


class MIDI2Translator
{
public:
	class Listener;

	MIDI2Translator();
	MIDI2Translator(Listener* listener);

	void setListener(Listener* listener);
	Listener* getListener() const;

	/** set the MIDI 2.0 Group for MIDI 2.0 messages sent to the listener */
	void setTranslateToMIDI2Group(uint4 group);
	/** get the MIDI 2.0 Group used for MIDI 2.0 messages sent to the listener */
	int getTranslateToMIDI2Group() const;

	/** On which MIDI 2 group are messages translated to MIDI 1? -1 means all groups. */
	void setTranslateFromMIDI2Group(uint4 group);
	/** On which MIDI 2 group are messages translated to MIDI 1? -1 means all groups. */
	int getTranslateFromMIDI2Group() const;

	// ---------------------------------------

	/**
	 * convert the given MIDI 1.0 message to MIDI 2.0.
	 * The MIDI 2 message will be sent to the listener's translatedMessage() method.
	 *
	 * @return TRUE if message was processed
	 */
	bool midi1Received(const byte* message, int length);

	/**
	 * Convert the given UMP packet to MIDI.
	 * The MIDI message will be sent to the listener's translatedMessage() method.
	 *
	 * @return TRUE if message was processed
	 */
	bool umpReceived(const UMPacket& packet);

	// ---------------------------------------

	static uint16 convert7to16(byte value7);
	static uint32 convert7to32(byte value7);
	static uint32 convert14to32(byte lsb, byte msb);
	static uint32 convert14to32(uint16 value14);

	static byte convert16to7(uint16 value16) { return (byte)(value16 >> 9); }
	static byte convert32to7(uint32 value32) { return (byte)(value32 >> 25); }
	static uint16 convert16to14(uint16 value16) { return (value16 >> 2); }
	static byte convert16to14_MSB(uint16 value16) { return (byte)(value16 >> 9); }
	static byte convert16to14_LSB(uint16 value16) { return (byte)((value16 >> 2) & 0x7F); }
	static uint16 convert32to14(uint32 value32) { return (uint16)(value32 >> 18); }
	static byte convert32to7_MSB(uint32 value32) { return (byte)(value32 >> 25); }
	static byte convert32to7_LSB(uint32 value32) { return (byte)((value32 >> 18) & 0x7F); }

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void translatedMessage(const UMPacket& packet) = 0;
		virtual void translatedMessage(const byte* data, int length, uint4 midi2Group) = 0;
	};

private:
	void init();
	bool midi1ControlChangeReceived(uint4 channel, uint7 index, uint7 value);

	Listener* listener;
	int translateToMIDI2Group;
	int translateFromMIDI2Group;
	byte bankChangeLSB, bankChangeMSB;
	uint32 bankChangeLSBtime, bankChangeMSBtime;
	
	enum RuntimeFlags {
		// runtime flags
		receivedRPN = 1 << 0, // we're currently receiving RPN's
		receivedNRPN = 1 << 1, // we're currently receiving NRPN's
		receivedNRPNValueMSB = 1 << 2, // (N)RPN value
		receivedNRPNParamMSB = 1 << 3, // (N)RPN param#
		receivedNRPNParamLSB = 1 << 4, // (N)RPN param#
	};
	uint32 runtimeFlags[MIDI_CHANNEL_COUNT]; // OR'ed RuntimeFlags (for each channel)
	byte paramNRPN_MSB[MIDI_CHANNEL_COUNT]; // for (N)RPN
	byte paramNRPN_LSB[MIDI_CHANNEL_COUNT]; // for (N)RPN
	byte valueNRPN_MSB[MIDI_CHANNEL_COUNT]; // value MSB for (N)RPN
};
