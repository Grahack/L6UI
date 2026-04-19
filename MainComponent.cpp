#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(1000, 1000);

    juce::Colour dark = juce::Colour (40, 40, 40);
    juce::Colour l6 = customLookAndFeel.l6;

    getLookAndFeel().setColour(juce::Slider::thumbColourId, l6);
    getLookAndFeel().setColour(juce::Slider::textBoxTextColourId,
                               juce::Colours::white);
    getLookAndFeel().setColour(juce::Slider::textBoxBackgroundColourId, dark);
    getLookAndFeel().setColour(juce::Slider::trackColourId,
                               juce::Colours::darkgrey);
    getLookAndFeel().setColour(juce::Slider::backgroundColourId,
                               juce::Colours::lightgrey);
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, dark);
    customLookAndFeel.setColour(juce::Label::backgroundColourId, l6);
    customLookAndFeel.setColour(juce::Label::textColourId,
                               juce::Colours::black);

    // MIDI channel
    channel = 0;
    channelSelector.addListener(this);
    channelSelector.setJustificationType(juce::Justification::centred);
    channelSelector.setColour(ComboBox::ColourIds::backgroundColourId, dark);
    addAndMakeVisible(channelSelector);
    for (int i = 0; i < 16; i++)
    {
        channelSelector.addItem("CHAN " + std::to_string(i), i+1);
    }
    channelSelector.setSelectedId(1);

    // MIDI in/out
    midiInputSelector.addListener(this);
    midiInputSelector.setColour(ComboBox::ColourIds::backgroundColourId, dark);
    addAndMakeVisible(midiInputSelector);
    midiOutputSelector.addListener(this);
    midiOutputSelector.setColour(ComboBox::ColourIds::backgroundColourId, dark);
    addAndMakeVisible(midiOutputSelector);
    refreshMidiPorts();
    refreshButton.setButtonText("REFRESH");
    refreshButton.addListener(this);
    addAndMakeVisible(refreshButton);

    // tracks section
    // See controllers section in MainComponent.h
    for (int i = 0; i < slidersCount; i++)
    {
        slidersArray.add(new juce::Slider());
        slidersArray[i]->addListener(this);
        slidersArray[i]->setRange(0, 127, 1);
        slidersArray[i]->setNumDecimalPlacesToDisplay(0);
        addAndMakeVisible(*slidersArray[i]);
        if ((i+1) % 9 == 0)
        {
            slidersArray[i]->setSliderStyle(juce::Slider::LinearVertical);
            slidersArray[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
        } else {
            slidersArray[i]->setSliderStyle(juce::Slider::Rotary);
            slidersArray[i]->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
            slidersArray[i]->setValue(64);
            slidersArray[i]->textFromValueFunction = [](double value)
            {
                int v = static_cast<int>(value);
                if (v == 64) return juce::String("C"); // centre
                if (v < 64) return "L" + juce::String(64 - v);
                return "R" + juce::String(v - 64);
            };
        }
    }
    for (int i = 0; i < 6; i++)
    {
        mutesArray.add(new juce::TextButton(std::to_string(i+1)));
        mutesArray[i]->setClickingTogglesState(true);
        mutesArray[i]->setToggleState(true, juce::dontSendNotification);
        mutesArray[i]->setColour(juce::TextButton::buttonColourId,
                                 juce::Colours::darkred);
        mutesArray[i]->setColour(juce::TextButton::buttonOnColourId,
                                 juce::Colours::darkgreen);
        mutesArray[i]->addListener(this);
        addAndMakeVisible(*mutesArray[i]);
    }
    // global section
    // scenes
    const char* scenesTxt[] = {"A", "B", "C"};
    for (int i = 0; i < 3; i++)
    {
        scenesArray.add(new juce::TextButton(scenesTxt[i]));
        scenesArray[i]->setColour(juce::TextButton::buttonColourId,
                                  juce::Colours::darkred);
        scenesArray[i]->setColour(juce::TextButton::buttonOnColourId,
                                  juce::Colours::darkgreen);
        scenesArray[i]->addListener(this);
        addAndMakeVisible(*scenesArray[i]);
    }
    scenesArray[0]->setToggleState(true, juce::dontSendNotification);
    // fx
    const char* fxTxt[] = {"HALL", "ROOM", "SPRG", "DEL", "ECHO"};
    for (int i = 0; i < 5; i++)
    {
        fxArray.add(new juce::TextButton(fxTxt[i]));
        fxArray[i]->setColour(juce::TextButton::buttonColourId,
                              juce::Colours::darkred);
        fxArray[i]->setColour(juce::TextButton::buttonOnColourId,
                              juce::Colours::darkgreen);
        fxArray[i]->addListener(this);
        addAndMakeVisible(*fxArray[i]);
    }
    fxArray[0]->setToggleState(true, juce::dontSendNotification);
    // compression
    compButton.setButtonText("COMP");
    compButton.setClickingTogglesState(true);
    compButton.setToggleState(false, juce::dontSendNotification);
    compButton.setColour(juce::TextButton::buttonColourId,
                         juce::Colours::darkgrey);
    compButton.setColour(juce::TextButton::buttonOnColourId,
                             juce::Colours::darkgreen);
    compButton.addListener(this);
    addAndMakeVisible(compButton);

    // First drawing request because previous ones were aborted
    // because of arrays not full of what we needed.
    resized();
}

