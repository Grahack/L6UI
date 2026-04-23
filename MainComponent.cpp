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
        int sliderType = i % 9;
        if (sliderType == 8)
        {   // Level
            slidersArray[i]->setSliderStyle(juce::Slider::LinearVertical);
            slidersArray[i]->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        } else {
            // Rotaries
            slidersArray[i]->setSliderStyle(juce::Slider::Rotary);
            slidersArray[i]->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

            slidersArray[i]->setLookAndFeel(&customLookAndFeel);
            if (sliderType == 0 || sliderType == 1 || sliderType == 3)
            { // EQ
                slidersArray[i]->textFromValueFunction = [](double value)
                {
                    int v = static_cast<int>(value * 100 / 63.5);
                    return juce::String(v - 100);
                };
            }
            else if (sliderType == 2)
            { // Mids freq from 100 to 8000Hz
                slidersArray[i]->textFromValueFunction = [](double value)
                {
                    int m = 100;
                    int M = 8000;
                    int freq = m + value * (M-m)/127;
                    int v = static_cast<int>(freq);
                    return juce::String(v);
                };
            }
            // no else here
            if (sliderType == 4 || sliderType == 5 || sliderType == 7)
            { // Aux sends and Pan
                slidersArray[i]->setColour(Slider::backgroundColourId,
                        juce::Colours::darkgrey);
            }
            // no else here
            if (sliderType == 4 || sliderType == 5 || sliderType == 6)
            { // Aux sends and FX
                slidersArray[i]->setValue(0, juce::dontSendNotification);
                slidersArray[i]->textFromValueFunction = [](double value)
                {
                    int v = static_cast<int>(value * 100 /127);
                    return juce::String(v);
                };
            } else {
                slidersArray[i]->setValue(64, juce::dontSendNotification);
                // those already have a textFromValueFunction
            }
            // no else here
            if (sliderType == 7)
            { // Pan
                slidersArray[i]->textFromValueFunction = [](double value)
                {
                    int v = static_cast<int>(value);
                    if (v == 64) return juce::String("C");
                    if (v < 64)  return "L" + juce::String((64 - v) * 100 / 64);
                    return "R" + juce::String((v - 64) * 100 / 63);
                };
            }
        }
    }
    const char* labels[] = {"H", "M", "F", "L", "A1", "A2", "FX", "P"};
    for (int i = 0; i < labelsCount; i++)
    {
        labelsArray.add(new juce::Label());
        labelsArray[i]->setText (labels[i % 8], juce::dontSendNotification);
        labelsArray[i]->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*labelsArray[i]);
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
    enum RadioButtonIds {
        ScenesButtons = 1001,
        FxButtons = 1002
    };
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
        scenesArray[i]->setClickingTogglesState(true);
        scenesArray[i]->setRadioGroupId (ScenesButtons);
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
        fxArray[i]->setClickingTogglesState(true);
        fxArray[i]->setRadioGroupId (FxButtons);
        addAndMakeVisible(*fxArray[i]);
    }
    fxArray[0]->setToggleState(true, juce::dontSendNotification);
    // compression
    compButton.setButtonText("COMP");
    compButton.setClickingTogglesState(true);
    compButton.setToggleState(false, juce::dontSendNotification);
    compButton.setColour(juce::TextButton::buttonColourId,
                         juce::Colours::darkred);
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
    if (false)
    {
        // total height is approx ?
        internalMargin = 25;
        headerHeight = 50;
    } else {
        // total height is approx ?
        internalMargin = 10;
        headerHeight = 38;
    }

    auto area = getLocalBounds();
    int totalW = area.getWidth();
    int totalH = area.getHeight();
    auto headerArea = area.removeFromTop(headerHeight);
    channelSelector.setBounds(headerArea.removeFromLeft(totalW * 1 / 10));
    midiInputSelector.setBounds(headerArea.removeFromLeft(totalW * 4 / 10));
    midiOutputSelector.setBounds(headerArea.removeFromLeft(totalW * 4 / 10));
    refreshButton.setBounds(headerArea.removeFromLeft(totalW * 1 / 10));
    // some space
    area.removeFromTop(internalMargin);
    // Tracks section
    // We use a 7th (half) strip for the global section, so we have 6 spaces
    int trackSpace = totalW / 30;  // space between tracks
    int trackWidth = (totalW - 6 * trackSpace) / 6.5;
    int potsHeight = totalH / 9;  // 8 pots + 1 mute
    // Protect this section from a premature execution
    if (slidersArray.size() < slidersCount) return;
    if (labelsArray.size() < labelsCount) return;
    if (mutesArray.size() < 6) return;
    for (int i = 0; i < 6; i++)
    {
        auto trackArea = area.removeFromLeft(trackWidth);
        auto potsSliderArea = trackArea.removeFromTop(8*potsHeight);
        auto potsArea = potsSliderArea.removeFromLeft(trackWidth * 0.7);
        for (int j = 0; j < 8; j++)
        {
            slidersArray[9*i + j]->setBounds(potsArea.removeFromTop(potsHeight));
        }
        slidersArray[9*(i+1)-1]->setBounds(potsSliderArea);
        mutesArray[i]->setBounds(trackArea);
        auto labelsArea = area.removeFromLeft(trackSpace);
        if (i < 5)
        {
            for (int j = 0; j < 8; j++)
            {
                labelsArray[8*i + j]->setBounds(labelsArea.removeFromTop(potsHeight));
            }
        }
    }
    // Global section
    int buttonsHeight = area.getHeight() / 11;
    // Protect this section from a premature execution
    if (scenesArray.size() < 3) return;
    if (fxArray.size() < 5) return;
    // scenes
    for (int i = 0; i < 3; i++)
    {
        scenesArray[i]->setBounds(area.removeFromTop(buttonsHeight));
    }
    area.removeFromTop(buttonsHeight);
    // fx
    for (int i = 0; i < 5; i++)
    {
        fxArray[i]->setBounds(area.removeFromTop(buttonsHeight));
    }
    area.removeFromTop(buttonsHeight);
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
    auto state = button->getToggleState();
    for (int i = 0; i < 6; i++)
    {
        if (button == mutesArray[i])
        {
            juce::String stateString = state ? "unmute" : "mute";
            DBG(stateString + " track " + std::to_string(i+1));
            sendCC(channel, mutesCCs[i], state ? 1 : 0);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if (state && button == scenesArray[i])
        {
            DBG("switch to scene " + std::to_string(i));
            sendPC(channel, i);
        }
    }
    for (int i = 0; i < 5; i++)
    {
        if (state && button == fxArray[i])
        {
            DBG("FX " + std::to_string(i));
            sendCC(channel, fxCC, i);
        }
    }
    if (button == &compButton)
    {
        int value = button->getToggleState() ? 1 : 0;
        juce::String stateString = state ? "on" : "off";
        DBG("comp " + stateString);
        int ccValue = state ? 1 : 0;
        sendCC(channel, compCC, ccValue);
    }
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    for (int i = 0; i < slidersCount; i++)
    {
        if (slider == slidersArray[i])
        {
            int trackNum = (int)(i/9 + 1);
            int value = (*slider).getValue();
            DBG("Track " + std::to_string(trackNum));
            DBG((tracksNameCCs[i]).name);
            DBG(std::to_string((tracksNameCCs[i]).CC));
            DBG(std::to_string(value));
            DBG("");
            sendCC(channel, (tracksNameCCs[i]).CC, value);
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

void MainComponent::sendPC(int chan, int pc)
{
    if (midiOut != nullptr)
    {
        auto msg = juce::MidiMessage::programChange(chan, pc);
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
    DBG("Received MIDI message: " + message.getDescription());
    if (message.isSysEx()) return;

}
