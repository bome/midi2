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

#ifdef _CONSOLE
// for console applications, log using printf
#define LOG(...) printf(__VA_ARGS__); printf("\n")
#else
// define LOG() macro to output using the MIDI2Logger
#define LOG(...) MIDI2Logger::global().logFormatted(__VA_ARGS__)
#endif

// a simplistic logger without concurrency

class MIDI2Logger
{
public:
	static MIDI2Logger& global() { return globalInstance; }

	void log(String message);
	void logFormatted(const char* format, ...);

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void onLog(String message) = 0;
	};

	void addListener(Listener* listener) { listeners.addIfNotAlreadyThere(listener); }
	void removeListener(Listener* listener) { listeners.removeFirstMatchingValue(listener); }

private:
	MIDI2Logger() { /*nothing*/ }
	// note: could also use juce::ListenerList
	Array<Listener*> listeners;
	static MIDI2Logger globalInstance;
};