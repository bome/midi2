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
#include "../src/midi2_apple_output.h"

#define VIRTUAL_PORT_NAME  "UMP Demo"

/**
 * @return the user-entered number, or (min-1) if user pressed ENTER
 */
int promptForNumber(int min, int max)
{
    int num = min - 1;
    while (true)
    {
        printf("Please enter a number from %d to %d, or ENTER to create virtual port: ", min, max);
        char bf[20];
        fgets(bf, sizeof(bf) - 1, stdin);
        if (bf[0] <= 32)
        {
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


UMPacket getRandomMessage()
{
#define RANDOM_MESSAGE_COUNT  10
    
    int msgIndex = brandom(0, RANDOM_MESSAGE_COUNT - 1);
    // group
    uint4 g = brandom(0, 15);
    // channel
    uint4 c = brandom(0, 15);
    // noteNumber
    uint7 n = brandom(0, 127);
    
    switch (msgIndex)
    {
        case 0: return UMPacket().initNoteOn(g, c, n, brandom16() /*velocity*/);
        case 1: return UMPacket().initNoteOn(g, c, n, brandom16() /*velocity*/, brandom8() /*attributeType*/, brandom16() /*attribute*/);

        case 2: return UMPacket().initNoteOff(g, c, n, brandom16() /*velocity*/);
        case 3: return UMPacket().initNoteOff(g, c, n, brandom16() /*velocity*/, brandom8() /*attributeType*/, brandom16() /*attribute*/);
        
        case 4: return UMPacket().initPitchBend(g, c, brandom32()/*value*/);

        case 5: return UMPacket().initAssignableCC(g, c, brandom(0,127)/*bank*/, brandom(0, 127)/*index*/, brandom32()/*value*/);
        case 6: return UMPacket().initRegisteredCC(g, c, brandom(0,127)/*bank*/, brandom(0, 127)/*index*/, brandom32()/*value*/);

        case 7: return UMPacket().initPerNoteAssignableCC(g, c, n, brandom(0, 127)/*index*/, brandom32()/*value*/);
        case 8: return UMPacket().initPerNoteRegisteredCC(g, c, n, brandom(0, 127)/*index*/, brandom32()/*value*/);
        case 9: return UMPacket().initPerNoteManagement(g, c, n, brandom(0, 3)/*optionFlags*/);
    }
    
    return UMPacket().initNoteOn(g, c, brandom(0, 127)/*noteNumber*/, brandom16() /*velocity*/);
}



int main(int argc, char** argv)
{
    char textBuffer[1024];
    
    MIDI2AppleOutput outputDevice;
    
    int deviceCount = outputDevice.getDeviceCount();
    
    PRINT1("Available MIDI OUTPUT devices:");
    int actualCount = 0;
    for (int devid = 0; devid < deviceCount; devid++)
    {
        const char* name = outputDevice.getDeviceName(devid, textBuffer, sizeof(textBuffer));
        if (name != nullptr)
        {
            PRINT("%2d: %s", devid, name);
            actualCount++;
        }
    }

    int selectedPortIndex = -1;

    if (actualCount == 0)
    {
        PRINT1("NOTE: no MIDI OUTPUT devices available.");
    }
    else
    {
        selectedPortIndex = promptForNumber(0, deviceCount - 1);
    }
    
    bool ok;
    if (selectedPortIndex >= 0)
    {
        ok = outputDevice.open(selectedPortIndex);
    }
    else
    {
        PRINT("Using virtual output port \"%s\".", VIRTUAL_PORT_NAME);
        ok = outputDevice.openVirtualPort(VIRTUAL_PORT_NAME);
    }

    if (ok)
    {
        PRINT1("Press [ENTER] to send a UMP message.");
        PRINT1("Enter 'q' to quit.");
        char bf[200];
        while (true)
        {
            fgets(bf, sizeof(bf) - 1, stdin);
            if (bf[0] == 'q')
            {
                break;
            }
            UMPacket packet(getRandomMessage());
            PRINT("%s", packet.toString());
            outputDevice.send(packet);
        }
    }
 
    outputDevice.close();
    
    return 0;
}
