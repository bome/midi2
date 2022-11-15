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
#include "Logger.h"
#include <stdarg.h>

void MIDI2Logger::log(String message)
{
	for (int i = listeners.size() - 1; i >= 0; i--)
	{
		listeners[i]->onLog(message);
	}
}

#define STRING_FORMAT_MAXBUFFER_FIRST_TRY  (512)
#define STRING_FORMAT_MAXBUFFER_HEAP  (1<<20)

inline String String_format(const char* format, va_list ap)
{
	// first, find out the size of the required buffer (on a copy)
	// and if it fits 512 bytes, use it directly
	String ret;
	ret.preallocateBytes(STRING_FORMAT_MAXBUFFER_FIRST_TRY);
	char* p = const_cast<char*>(ret.toRawUTF8());
	p[0] = 0;
	va_list ap2;
	va_copy(ap2, ap);
	int res = vsnprintf(p, STRING_FORMAT_MAXBUFFER_FIRST_TRY, format, ap2);
	va_end(ap2);
	if (res >= STRING_FORMAT_MAXBUFFER_FIRST_TRY)
	{
		// did not fit...
		res++;
		if (res > STRING_FORMAT_MAXBUFFER_HEAP)
		{
			res = STRING_FORMAT_MAXBUFFER_HEAP;
		}
		ret.preallocateBytes(res);
		p = const_cast<char*>(ret.toRawUTF8());
		p[0] = '\0';
		vsnprintf(p, (size_t)res, format, ap);
	}
	return ret;
}


void MIDI2Logger::logFormatted(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	String formatted = String_format(format, ap);
	va_end(ap);
	log(formatted);
}

MIDI2Logger MIDI2Logger::globalInstance;


