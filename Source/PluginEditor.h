
#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class KLUTCH11DREAMSCAPEAudioProcessorEditor
    : public juce::AudioProcessorEditor
{
public:
    explicit KLUTCH11DREAMSCAPEAudioProcessorEditor(
        KLUTCH11DREAMSCAPEAudioProcessor&);
    ~KLUTCH11DREAMSCAPEAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    KLUTCH11DREAMSCAPEAudioProcessor& audioProcessor;

    juce::ComboBox modeSelector;
    juce::Slider dreamSlider, spaceSlider, shimmerSlider;
    juce::Slider widthSlider, chorusSlider, decaySlider, mixSlider;

    juce::Label titleLabel, subtitleLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> dreamAttachment, spaceAttachment;
    std::unique_ptr<Attachment> shimmerAttachment, widthAttachment;
    std::unique_ptr<Attachment> chorusAttachment, decayAttachment;
    std::unique_ptr<Attachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        KLUTCH11DREAMSCAPEAudioProcessorEditor)
};
