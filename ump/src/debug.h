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
#ifndef __wtypes_h__
#include <wtypes.h>
#endif
#endif //_WIN32

#include "midi2_support.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
// Windows
#define PRINT(fmt, ...) \
	{ \
		/* quick & dirty, don't use this at home! */ \
		char buf[8192]; \
		sprintf(buf, fmt "\n", __VA_ARGS__); \
		fprintf(stderr, "%s", buf); \
		OutputDebugStringA(buf); \
	}
#define PRINT1 PRINT

#define FILE_LINE_FMT  "%s(%d): %s"

#else
// Non-Windows
#define PRINT(fmt, ...) \
    fprintf(stderr, fmt "\n", __VA_ARGS__)
#define PRINT1(fmt) \
    fprintf(stderr, fmt "\n")

#define FILE_LINE_FMT  "%s:%d %s"
#endif

#define CHECK_EQ(a, b) \
	if (a != b) { \
		PRINT(FILE_LINE_FMT, __FILE__, __LINE__, "failure"); \
        assert(FALSE); \
		return 1; \
	}

