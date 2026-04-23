class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:

    const juce::Colour l6 = juce::Colour (0xff00aaff);
    const juce::Colour l6_rotary_outline = juce::Colours::white;
    const juce::Colour l6_rotary_pointer = juce::Colours::black;

    CustomLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId, l6);
        setColour (juce::ComboBox::arrowColourId, juce::Colours::black);
        setColour (juce::ComboBox::textColourId, juce::Colours::black);
        setColour (juce::ComboBox::outlineColourId, juce::Colours::black);
    }

    Font getComboBoxFont (ComboBox& box) override
    {
        return Font(FontOptions(22.f, juce::Font::bold));
    }

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, Slider& slider) override
    {
        // https://forum.juce.com/t/tip-how-to-include-value-and-label-on-rotaryknob-without-declaring-a-label/35926
        auto radius = jmin (width / 2.25, height / 2.25) - 4.5;
        radius = jmax (radius, 1.0d);
        auto centreX = x + width / 2;
        auto centreY = y + height / 2 + height / 30;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto isMouseOver = slider.isMouseOverOrDragging () && slider.isEnabled ();

        // Box and outline of knob area
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.fillRect (x, y, width, height);

        g.setColour (Colour::fromRGB(16, 16, 16));
        g.drawRect (x, y, width, height);

        // Knob fill
        g.setColour (juce::Colours::black. withAlpha (isMouseOver ? 1.0f : 0.7f));
        g.fillEllipse (rx, ry, rw, rw);

        // Knob outline
        g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId).
        withAlpha (isMouseOver ? 1.0f : 0.7f));
        g.drawEllipse (rx, ry, rw, rw, 1.0f);

        // Knob pointer
        Path p;
        auto pointerLength = radius * 0.5f;
        auto pointerThickness = jmax (1.0f, width * 0.05f);
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));

        g.setColour (slider.findColour (Slider::thumbColourId));
        g.fillPath (p);

        // Value
        g.setColour (Colours::white);
        g.setFont (radius * 0.7f);
        juce::String text;
        if (slider.textFromValueFunction != nullptr)
            text = slider.textFromValueFunction(slider.getValue());
        else
            text = juce::String(slider.getValue(), 2);
        g.drawSingleLineText (text,
                              centreX, height * 0.6f,
                              juce::Justification::horizontallyCentred);
    }

private:
    bool isLabelInComboBox(juce::Label& label)
    {
        auto* parent = label.getParentComponent();
        while (parent != nullptr)
        {
            if (dynamic_cast<juce::ComboBox*>(parent) != nullptr)
                return true;
            parent = parent->getParentComponent();
        }
        return false;
    }
};
