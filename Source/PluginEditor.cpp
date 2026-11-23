
#include "PluginEditor.h"

using namespace juce;

namespace
{
    const Colour backgroundColour(0xFF08070D);
    const Colour panelColour(0xFF171023);
    const Colour accentColour(0xFF8B45D6);
    const Colour lavenderColour(0xFFC5A3FF);
    const Colour mutedColour(0xFF91869F);
}

KLUTCH11DREAMSCAPEAudioProcessorEditor::
KLUTCH11DREAMSCAPEAudioProcessorEditor(
    KLUTCH11DREAMSCAPEAudioProcessor& processor)
    : AudioProcessorEditor(&processor),
      audioProcessor(processor)
{
    setSize(680, 440);

    titleLabel.setText("DREAMSCAPE", dontSendNotification);
    titleLabel.setFont(Font(30.0f, Font::bold));
    titleLabel.setColour(Label::textColourId, Colours::white);
    titleLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("VOCAL ATMOSPHERE ENGINE  //  KLUTCH 11",
                          dontSendNotification);
    subtitleLabel.setFont(Font(10.0f));
    subtitleLabel.setColour(Label::textColourId, mutedColour);
    subtitleLabel.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(subtitleLabel);

    modeSelector.addItem("HEAVEN", 1);
    modeSelector.addItem("AFTERLIFE", 2);
    modeSelector.addItem("CLOUD 11", 3);

    modeSelector.setSelectedId(
        static_cast<int>(audioProcessor.parameters
            .getRawParameterValue("MODE")->load()) + 1,
        dontSendNotification);

    modeSelector.setColour(ComboBox::backgroundColourId, panelColour);
    modeSelector.setColour(ComboBox::textColourId, lavenderColour);
    modeSelector.setColour(ComboBox::outlineColourId, accentColour);
    modeSelector.setColour(ComboBox::arrowColourId, lavenderColour);

    modeSelector.onChange = [this]
    {
        audioProcessor.parameters.getParameter("MODE")
            ->setValueNotifyingHost(
                modeSelector.getSelectedId() - 1.0f);
    };

    addAndMakeVisible(modeSelector);

    auto setupSlider = [this](Slider& slider)
    {
        slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(Slider::TextBoxBelow, false, 65, 20);

        slider.setColour(Slider::rotarySliderFillColourId, accentColour);
        slider.setColour(Slider::rotarySliderOutlineColourId, panelColour);
        slider.setColour(Slider::thumbColourId, lavenderColour);
        slider.setColour(Slider::textBoxTextColourId, Colours::white);
        slider.setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
        slider.setColour(Slider::textBoxBackgroundColourId, panelColour);

        addAndMakeVisible(slider);
    };

    setupSlider(dreamSlider);
    setupSlider(spaceSlider);
    setupSlider(shimmerSlider);
    setupSlider(widthSlider);
    setupSlider(chorusSlider);
    setupSlider(decaySlider);
    setupSlider(mixSlider);

    auto& state = audioProcessor.parameters;

    dreamAttachment = std::make_unique<Attachment>(state, "DREAM", dreamSlider);
    spaceAttachment = std::make_unique<Attachment>(state, "SPACE", spaceSlider);
    shimmerAttachment = std::make_unique<Attachment>(state, "SHIMMER", shimmerSlider);
    widthAttachment = std::make_unique<Attachment>(state, "WIDTH", widthSlider);
    chorusAttachment = std::make_unique<Attachment>(state, "CHORUS", chorusSlider);
    decayAttachment = std::make_unique<Attachment>(state, "DECAY", decaySlider);
    mixAttachment = std::make_unique<Attachment>(state, "MIX", mixSlider);
}

void KLUTCH11DREAMSCAPEAudioProcessorEditor::paint(Graphics& g)
{
    g.fillAll(backgroundColour);

    g.setColour(panelColour);
    g.fillRoundedRectangle(18.0f, 18.0f, 644.0f, 95.0f, 14.0f);
    g.fillRoundedRectangle(18.0f, 130.0f, 644.0f, 290.0f, 14.0f);

    g.setColour(accentColour);
    g.fillRoundedRectangle(18.0f, 18.0f, 5.0f, 95.0f, 2.0f);

    g.setColour(lavenderColour);
    g.setFont(Font(11.0f, Font::bold));

    g.drawText("SELECT YOUR UNIVERSE",
               38, 119, 250, 22, Justification::centredLeft);

    g.setColour(mutedColour);
    g.setFont(Font(10.0f));

    const StringArray names {
        "DREAM", "SPACE", "SHIMMER",
        "WIDTH", "CHORUS", "DECAY", "MIX"
    };

    const Slider* sliders[] = {
        &dreamSlider, &spaceSlider, &shimmerSlider,
        &widthSlider, &chorusSlider, &decaySlider, &mixSlider
    };

    for (int i = 0; i < 7; ++i)
    {
        auto bounds = sliders[i]->getBounds();
        g.drawText(names[i],
                   bounds.getX() - 5,
                   bounds.getY() - 17,
                   bounds.getWidth() + 10,
                   16,
                   Justification::centred);
    }

    g.setColour(accentColour.withAlpha(0.7f));
    g.drawRoundedRectangle(18.0f, 18.0f, 644.0f, 402.0f, 14.0f, 1.0f);

    g.setColour(mutedColour);
    g.setFont(Font(9.0f));
    g.drawText("KLUTCH 11  //  DREAM IN COLOUR",
               0, 420, 680, 15, Justification::centred);
}

void KLUTCH11DREAMSCAPEAudioProcessorEditor::resized()
{
    titleLabel.setBounds(38, 29, 350, 42);
    subtitleLabel.setBounds(40, 72, 350, 20);

    modeSelector.setBounds(390, 48, 235, 35);

    const int sliderWidth = 135;
    const int sliderHeight = 115;
    const int startX = 35;
    const int startY = 170;
    const int gapX = 180;

    Slider* sliders[] = {
        &dreamSlider, &spaceSlider, &shimmerSlider,
        &widthSlider, &chorusSlider, &decaySlider, &mixSlider
    };

    for (int i = 0; i < 7; ++i)
    {
        const int row = i / 4;
        const int column = i % 4;

        sliders[i]->setBounds(
            startX + column * gapX,
            startY + row * 135,
            sliderWidth,
            sliderHeight
        );
    }
}
