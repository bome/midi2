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
#include "ProfileComponent.h"
#include "Logger.h"

#define PROFILECOMPONENT_LINEHEIGHT   24
#define PROFILECOMPONENT_GAP          4

ProfileComponent::ProfileComponent()
    : device(nullptr)
{
    buttonProfileSpecificData = new TextButton("specific data");
    buttonProfileSpecificData->changeWidthToFitText(PROFILECOMPONENT_LINEHEIGHT);
    buttonProfileSpecificData->addListener(this);
    addAndMakeVisible(buttonProfileSpecificData);
}

ProfileComponent::~ProfileComponent()
{
    setDevice(nullptr);
    delete buttonProfileSpecificData;
}

void ProfileComponent::setDevice(MIDICIDevice* device_)
{
    if (device != nullptr)
    {
        device->getLocalInfo().profiles.removeListener(this);
    }
    device = device_;
    if (device != nullptr)
    {
        device->getLocalInfo().profiles.addListener(this);
    }
}

int ProfileComponent::getPreferredHeight() const
{
    return (2 * PROFILECOMPONENT_LINEHEIGHT) + PROFILECOMPONENT_GAP;
}

void ProfileComponent::resized()
{
    int x = 0;
    int y = 0;

    for (auto b : profileButtons)
    {
        if (y == 0 && x + b->getWidth() > getWidth())
        {
            // 2nd row
            y += PROFILECOMPONENT_LINEHEIGHT + PROFILECOMPONENT_GAP;
            x = 0;
        }
        b->setTopLeftPosition(x, y);
        x += b->getWidth() + PROFILECOMPONENT_GAP;
    }

    buttonProfileSpecificData->setTopRightPosition(getWidth(), getHeight() - buttonProfileSpecificData->getHeight());
}

void ProfileComponent::handleAsyncUpdate()
{
    const MIDICIProfileList& profiles = device->getLocalInfo().profiles;
    bool relayout = false;

    for (int i = profileButtons.size() - 1; i >= 0; i--)
    {
        ProfileButton* b = profileButtons.getUnchecked(i);
        if (b->added)
        {
            b->added = false;
            b->setSize(100, PROFILECOMPONENT_LINEHEIGHT);
            b->changeWidthToFitText();
            b->addListener(this);
            addAndMakeVisible(b);
            relayout = true;
        }
        if (b->removed)
        {
            profileButtons.remove(i);
            relayout = true;
        }
        else
        {
            b->setToggleState(profiles.getProfile(b->getId()).isChannelEnabled(b->getChannel()), dontSendNotification);
        }
    }
    if (relayout)
    {
        resized();
        repaint();
    }
}

void ProfileComponent::buttonClicked(Button* button)
{
    if (button == buttonProfileSpecificData)
    {
        sendRandomProfileSpecificData();
    }
    else
    {
        ProfileButton* b = (ProfileButton*)button;
        device->getLocalInfo().profiles.getProfile(b->getId()).setChannelEnabled(b->getChannel(), b->getToggleState());
    }
}


void ProfileComponent::sendRandomProfileSpecificData()
{
    bool found = false;
    MIDICIDeviceInfo* info = device->getMostRecentRemoteInfo();
    if (info != nullptr && info->profiles.getProfileCount() > 0)
    {
        Random rng(Time::currentTimeMillis());
        // 5..50 bytes random data
        MemoryBlock randomData(((rng.nextInt() & 0xFFFF) % 45) + 5);
        MIDIByte* data = (MIDIByte*)randomData.getData();
        for (int x = 0; x < randomData.getSize(); x++)
        {
            // 7-bit
            data[x] = rng.nextInt() & 0x7F;
        }
        int index = (rng.nextInt() & 0xFFFF) % info->profiles.getProfileCount();
        MIDICIProfileState& profile = info->profiles.getProfile(index);
        LOG("Send random profile specific data to most recent remote profile on randomly selected remote device %s", MUID2STRING(info->getMUID()));
        profile.setSpecificData(randomData);
        found = true;
    }
    if (!found)
    {
        LOG("No remote device with known profiles found.");
    }
}


void ProfileComponent::onProfileAdded(MIDICIProfileList& /*list*/, MIDICIProfileState& state)
{
    // if button does not exist, add it
    if (getProfileButtonById(state.getId()) == nullptr)
    {
        int exclusiveGroup = ((profileButtons.size() >= 2) && (profileButtons.size() <= 4)) ? 1 : 0;
        MIDIByte channel = MIDICI_CHANNEL_PORT;
        if (!state.isChannelAvailable(channel))
        {
            channel = state.getFirstAvailableChannel();
        }
        ProfileButton* b = new ProfileButton(state.getId(), exclusiveGroup, channel);
        b->added = true;
        profileButtons.add(b);
        // adding to the GUI must be done in GUI thread
        triggerAsyncUpdate();
    }
}

void ProfileComponent::onProfileRemoved(MIDICIProfileList& /*list*/, MIDICIProfileState& state)
{
    ProfileButton* b = getProfileButtonById(state.getId());
    if (b != nullptr)
    {
        // do the actual deletion in GUI thread
        b->removed = true;
        triggerAsyncUpdate();
    }
}

void ProfileComponent::onProfileCanEnable(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel, bool& /*newEnabledState*/)
{
    // check if this is a mutually exclusive profile
    ProfileButton* b = getProfileButtonById(state.getId());
    if (b != nullptr && b->getExclusiveGroup() != 0)
    {
        // turn off any other mutually exclusive profiles
        for (auto other : profileButtons)
        {
            if (other->getExclusiveGroup() == b->getExclusiveGroup()
                && b != other)
            {
                list.getProfile(other->getId()).setChannelEnabled(channel, false);
            }
        }
    }
    // allow newEnabledState
}

void ProfileComponent::onProfileEnabledChange(MIDICIProfileList& /*list*/, MIDICIProfileState& /*state*/, MIDIByte /*channel*/)
{
    triggerAsyncUpdate();
}

ProfileComponent::ProfileButton* ProfileComponent::getProfileButtonById(const MIDICIProfileId& id) const
{
    for (auto b : profileButtons)
    {
        if (b->getId() == id)
        {
            return b;
        }
    }
    return nullptr;
}


//
// ProfileButton
//

ProfileComponent::ProfileButton::ProfileButton(const MIDICIProfileId& id_, int exclusiveGroup_, MIDIByte channel_ /*=MIDICI_CHANNEL_PORT*/, String caption /* ="" */)
    : ToggleButton(caption.isEmpty() ? id_.toRawString() : caption)
    , added(false)
    , removed(false)
    , id(id_)
    , exclusiveGroup(exclusiveGroup_)
    , channel(channel_)
{
    // nothing
}
