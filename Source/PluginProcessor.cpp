
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

    layout.add(std::make_unique<AudioParameterFloat>(
        "PITCH", "PITCH", NormalisableRange<float>(-12.0f, 12.0f), 0.0f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "ECHO", "ECHO", NormalisableRange<float>(0.0f, 1.0f), 0.35f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "FILTER", "FILTER", NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "MOTION", "MOTION", NormalisableRange<float>(0.0f, 1.0f), 0.4f));

    layout.add(std::make_unique<AudioParameterFloat>(
        "GLIDE", "GLIDE", NormalisableRange<float>(0.0f, 1.0f), 0.3f));

    return layout;
}

void KLUTCH11DREAMSCAPEAudioProcessor::prepareToPlay(
    double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    reverb.reset();
    chorus.reset();
    delayLine.reset();

    reverb.prepare(spec);
    chorus.prepare(spec);
    delayLine.prepare(spec);

    delayLine.setDelay(static_cast<float>(sampleRate * 0.35));

    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    for (auto& filter : toneFilters)
    {
        filter.reset();

        filter.coefficients =
            juce::dsp::IIR::Coefficients<float>::makeLowPass(
                sampleRate, 18000.0f);

        filter.prepare(monoSpec);
    }
}

void KLUTCH11DREAMSCAPEAudioProcessor::releaseResources()
{
}

void KLUTCH11DREAMSCAPEAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const int inputChannels = getTotalNumInputChannels();
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int channel = inputChannels; channel < numChannels; ++channel)
        buffer.clear(channel, 0, numSamples);

    if (numChannels == 0 || numChannels > 2)
        return;

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
    const float echo = getValue("ECHO");
    const float filter = getValue("FILTER");
    const float motion = getValue("MOTION");

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    juce::AudioBuffer<float> wetBuffer;
    wetBuffer.makeCopyOf(buffer);

    // REVERB ENGINE

    juce::dsp::Reverb::Parameters reverbParams;

    if (mode == 0) // HEAVEN
    {
        reverbParams.roomSize = 0.95f;
        reverbParams.damping = 0.15f;
        reverbParams.wetLevel = 0.70f;
    }
    else if (mode == 1) // AFTERLIFE
    {
        reverbParams.roomSize = 0.72f;
        reverbParams.damping = 0.85f;
        reverbParams.wetLevel = 0.60f;
    }
    else // CLOUD 11
    {
        reverbParams.roomSize = 0.90f;
        reverbParams.damping = 0.35f;
        reverbParams.wetLevel = 0.68f;
    }

    reverbParams.roomSize = juce::jlimit(
        0.1f, 0.98f,
        reverbParams.roomSize * (0.7f + space * 0.3f)
        * (0.8f + decay * 0.2f));

    reverbParams.damping = juce::jlimit(
        0.0f, 1.0f, reverbParams.damping + (1.0f - decay) * 0.15f);

    reverbParams.wetLevel *= 0.5f + dream * 0.5f;
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = width;
    reverbParams.freezeMode = 0.0f;

    reverb.setParameters(reverbParams);

    // CHORUS ENGINE

    if (mode == 0) // HEAVEN
    {
        chorus.setRate(0.25f + motion * 0.7f);
        chorus.setDepth(0.25f + chorusAmount * 0.35f);
        chorus.setCentreDelay(12.0f + shimmer * 8.0f);
    }
    else if (mode == 1) // AFTERLIFE
    {
        chorus.setRate(0.15f + motion * 0.35f);
        chorus.setDepth(0.10f + chorusAmount * 0.20f);
        chorus.setCentreDelay(8.0f);
    }
    else // CLOUD 11
    {
        chorus.setRate(0.7f + motion * 1.8f);
        chorus.setDepth(0.45f + chorusAmount * 0.35f);
        chorus.setCentreDelay(18.0f);
    }

    chorus.setFeedback(0.1f);
    chorus.setMix(1.0f);

    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> context(wetBlock);

    chorus.process(context);
    reverb.process(context);

    // ECHO ENGINE

    const float delaySeconds =
        mode == 0 ? 0.42f :
        mode == 1 ? 0.62f : 0.28f;

    const float delaySamples = juce::jlimit(
        1.0f, 190000.0f,
        static_cast<float>(currentSampleRate * delaySeconds));

    delayLine.setDelay(delaySamples);

    const float feedback = echo * (mode == 1 ? 0.55f : 0.40f);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* wet = wetBuffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float delayed = delayLine.popSample(channel);
            const float input = wet[sample];

            delayLine.pushSample(
                channel, input + delayed * feedback);

            wet[sample] = input + delayed * echo;
        }
    }

    // EFFECT FILTER

    const float effectCutoff = juce::jmap(
        filter, 1200.0f, 18000.0f);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        toneFilters[channel].coefficients =
            juce::dsp::IIR::Coefficients<float>::makeLowPass(
                currentSampleRate, effectCutoff);

        juce::dsp::AudioBlock<float> fullWetBlock(wetBuffer);

        auto channelBlock = fullWetBlock.getSingleChannelBlock(
            static_cast<size_t>(channel));

        juce::dsp::ProcessContextReplacing<float> filterContext(
            channelBlock);

        toneFilters[channel].process(filterContext);
    }

    // DRY / WET MIX

    const float wetLevel = juce::jlimit(
        0.0f, 1.0f, mix * (0.35f + dream * 0.65f));

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* output = buffer.getWritePointer(channel);
        const auto* dry = dryBuffer.getReadPointer(channel);
        const auto* wet = wetBuffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            output[sample] =
                dry[sample] * (1.0f - wetLevel)
                + wet[sample] * wetLevel;
        }
    }
}

bool KLUTCH11DREAMSCAPEAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor*
KLUTCH11DREAMSCAPEAudioProcessor::createEditor()
{
    return new KLUTCH11DREAMSCAPEAudioProcessorEditor(*this);
}

const juce::String KLUTCH11DREAMSCAPEAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KLUTCH11DREAMSCAPEAudioProcessor::acceptsMidi() const
{
    return false;
}

bool KLUTCH11DREAMSCAPEAudioProcessor::producesMidi() const
{
    return false;
}

bool KLUTCH11DREAMSCAPEAudioProcessor::isMidiEffect() const
{
    return false;
}

double KLUTCH11DREAMSCAPEAudioProcessor::getTailLengthSeconds() const
{
    return 10.0;
}

int KLUTCH11DREAMSCAPEAudioProcessor::getNumPrograms()
{
    return 1;
}

int KLUTCH11DREAMSCAPEAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KLUTCH11DREAMSCAPEAudioProcessor::setCurrentProgram(int)
{
}

const juce::String KLUTCH11DREAMSCAPEAudioProcessor::getProgramName(int)
{
    return {};
}

void KLUTCH11DREAMSCAPEAudioProcessor::changeProgramName(
    int, const juce::String&)
{
}

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
    std::unique_ptr<juce::XmlElement> xml(
        getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KLUTCH11DREAMSCAPEAudioProcessor();
}