MainComponent::~MainComponent()
{
    channelSelector.removeListener(this);
    midiInputSelector.removeListener(this);
    midiOutputSelector.removeListener(this);
    refreshButton.removeListener(this);
    midiOut.reset(); // Closes the MIDI out port
    for (int i = 0; i < slidersCount; i++)
        slidersArray[i]->removeListener(this);
    for (int i = 0; i < 3; i++)
        scenesArray[i]->removeListener(this);
    for (int i = 0; i < 5; i++)
    {
        fxArray[i]->removeListener(this);
    }
    compButton.removeListener(this);

    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
    int internalMargin;  // margin between sections
    int headerHeight;    // channel and ports
    int tracksHeight;
    int buttonsHeight;
    if (false)
    {
        // total height is approx ?
        internalMargin = 25;
        headerHeight = 50;
        tracksHeight = 600;
        buttonsHeight = 30;
    } else {
        // total height is approx ?
        internalMargin = 10;
        headerHeight = 38;
        tracksHeight = 580;
        buttonsHeight = 30;
    }

    auto area = getLocalBounds();
    int totalW = area.getWidth();
    auto headerArea = area.removeFromTop(headerHeight);
    channelSelector.setBounds(headerArea.removeFromLeft(totalW * 1 / 10));
    midiInputSelector.setBounds(headerArea.removeFromLeft(totalW * 4 / 10));
    midiOutputSelector.setBounds(headerArea.removeFromLeft(totalW * 4 / 10));
    refreshButton.setBounds(headerArea.removeFromLeft(totalW * 1 / 10));
    // some space
    area.removeFromTop(internalMargin);
    // Tracks section
    int trackSpace = 20;
    int trackWidth = (totalW - 5 * trackSpace) / 6;
    int sliderHeight = tracksHeight - buttonsHeight;
    int potHeight = sliderHeight / 8;
    // Protect this section from a premature execution
    if (slidersArray.size() < slidersCount) return;
    if (mutesArray.size() < 6) return;
    auto tracksArea = area.removeFromTop(tracksHeight);
    auto slidersArea = tracksArea.removeFromTop(sliderHeight);
    auto mutesArea = tracksArea.removeFromTop(buttonsHeight);
    for (int i = 0; i < 6; i++)
    {
        auto trackArea = slidersArea.removeFromLeft(trackWidth);
        slidersArea.removeFromLeft(trackSpace);
        auto potsArea = trackArea.removeFromLeft(trackWidth/2);
        for (int j = 0; j < 8; j++)
        {
            slidersArray[9*i + j]->setBounds(potsArea.removeFromTop(potHeight));
        }
        slidersArray[9*(i+1)-1]->setBounds(trackArea);
        auto muteArea = mutesArea.removeFromLeft(trackWidth);
        mutesArea.removeFromLeft(trackSpace);
        mutesArray[i]->setBounds(muteArea);
    }
    // some space
    area.removeFromTop(internalMargin);
    // Global section
    int buttonsWidth = totalW / 11;
    // Protect this section from a premature execution
    if (scenesArray.size() < 3) return;
    if (fxArray.size() < 5) return;
    // scenes
    for (int i = 0; i < 3; i++)
    {
        scenesArray[i]->setBounds(area.removeFromLeft(buttonsWidth));
    }
    area.removeFromLeft(buttonsWidth);
    // fx
    for (int i = 0; i < 5; i++)
    {
        fxArray[i]->setBounds(area.removeFromLeft(buttonsWidth));
    }
    area.removeFromLeft(buttonsWidth);
    // compression
    compButton.setBounds(area);
}

