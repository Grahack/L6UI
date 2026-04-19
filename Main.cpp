#include <JuceHeader.h>
#include "MainComponent.h"

class MyApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override     { return "L6UI"; }
    const juce::String getApplicationVersion() override  { return "0.1.0"; }

    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow("L2UI", new MainComponent(), *this));
    }

    void shutdown() override {
        mainWindow = nullptr;
    }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(juce::String name, juce::Component* c, JUCEApplication& app)
          : DocumentWindow(name,
                           juce::Desktop::getInstance().getDefaultLookAndFeel()
                              .findColour(ResizableWindow::backgroundColourId),
                           DocumentWindow::allButtons),
            ownerApp(app)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override {
            ownerApp.systemRequestedQuit();
        }

    private:
        JUCEApplication& ownerApp;
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(MyApp)

