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
#include "MIDICIDevice.h"
#include "MIDICIProfiles.h"
#include "MIDICIProfileTypes.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class ProfileComponent
    : public juce::Component
    , protected AsyncUpdater
    , protected TextButton::Listener
    , protected MIDICIProfileList::Listener
{
public:
    //==============================================================================
    ProfileComponent();
    ~ProfileComponent() override;

    void setDevice(MIDICIDevice* device_);

    int getPreferredHeight() const;

    void resized() override;

    // AsyncUpdater
    void handleAsyncUpdate() override;

protected:
    //==============================================================================
    // TextButton::Listener
    void buttonClicked(Button*) override;
    void sendRandomProfileSpecificData();

    // MIDICIProfileList::Listener
    void onProfileAdded(MIDICIProfileList& list, MIDICIProfileState& state) override;
    void onProfileRemoved(MIDICIProfileList& list, MIDICIProfileState& state) override;
    void onProfileCanEnable(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel, /*IN OUT*/bool& newEnabledState) override;
    void onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel) override;

private:
    //==============================================================================

    class ProfileButton
        : public ToggleButton
    {
    public:
        /** @param exclusiveGroup if non-zero, part of a mutually exclusive button group */
        ProfileButton(const MIDICIProfileId& id_, int exclusiveGroup_, MIDIByte channel_ = MIDICI_CHANNEL_PORT, String caption = "");
        const MIDICIProfileId& getId() const { return id; }
        int getExclusiveGroup() const { return exclusiveGroup; }
        MIDIByte getChannel() const { return channel; }

        bool added;
        bool removed;

    private:
        MIDICIProfileId id;
        int exclusiveGroup;
        MIDIByte channel;
    };

    ProfileButton* getProfileButtonById(const MIDICIProfileId& id) const;

    TextButton* buttonProfileSpecificData;
    OwnedArray<ProfileButton> profileButtons;
    MIDICIDevice* device;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProfileComponent)
};
