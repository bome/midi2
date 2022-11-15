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
#include "Logger.h"
#include "JuceMidiBridge.h"
#include "MIDICIDevice.h"
#include "MIDICIDiscovery.h"
#include "MIDICIProfiles.h"
#include "ProfileComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent
	: public juce::Component
	, public MIDI2Logger::Listener
    , protected AsyncUpdater
    , protected TextButton::Listener
    , protected MIDICIProfileList::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    void start();
    void stop();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    // AsyncUpdater
    void handleAsyncUpdate() override;

	void log(String message);
	void onLog(String message) override;

    void setScalingFactor(float factor = 1.0f);
    float getScalingFactor() const { return scaleFactor; }

    void mouseWheelMove(const MouseEvent& event,
        const MouseWheelDetails& wheel) override;
    void mouseMagnify(const MouseEvent& event, float scaleFactor_) override;

protected:
    // TextButton::Listener
    void buttonClicked(Button*) override;

    // MIDICIProfileList::Listener
    void onProfileAdded(MIDICIProfileList& list, MIDICIProfileState& state) override;
    void onProfileRemoved(MIDICIProfileList& list, MIDICIProfileState& state) override;
    void onProfileAvailableChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel) override;
    void onProfileCanEnable(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel, /*IN OUT*/bool& newEnabledState) override;
    void onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel) override;
    void onProfileSpecificDataChange(MIDICIProfileList& list, MIDICIProfileState& state) override;

private:
    //==============================================================================
    void logInMainThread(String message);

    // Your private member variables go here...
	TextEditor* logTextEditor;
    String pendingLogMessages;
    CriticalSection pendingLogMessagesLock;

    TextButton* clearButton;
    TextButton* discoveryButton;
    TextButton* newMUIDButton;
    TextButton* profileInquiryButton;

    ProfileComponent* profileComponent;

    // MIDI-CI
    JuceMidiBridge bridge;
    MIDICIDevice device;
    MIDICIDiscovery discovery;
    MIDICIProfiles profiles;
    bool pendingStart;
    float scaleFactor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
