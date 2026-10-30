#pragma once

#include <JuceHeader.h>
#include "CallFxEngine.h"
#include <atomic>

class GumzyyCallFXAudioProcessor final : public juce::AudioProcessor
{
public:
    GumzyyCallFXAudioProcessor();
    ~GumzyyCallFXAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    float inputPeak() const noexcept { return inputPeak_.load(std::memory_order_relaxed); }
    float outputPeak() const noexcept { return outputPeak_.load(std::memory_order_relaxed); }

private:
    gumzyy::CallFxEngine engine_;
    std::atomic<float> inputPeak_ { 0.0f }, outputPeak_ { 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GumzyyCallFXAudioProcessor)
};
