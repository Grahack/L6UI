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
    void updateRadios(juce::Button* button, juce::String name);
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void sendCC(int chan, int cc, int val);
    void sendPC(int chan, int pc);
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
    static const int labelsCount = 5*8;  // 8 labels between the tracks
    struct nameCC {
        char name[4];
        int  CC;
    };
    nameCC tracksNameCCs[slidersCount] = {
        // Names are just here for convenience, they are used in the real code
        // They are used in DBG though
        // track 1
        {"H", 1}, {"F", 11}, {"M", 21}, {"L", 33},
        {"A1", 43}, {"A2", 53}, {"MX", 63}, {"P", 73}, {"LVL", 83},
        // track 2
        {"H", 2}, {"F", 12}, {"M", 22}, {"L", 34},
        {"A1", 44}, {"A2", 54}, {"MX", 64}, {"P", 74}, {"LVL", 84},
        // track 3
        {"H", 3}, {"F", 13}, {"M", 23}, {"L", 35},
        {"A1", 45}, {"A2", 55}, {"MX", 65}, {"P", 75}, {"LVL", 85},
        // track 4
        {"H", 4}, {"F", 14}, {"M", 24}, {"L", 36},
        {"A1", 46}, {"A2", 56}, {"MX", 66}, {"P", 76}, {"LVL", 86},
        // track 5
        {"H", 5}, {"F", 15}, {"M", 25}, {"L", 37},
        {"A1", 47}, {"A2", 57}, {"MX", 67}, {"P", 77}, {"LVL", 87},
        // track 6
        {"H", 6}, {"F", 16}, {"M", 26}, {"L", 38},
        {"A1", 48}, {"A2", 58}, {"FX", 68}, {"P", 78}, {"LVL", 88},
    };
    OwnedArray<Slider> slidersArray;
    OwnedArray<Label> labelsArray;
    // mute buttons
    int mutesCCs[6] = {93, 94, 95, 102, 103, 104};
    OwnedArray<juce::TextButton> mutesArray;
    // global toggle buttons
    // scenes buttons
    OwnedArray<juce::TextButton> scenesArray;
    // FX buttons
    int fxCC =  117;
    int fxValues[6] = {0, 26, 51, 77, 102};
    OwnedArray<juce::TextButton> fxArray;
    // comp button
    int compCC = 119;
    juce::TextButton compButton;

    void refreshMidiPorts();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
