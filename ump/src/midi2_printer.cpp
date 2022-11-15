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
#include "midi2_printer.h"

#include <stdio.h>


const char* getMIDI1MessageDescription(const byte* data, int length)
{
    static char msg[100];
    switch (length)
    {
    case 1:
        sprintf(msg, "%02X", data[0]);
        break;
    case 2:
        sprintf(msg, "%02X %02X", data[0], data[1]);
        break;
    case 3:
        sprintf(msg, "%02X %02X %02X", data[0], data[1], data[2]);
        break;
    default:
        sprintf(msg, "(%d bytes)", length);
        break;
    }
    return msg;
}


MIDI2Printer::MIDI2Printer()
    : MIDI2Processor()
    , firstTimestamp(0)
{
    // nothing
}

void MIDI2Printer::process(uint64 timestamp, const UMPacket &packet)
{
    if (firstTimestamp == 0)
    {
        firstTimestamp = timestamp;
    }
    printf("%6llu: %s\n", (timestamp - firstTimestamp) / 1000000L, packet.toString());
}

void MIDI2Printer::onCorruptRawData(const char* errorMessage)
{
    fprintf(stderr, "%s\n", errorMessage);
}

