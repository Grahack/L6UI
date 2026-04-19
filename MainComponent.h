#pragma once

#include <map>
#include <unordered_map>
#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

using namespace std;

class MainComponent  : public juce::Component,
                       public juce::MidiInputCallback,
                       public juce::ComboBox::Listener,
                       public juce::Button::Listener,
                       public juce::Slider::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void sendCC(int chan, int cc, int val);
    void handleIncomingMidiMessage(juce::MidiInput* source,
                                   const juce::MidiMessage& message) override;

private:
    CustomLookAndFeel customLookAndFeel;
    // MIDI channel
    int channel;
    juce::ComboBox channelSelector;
    // MIDI in/out
    juce::ComboBox midiInputSelector;
    juce::Array<juce::MidiDeviceInfo> availableMidiInputs;
    std::unique_ptr<juce::MidiInput> midiIn;
    juce::ComboBox midiOutputSelector;
    juce::Array<juce::MidiDeviceInfo> availableMidiOutputs;
    std::unique_ptr<juce::MidiOutput> midiOut;
    juce::TextButton refreshButton;
    // tracks section
    // rotary sliders
    static const int slidersCount = 6*9;
    struct nameCC {
        char name[3];
        int  CC;
    };
    nameCC tracksNameCCs[slidersCount] = {
        // track 1
        {"H", 1}, {"M", 21}, {"F", 11}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"FX", 63}, {"P", 73}, {"1", 83},
        // track 2
        {"H", 1}, {"M", 21}, {"F", 11}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"FX", 63}, {"P", 73}, {"1", 83},
        // track 3
        {"H", 1}, {"M", 21}, {"F", 11}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"FX", 63}, {"P", 73}, {"1", 83},
        // track 4
        {"H", 1}, {"M", 21}, {"F", 11}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"FX", 63}, {"P", 73}, {"1", 83},
        // track 5
        {"H", 1}, {"M", 21}, {"F", 11}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"FX", 63}, {"P", 73}, {"1", 83},
        // track 6
        {"H", 1}, {"M", 21}, {"F", 11}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"FX", 63}, {"P", 73}, {"1", 83},
    };
    OwnedArray<Slider> slidersArray;
    OwnedArray<Label> labelsArray;
    // mute buttons
    int mutesCCs[6] = {93, 93, 93, 93, 93, 93};
    OwnedArray<juce::TextButton> mutesArray;
    // global toggle buttons
    // scenes buttons
    OwnedArray<juce::TextButton> scenesArray;
    // FX buttons
    int fxCC =  117;
    OwnedArray<juce::TextButton> fxArray;
    // comp button
    int compCC = 119;
    juce::TextButton compButton;

    void refreshMidiPorts();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
