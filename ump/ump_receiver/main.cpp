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
#include "../src/debug.h"
#include "../src/midi2.h"
#include "../src/midi2_printer.h"
#include "../src/midi2_apple_util.h"
#include "../src/midi2_apple_input.h"


/**
 * @return the user-entered number, or (min-1) if user pressed ENTER
 */
int promptForNumber(int min, int max)
{
    int num = min - 1;
    while (true)
    {
        printf("Please enter a number from %d to %d, or ENTER to abort: ", min, max);
        char bf[20];
        fgets(bf, sizeof(bf) - 1, stdin);
        if (bf[0] <= 32)
        {
            PRINT1("\nAborted.");
            return min - 1;
        }
        num = atoi( bf );
        if (num < min || num > max)
        {
            PRINT1("ERROR: not a valid number.");
        }
        else
        {
            break;
        }
    }
    return num;
}


int main(int argc, char** argv)
{
    char textBuffer[1024];
    
    MIDI2AppleInput inputDevice;
    
    int deviceCount = inputDevice.getDeviceCount();
    
    if (deviceCount == 0)
    {
        PRINT1("Please make sure there are MIDI devices available:");
        PRINT1("- plug in a MIDI device");
        PRINT1("- start a network MIDI connection");
        PRINT1("- launch a program which exposes MIDI ports");
        return 1;
    }
    
    PRINT1("Available MIDI INPUT devices:");
    int actualCount = 0;
    for (int devid = 0; devid < deviceCount; devid++)
    {
        const char* name = inputDevice.getDeviceName(devid, textBuffer, sizeof(textBuffer));
        if (name != nullptr)
        {
            PRINT("%2d: %s", devid, name);
            actualCount++;
        }
    }

    if (actualCount == 0)
    {
        PRINT1("ERROR: no MIDI INPUT devices available.");
        return 1;
    }

    int selectedPortIndex = promptForNumber(0, deviceCount - 1);
    
    MIDI2Printer midi2printer;

    if (selectedPortIndex >= 0)
    {
        if (inputDevice.open(selectedPortIndex, &midi2printer))
        {
#if 1
            PRINT1("Now printing incoming UMP packets. [ENTER] to quit.");
            char bf[200];
            fgets(bf, sizeof(bf) - 1, stdin);
#else
            PRINT1("Now printing incoming UMP packets for 30 seconds.");
            NSRunLoop* myRunLoop = [NSRunLoop currentRunLoop];
            NSInteger    loopCount = 30;
            do
            {
                // Run the run loop 30 times to let the timer fire.
                [myRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
                loopCount--;
            }
            while (loopCount);
#endif
        }
    }
 
    inputDevice.close();
    
    return 0;
}
