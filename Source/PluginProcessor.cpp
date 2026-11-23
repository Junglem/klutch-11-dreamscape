
#include "PluginProcessor.h"
#include "PluginEditor.h"

KLUTCH11DREAMSCAPEAudioProcessor::KLUTCH11DREAMSCAPEAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
KLUTCH11DREAMSCAPEAudioProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterChoice>(
        "MODE", "MODE",
        StringArray { "HEAVEN", "AFTERLIFE", "CLOUD 11" }, 0));

    layout.add(std::make_unique<AudioParameterFloat>(
        "DREAM", "DREAM", NormalisableRange<float>(0.0f, 1.0f), 0.65f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "SPACE", "SPACE", NormalisableRange<float>(0.0f, 1.0f), 0.65f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "SHIMMER", "SHIMMER", NormalisableRange<float>(0.0f, 1.0f), 0.45f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "WIDTH", "WIDTH", NormalisableRange<float>(0.0f, 1.0f), 0.75f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "CHORUS", "CHORUS", NormalisableRange<float>(0.0f, 1.0f), 0.35f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "DECAY", "DECAY", NormalisableRange<float>(0.0f, 1.0f), 0.70f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "MIX", "MIX", NormalisableRange<float>(0.0f, 1.0f), 0.45f));

    return layout;
}

void KLUTCH11DREAMSCAPEAudioProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    reverb.reset();
    chorus.reset();
    reverb.prepare(spec);
    chorus.prepare(spec);
}

void KLUTCH11DREAMSCAPEAudioProcessor::releaseResources()
{
}

void KLUTCH11DREAMSCAPEAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalInputChannels = getTotalNumInputChannels();
    const int totalOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalInputChannels; channel < totalOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    auto getValue = [this](const char* id)
    {
        return parameters.getRawParameterValue(id)->load();
    };

    const int mode = juce::roundToInt(getValue("MODE") * 2.0f);
    const float dream = getValue("DREAM");
    const float space = getValue("SPACE");
    const float shimmer = getValue("SHIMMER");
    const float width = getValue("WIDTH");
    const float chorusAmount = getValue("CHORUS");
    const float decay = getValue("DECAY");
    const float mix = getValue("MIX");

    juce::dsp::Reverb::Parameters reverbParams;

    float roomSize = 0.65f + space * 0.30f;
    float damping = 0.35f;
    float wetLevel = 0.35f + dream * 0.35f;
    float dryLevel = 0.0f;

    if (mode == 0) // HEAVEN
    {
        roomSize = 0.75f + space * 0.23f;
        damping = 0.25f;
        wetLevel *= 1.0f;
    }
    else if (mode == 1) // AFTERLIFE
    {
        roomSize = 0.65f + space * 0.25f;
        damping = 0.75f;
        wetLevel *= 0.85f;
    }
    else // CLOUD 11
    {
        roomSize = 0.85f + space * 0.14f;
        damping = 0.40f;
        wetLevel *= 1.15f;
    }

    reverbParams.roomSize = juce::jlimit(0.0f, 1.0f, roomSize);
    reverbParams.damping = juce::jlimit(0.0f, 1.0f, damping + (1.0f - decay) * 0.2f);
    reverbParams.wetLevel = juce::jlimit(0.0f, 1.0f, wetLevel);
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = width;
    reverbParams.freezeMode = 0.0f;

    reverb.setParameters(reverbParams);

    chorus.setRate(mode == 1 ? 0.18f : 0.30f + shimmer * 0.8f);
    chorus.setDepth(0.15f + chorusAmount * 0.65f);
    chorus.setCentreDelay(7.0f + shimmer * 12.0f);
    chorus.setFeedback(mode == 1 ? 0.20f : 0.08f);
    chorus.setMix(0.25f + chorusAmount * 0.25f);

    juce::AudioBuffer<float> dryBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    dryBuffer.makeCopyOf(buffer);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    chorus.process(context);
    reverb.process(context);

    const float effectLevel = juce::jlimit(0.0f, 1.0f, mix);
    const float dryLevelMix = 1.0f - effectLevel;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* wet = buffer.getWritePointer(channel);
        const auto* dry = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            wet[sample] = dry[sample] * dryLevelMix
                        + wet[sample] * effectLevel;
        }
    }
}

bool KLUTCH11DREAMSCAPEAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor*
KLUTCH11DREAMSCAPEAudioProcessor::createEditor()
{
    return new KLUTCH11DREAMSCAPEAudioProcessorEditor(*this);
}

const juce::String KLUTCH11DREAMSCAPEAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KLUTCH11DREAMSCAPEAudioProcessor::acceptsMidi() const { return false; }
bool KLUTCH11DREAMSCAPEAudioProcessor::producesMidi() const { return false; }
bool KLUTCH11DREAMSCAPEAudioProcessor::isMidiEffect() const { return false; }
double KLUTCH11DREAMSCAPEAudioProcessor::getTailLengthSeconds() const { return 10.0; }

int KLUTCH11DREAMSCAPEAudioProcessor::getNumPrograms() { return 1; }
int KLUTCH11DREAMSCAPEAudioProcessor::getCurrentProgram() { return 0; }
void KLUTCH11DREAMSCAPEAudioProcessor::setCurrentProgram(int) {}
const juce::String KLUTCH11DREAMSCAPEAudioProcessor::getProgramName(int) { return {}; }
void KLUTCH11DREAMSCAPEAudioProcessor::changeProgramName(int, const juce::String&) {}

void KLUTCH11DREAMSCAPEAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void KLUTCH11DREAMSCAPEAudioProcessor::setStateInformation(
    const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KLUTCH11DREAMSCAPEAudioProcessor();
}
