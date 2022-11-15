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
#include "MainComponent.h"
#include "Constants.h"

//==============================================================================
MainComponent::MainComponent()
	: pendingStart(true)
	, scaleFactor(1.0f)
{
	clearButton = new TextButton("clear");
	clearButton->changeWidthToFitText(24);
	clearButton->addListener(this);
	addAndMakeVisible(clearButton);

	discoveryButton = new TextButton("discover");
	discoveryButton->changeWidthToFitText(24);
	discoveryButton->addListener(this);
	addAndMakeVisible(discoveryButton);

	newMUIDButton = new TextButton("new MUID");
	newMUIDButton->changeWidthToFitText(24);
	newMUIDButton->addListener(this);
	addAndMakeVisible(newMUIDButton);

	profileInquiryButton = new TextButton("inquire profiles");
	profileInquiryButton->changeWidthToFitText(24);
	profileInquiryButton->addListener(this);
	addAndMakeVisible(profileInquiryButton);

	profileComponent = new ProfileComponent();
	addAndMakeVisible(profileComponent);


	logTextEditor = new TextEditor("log");
	logTextEditor->setReadOnly(true);
	logTextEditor->setMultiLine(true);
	logTextEditor->setScrollbarsShown(true);
	logTextEditor->setCaretVisible(false);
	logTextEditor->setPopupMenuEnabled(true);
	addAndMakeVisible(logTextEditor);

	MIDI2Logger::global().addListener(this);
	setSize(600, 400);

	// asynchronously start
	triggerAsyncUpdate();
}

MainComponent::~MainComponent()
{
	stop();
	MIDI2Logger::global().removeListener(this);
	deleteAllChildren();
}

void MainComponent::start()
{
	// now wire all MIDI-CI components
	bridge.setReceiver(&device);
	bridge.setMIDIPortNames(MIDI_INPUT_PORT_NAME, MIDI_OUTPUT_PORT_NAME);
	
	device.setSender(&bridge);
	device.addHandler(&discovery);

	profileComponent->setDevice(&device);
	device.getLocalInfo().profiles.addListener(this);
	device.addRemoteProfileListener(this);
	device.addHandler(&profiles);

	if (bridge.start())
	{
		device.start();
		LOG("%s v%s started.", ProjectInfo::projectName, ProjectInfo::versionString);
	}
	else
	{
		LOG("Errors occurred. Fix the problem, then restart.");
	}

	// now add a few profiles for fun
	device.getLocalInfo().profiles.addProfile(MIDICI_CHANNEL_PORT, true/*enabled*/, MIDICIProfileId(0x7D, 0, 0, 12, 1));
	device.getLocalInfo().profiles.addProfile(MIDICI_CHANNEL_PORT, false/*enabled*/, MIDICIProfileId(0x00, 0x21, 0x32, 20, 4));
	device.getLocalInfo().profiles.addProfile(3, false/*enabled*/, MIDICIProfileId(0x7D, 0, 0, 64, 0));
	device.getLocalInfo().profiles.addProfile(3, false/*enabled*/, MIDICIProfileId(0x7D, 0, 0, 65, 0));
	device.getLocalInfo().profiles.addProfile(3, false/*enabled*/, MIDICIProfileId(0x7D, 0, 0, 66, 0));

	// also add a remote info for testing
	//device.addRemoteInfo(MIDICIDeviceInfo(1234567));
	//device.getRemoteInfo(1234567).profiles.addProfile(MIDICI_CHANNEL_PORT, true/*enabled*/, MIDICIProfileId(0x7D, 0, 0, 60, 50));

}

void MainComponent::stop()
{
	profileComponent->setDevice(nullptr);
	device.stop();
	bridge.stop();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
	float border = (10 / scaleFactor);
	float gap = 1.0f;
	float buttonGap = 4.0f;
	float y = border;
	float clientWidth = (getWidth() / scaleFactor) - border - border;
	float height = getHeight() / scaleFactor;

	clearButton->setTopLeftPosition((int)border, (int)y);
	discoveryButton->setTopLeftPosition((int)(clearButton->getRight() + buttonGap), (int)y);
	newMUIDButton->setTopLeftPosition((int)(discoveryButton->getRight() + buttonGap), (int)y);
	profileInquiryButton->setTopLeftPosition((int)(newMUIDButton->getRight() + buttonGap), (int)y);

	y += discoveryButton->getHeight() + gap;

	profileComponent->setBounds((int)border, (int)y, (int)clientWidth, profileComponent->getPreferredHeight());
	y += profileComponent->getHeight() + gap;


	logTextEditor->setBounds((int)border, (int)y, (int)clientWidth, (int)(height - border - y));

}


void MainComponent::log(String message)
{
	if (MessageManager::existsAndIsCurrentThread() && logTextEditor != nullptr)
	{
		logInMainThread(message);
	}
	else
	{
		ScopedLock lock(pendingLogMessagesLock);
		pendingLogMessages += message + juce::newLine;
		triggerAsyncUpdate();
	}
}

void MainComponent::onLog(String message)
{
	log(message);
}

void MainComponent::setScalingFactor(float factor)
{
	AffineTransform transform;
	if (factor < 0.4f)
	{
		factor = 0.4f;
	}
	if (factor != 1.0f)
	{
		transform = transform.scaled(factor, factor);
	}
	for (int i = 0; i < getNumChildComponents(); i++)
	{
		getChildComponent(i)->setTransform(transform);
	}
	scaleFactor = factor;
	resized();
}

