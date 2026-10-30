#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout GumzyyCallFXAudioProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    using Choice = juce::AudioParameterChoice;
    using Bool = juce::AudioParameterBool;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<Choice>(juce::ParameterID { "style", 1 }, "Call Style",
        juce::StringArray { "ANGEL", "PHONE", "RADIO", "WALKIE", "MEGAPHONE", "BROKEN" }, 0));
    params.push_back(std::make_unique<Bool>(juce::ParameterID { "bypass", 1 }, "Bypass", false));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "hp", 1 }, "High Pass",
        juce::NormalisableRange<float>(20.0f, 1200.0f, 1.0f, 0.38f), 85.0f, " Hz"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "lp", 1 }, "Low Pass",
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.45f), 18000.0f, " Hz"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "air", 1 }, "Air",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.01f), 8.0f, " dB"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "drive", 1 }, "Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 5.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "crush", 1 }, "Crush",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "width", 1 }, "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f), 158.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "reverb", 1 }, "Reverb",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 48.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "delay", 1 }, "Delay",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 22.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "delayTime", 1 }, "Delay Time",
        juce::NormalisableRange<float>(60.0f, 800.0f, 1.0f, 0.5f), 430.0f, " ms"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "feedback", 1 }, "Feedback",
        juce::NormalisableRange<float>(0.0f, 75.0f, 0.1f), 26.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "mix", 1 }, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f, "%"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "output", 1 }, "Output",
        juce::NormalisableRange<float>(-18.0f, 12.0f, 0.01f), 0.0f, " dB"));
    // Added at the end so existing v0.1.0 automation IDs remain unchanged.
    params.push_back(std::make_unique<Float>(juce::ParameterID { "input", 1 }, "Input Gain",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f, " dB"));
    params.push_back(std::make_unique<Float>(juce::ParameterID { "tone", 1 }, "Tone",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f), 24.0f, "%"));
    // Added as a new stable ID so older projects retain all existing automation.
    params.push_back(std::make_unique<Float>(juce::ParameterID { "shimmer", 1 }, "Shimmer",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 62.0f, "%"));

    return { params.begin(), params.end() };
}

GumzyyCallFXAudioProcessor::GumzyyCallFXAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "GUMZYY_CALL_FX_STATE", createParameterLayout())
{
}

void GumzyyCallFXAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine_.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    // The dry path is never delayed. Delay and reverb live only on the wet path.
    setLatencySamples(0);
}

void GumzyyCallFXAudioProcessor::releaseResources()
{
    engine_.reset();
}

bool GumzyyCallFXAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    return input == output && (input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo());
}

void GumzyyCallFXAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (parameters.getRawParameterValue("bypass")->load(std::memory_order_relaxed) >= 0.5f)
    {
        float peak = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            const auto* data = buffer.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                peak = juce::jmax(peak, std::abs(data[i]));
        }
        inputPeak_.store(peak, std::memory_order_relaxed);
        outputPeak_.store(peak, std::memory_order_relaxed);
        return;
    }

    gumzyy::CallFxParameters p;
    p.style = static_cast<int>(parameters.getRawParameterValue("style")->load(std::memory_order_relaxed));
    p.highPassHz = parameters.getRawParameterValue("hp")->load(std::memory_order_relaxed);
    p.lowPassHz = parameters.getRawParameterValue("lp")->load(std::memory_order_relaxed);
    p.inputDb = parameters.getRawParameterValue("input")->load(std::memory_order_relaxed);
    p.tone = parameters.getRawParameterValue("tone")->load(std::memory_order_relaxed);
    p.airDb = parameters.getRawParameterValue("air")->load(std::memory_order_relaxed);
    p.shimmer = parameters.getRawParameterValue("shimmer")->load(std::memory_order_relaxed);
    p.drive = parameters.getRawParameterValue("drive")->load(std::memory_order_relaxed);
    p.crush = parameters.getRawParameterValue("crush")->load(std::memory_order_relaxed);
    p.width = parameters.getRawParameterValue("width")->load(std::memory_order_relaxed);
    p.reverb = parameters.getRawParameterValue("reverb")->load(std::memory_order_relaxed);
    p.delay = parameters.getRawParameterValue("delay")->load(std::memory_order_relaxed);
    p.delayTimeMs = parameters.getRawParameterValue("delayTime")->load(std::memory_order_relaxed);
    p.feedback = parameters.getRawParameterValue("feedback")->load(std::memory_order_relaxed);
    p.mix = parameters.getRawParameterValue("mix")->load(std::memory_order_relaxed);
    p.outputDb = parameters.getRawParameterValue("output")->load(std::memory_order_relaxed);

    float* channels[2] = { nullptr, nullptr };
    const int count = juce::jmin(2, buffer.getNumChannels());
    for (int ch = 0; ch < count; ++ch)
        channels[ch] = buffer.getWritePointer(ch);

    engine_.process(channels, count, buffer.getNumSamples(), p);
    inputPeak_.store(engine_.inputPeak(), std::memory_order_relaxed);
    outputPeak_.store(engine_.outputPeak(), std::memory_order_relaxed);
}

juce::AudioProcessorEditor* GumzyyCallFXAudioProcessor::createEditor()
{
    return new GumzyyCallFXAudioProcessorEditor(*this);
}

void GumzyyCallFXAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    const auto state = parameters.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, destination);
}

void GumzyyCallFXAudioProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GumzyyCallFXAudioProcessor();
}