void MainComponent::refreshMidiPorts()
{
    // MIDI IN
    midiInputSelector.clear();
    availableMidiInputs = juce::MidiInput::getAvailableDevices();

    for (int i = 0; i < availableMidiInputs.size(); ++i)
    {
        midiInputSelector.addItem("IN: " + availableMidiInputs[i].name, i + 1);
    }

    if (availableMidiInputs.isEmpty())
    {
        midiInputSelector.addItem("IN: No MIDI in found!", 1);
        midiInputSelector.setEnabled(false);
    }
    else if (availableMidiInputs.size() > 1)
    {
        // Selects the second available
        midiInputSelector.setSelectedId(2);
        comboBoxChanged(&midiInputSelector);
    }
    else
    {
        // Selects the first available
        midiInputSelector.setSelectedId(1);
        comboBoxChanged(&midiInputSelector);
    }
    // MIDI OUT
    midiOutputSelector.clear();
    availableMidiOutputs = juce::MidiOutput::getAvailableDevices();

    for (int i = 0; i < availableMidiOutputs.size(); ++i)
    {
        midiOutputSelector.addItem("OUT: " + availableMidiOutputs[i].name, i + 1);
    }

    if (availableMidiOutputs.isEmpty())
    {
        midiOutputSelector.addItem("OUT: No MIDI out found!", 1);
        midiOutputSelector.setEnabled(false);
    }
    else if (availableMidiOutputs.size() > 1)
    {
        // Selects the second available
        midiOutputSelector.setSelectedId(2);
        comboBoxChanged(&midiOutputSelector);
    }
    else
    {
        // Selects the first available
        midiOutputSelector.setSelectedId(1);
        comboBoxChanged(&midiOutputSelector);
    }
}

void MainComponent::comboBoxChanged(juce::ComboBox* combo)
{
    if (combo == &channelSelector)
    {
        channel = channelSelector.getSelectedId();
        DBG("CHAN is now: " + std::to_string(channel));
    }
    else if (combo == &midiInputSelector)
    {
        int index = midiInputSelector.getSelectedId() - 1;
        if (index >= 0 && index < availableMidiInputs.size())
        {
            auto deviceInfo = availableMidiInputs[index];
            // Arrêter l'entrée MIDI précédente si elle existe
            if (midiIn)
                midiIn->stop();
            midiIn = juce::MidiInput::openDevice(deviceInfo.identifier, this);
            if (midiIn)
            {
                midiIn->start();
                DBG("MIDI in  is now: " + deviceInfo.name);
            }
            else
            {
                DBG("Could not open MIDI in: " + deviceInfo.name);
            }
        }
    }
    else if (combo == &midiOutputSelector)
    {
        int index = midiOutputSelector.getSelectedId() - 1;
        if (index >= 0 && index < availableMidiOutputs.size())
        {
            midiOut.reset(); // Closes the current one
            auto deviceInfo = availableMidiOutputs[index];
            midiOut = juce::MidiOutput::openDevice(deviceInfo.identifier);
            if (midiOut != nullptr)
                DBG("MIDI out is now: " + deviceInfo.name);
            else
                DBG("Could not open MIDI out: " + deviceInfo.name);
        }
    }
}

void MainComponent::buttonClicked(juce::Button* button)
{
    for (int i = 0; i < 3; i++)
    {
        if (button == scenesArray[i])
        {
        }
    }
    for (int i = 0; i < 5; i++)
    {
        if (button == fxArray[i])
        {
        }
    }
    if (button == &compButton)
    {
        int value = button->getToggleState() ? 1 : 0;
    }
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    for (int i = 0; i < slidersCount; i++)
    {
        if (slider == slidersArray[i])
        {
            int value = (*slider).getValue();
            if ( i == 14 )  // EQ
            {
            }
            else if ( i == 34 )  // Cutoff
            {
            }
            else if ( i == 36 )  // Delay time
            {
            }
            else
            {
            }
            // color code tuning and mix sliders
            if ( i < 12 && ((i-2) % 4 == 0 || (i-3) % 4 == 0 ) || i == 16 )
            {
                juce::Colour colour = juce::Colours::darkgrey;
                if (value < 64)
                {
                    colour = juce::Colours::darkred;
                }
                else if (value > 64)
                {
                    colour = juce::Colours::darkgreen;
                }
                slider->setColour(juce::Slider::trackColourId, colour);
            }
            // color code eq slider
            if ( i == 14 )
            {
                juce::Colour colour = juce::Colours::darkgrey;
                if (value < 128)
                {
                    colour = juce::Colours::darkred;
                }
                else if (value > 128)
                {
                    colour = juce::Colours::darkgreen;
                }
                slider->setColour(juce::Slider::trackColourId, colour);
            }
        }
    }
}

void MainComponent::sendCC(int chan, int cc, int val)
{
    if (midiOut != nullptr)
    {
        auto msg = juce::MidiMessage::controllerEvent(chan, cc, val);
        midiOut->sendMessageNow(msg);
    }
    else
    {
        DBG("No active MIDI out!");
    }
}

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source,
                                              const juce::MidiMessage& message)
{
    //DBG("Received MIDI message: " + message.getDescription());
    if (message.isSysEx()) return;

}