void MainComponent::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
#ifdef _WINDOWS
	if (event.mods.isCtrlDown())
#else
	if (event.mods.isCommandDown())
#endif
	{
		if (wheel.deltaY > 0)
		{
			setScalingFactor(scaleFactor + 0.1f);
		}
		else if (wheel.deltaY < 0)
		{
			setScalingFactor(scaleFactor - 0.1f);
		}
	}
	else
	{
		Component::mouseWheelMove(event, wheel);
	}
}

void MainComponent::mouseMagnify(const MouseEvent&, float scaleFactor_)
{
	if (scaleFactor_ > 1.0f)
	{
		setScalingFactor(scaleFactor + scaleFactor_ - 1.0f);
	}
	else
	{
		setScalingFactor(scaleFactor - (1.0f - scaleFactor_));
	}
}

void MainComponent::logInMainThread(String message)
{
	if (logTextEditor != nullptr)
	{
		logTextEditor->moveCaretToEnd();
		if (message.endsWith(juce::newLine))
		{
			logTextEditor->insertTextAtCaret(message);
		}
		else
		{
			logTextEditor->insertTextAtCaret(message + juce::newLine);
		}
	}
}

void MainComponent::buttonClicked(Button* button)
{
	if (button == clearButton)
	{
		logTextEditor->clear();
	}
	else if (button == discoveryButton)
	{
		discovery.triggerDiscovery();
	}
	else if (button == newMUIDButton)
	{
		discovery.generateNewRandomMUID();
	}
	else if (button == profileInquiryButton)
	{
		for (int i = 0; i < device.getRemoteInfoCount(); i++)
		{
			profiles.triggerProfileInquiry(device.getRemoteInfoByIndex(i).getMUID());
		}
		if (device.getRemoteInfoCount() == 0)
		{
			LOG("Cannot send profile inquiry: no remote devices discovered.");
		}
	}
}

void MainComponent::onProfileAdded(MIDICIProfileList& list, MIDICIProfileState& state)
{
	if (device.isLocalMUID(list.getMUID()))
	{
		LOG("Callback: added local profile: '%s'", PROFILEID2STRING(state.getId()));
	}
	else
	{
		LOG("Callback: remote MUID %s added profile '%s'", MUID2STRING(list.getMUID()), PROFILEID2STRING(state.getId()));
	}
}

void MainComponent::onProfileRemoved(MIDICIProfileList& list, MIDICIProfileState& state)
{
	if (device.isLocalMUID(list.getMUID()))
	{
		LOG("Callback: removed local profile '%s'", PROFILEID2STRING(state.getId()));
	}
	else
	{
		LOG("Callback: remote MUID %s removed profile '%s'", MUID2STRING(list.getMUID()), PROFILEID2STRING(state.getId()));
	}
}

void MainComponent::onProfileAvailableChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel)
{
	if (device.isLocalMUID(list.getMUID()))
	{
		LOG("Callback: local profile '%s' on %s is now %s", PROFILEID2STRING(state.getId()), CHANNEL2STRING(channel), state.isChannelAvailable(channel) ? "available" : "unavailable");
	}
	else
	{
		LOG("Callback: remote MUID %s profile '%s' on %s is now %s", MUID2STRING(list.getMUID()), PROFILEID2STRING(state.getId()), CHANNEL2STRING(channel), state.isChannelAvailable(channel) ? "available" : "unavailable");
	}
}

void MainComponent::onProfileCanEnable(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel, bool& newEnabledState)
{
	if (device.isLocalMUID(list.getMUID()))
	{
		LOG("Callback: can %s local profile '%s' on %s?", newEnabledState ? "enable" : "disable", PROFILEID2STRING(state.getId()), CHANNEL2STRING(channel));
	}
	else
	{
		LOG("Callback: MUID %s can %s profile '%s' on %s?", MUID2STRING(list.getMUID()), newEnabledState ? "enable" : "disable", PROFILEID2STRING(state.getId()), CHANNEL2STRING(channel));
	}
}

void MainComponent::onProfileEnabledChange(MIDICIProfileList& list, MIDICIProfileState& state, MIDIByte channel)
{
	if (device.isLocalMUID(list.getMUID()))
	{
		LOG("Callback: local profile '%s' on %s now %s", PROFILEID2STRING(state.getId()), CHANNEL2STRING(channel), state.isChannelEnabled(channel) ? "enabled" : "disabled");
	}
	else
	{
		LOG("Callback: MUID %s profile '%s' on %s now %s", MUID2STRING(list.getMUID()), PROFILEID2STRING(state.getId()), CHANNEL2STRING(channel), state.isChannelEnabled(channel) ? "enabled" : "disabled");
	}
}

void MainComponent::onProfileSpecificDataChange(MIDICIProfileList& list, MIDICIProfileState& state)
{
	if (device.isLocalMUID(list.getMUID()))
	{
		LOG("Callback: specific data changed for local profile '%s'", PROFILEID2STRING(state.getId()));
	}
	else
	{
		LOG("Callback: MUID %s specific data changed for profile '%s'", MUID2STRING(list.getMUID()), PROFILEID2STRING(state.getId()));
	}
}

void MainComponent::handleAsyncUpdate()
{
	if (pendingStart)
	{
		pendingStart = false;
		start();
	}

	while (pendingLogMessages.isNotEmpty())
	{
		String message;
		{
			ScopedLock lock(pendingLogMessagesLock);
			message = pendingLogMessages;
			pendingLogMessages.clear();
		}
		log(message);
	}
}

