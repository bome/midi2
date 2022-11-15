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
#include "midi2_support.h"
#include <time.h> // initializing rand()
#include <stdlib.h>

#ifdef TARGET_WIN
#define _CRT_SECURE_NO_WARNINGS
#ifndef __wtypes_h__
#include <wtypes.h>
#endif
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

#ifdef TARGET_APPLE
#include <mach/mach_time.h>
#include <sys/time.h>
#endif

#ifdef TARGET_WIN
uint32 getMilliTime()
{
	static uint32 startTime = 0;
	if (startTime == 0) {
		startTime = timeGetTime();
		// get millisecond time with millisecond resolution
		timeBeginPeriod(1);
	}
	return timeGetTime() - startTime;
}


#elif defined(TARGET_APPLE)

uint32 getMilliTime()
{
	static uint64 startTick = 0;
	static double factor = 0.0;
	if (startTick == 0)
	{
		mach_timebase_info_data_t tinfo;
		kern_return_t kerror = mach_timebase_info(&tinfo);
		if (kerror == KERN_SUCCESS)
		{
			startTick = mach_absolute_time();
			// tick to millis
			factor = ((double)tinfo.numer / tinfo.denom) / 1000000.0;
			return 0;
		}
	}
	else
	{
		return (uint32)((mach_absolute_time() - startTick) * factor);
	}
    // fallback
    struct timeval t;
    gettimeofday(&t, 0);
    static time_t startSeconds = t.tv_sec;
    return (uint32)(((t.tv_sec - startSeconds) * 1000) + (t.tv_usec / 1000));
}


#elif defined(TARGET_LINUX)

uint32 getMilliTime()
{
	static BBool supportsMonotonicClock = TRUE;
	if (supportsMonotonicClock)
	{
		struct timespec ts;
		if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		{
			static time_t tsStartSeconds = ts.tv_sec;
			return (uint32)(((ts.tv_sec - tsStartSeconds) * 1000) + (ts.tv_nsec / 1000000));
		}
		else
		{
			supportsMonotonicClock = FALSE;
		}
	}

	// fallback
	struct timeval t;
	gettimeofday(&t, 0);
	static time_t startSeconds = t.tv_sec;
	return (uint32)(((t.tv_sec - startSeconds) * 1000) + (t.tv_usec / 1000));
}
#endif


int brandom()
{
    static bool inited = FALSE;
    if (!inited) {
        srand((int)time(NULL));
        inited = true;
    }
    return rand();
}

int brandom_max()
{
    return RAND_MAX;
}

int brandom(int min, int max)
{
    if (min >= max)
    {
        return min;
    }
    uint32 range = (max - min);

    uint32 rnd;
    if (range > RAND_MAX)
    {
        if (range > 0xFFFF)
        {
            rnd = brandom32();
        }
        else
        {
            rnd = brandom16();
        }
    }
    else
    {
        rnd = (uint32)brandom();
    }
    return ((int)(rnd % (range + 1))) + min;
}


uint8 brandom8()
{
    return (uint8)(brandom() & 0xFF);
}


uint16 brandom16()
{
#if RAND_MAX < 0xFFFF
    return (uint16)(((brandom() & 0xFF) << 8) | (brandom() & 0xFF));
#else
    return (uint16)(brandom() & 0xFFFF);
#endif
}

uint32 brandom32()
{
#if RAND_MAX == 0xFFFFFFFF
    return (uint32)brandom();
#elif RAND_MAX >= 0xFFFF
    return (uint32)((brandom() & 0xFFFF) << 16) | (brandom() & 0xFFFF);
#elif RAND_MAX >= 0x7FF
    return (uint32)(((brandom() & 0x7FF) << 22) | ((brandom() & 0x7FF) << 11) | (brandom() & 0x7FF));
#else
    return (uint32)(((brandom() & 0xFF) << 24) | ((brandom() & 0xFF) << 16) | ((brandom() & 0xFF) << 8) | (brandom() & 0xFF));
#endif
}
