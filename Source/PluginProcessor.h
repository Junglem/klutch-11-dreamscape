
#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

class KLUTCH11DREAMSCAPEAudioProcessor : public juce::AudioProcessor
{
public:
    KLUTCH11DREAMSCAPEAudioProcessor();
    ~KLUTCH11DREAMSCAPEAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::dsp::Reverb reverb;
    juce::dsp::Chorus<float> chorus;

    juce::dsp::DelayLine<float,
        juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 192000 };

    juce::dsp::IIR::Filter<float> toneFilters[2];

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        KLUTCH11DREAMSCAPEAudioProcessor)
};


