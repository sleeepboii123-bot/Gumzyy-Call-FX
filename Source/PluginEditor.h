#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class GumzyyLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};

class GumzyyCallFXAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    explicit GumzyyCallFXAudioProcessorEditor(GumzyyCallFXAudioProcessor&);
    ~GumzyyCallFXAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    static constexpr std::size_t snapshotParameterCount = 16;
    using Snapshot = std::array<float, snapshotParameterCount>;

    void addKnob(juce::Slider&, juce::Label&, const juce::String&, const juce::String&,
                 const juce::String& tooltip, double defaultValue);
    void applyPreset(int index);
    void setParameter(const juce::String& id, float value);
    void clearLookAndFeel();
    void timerCallback() override;
    void updateStyleButtons();
    void randomizeCurrentStyle();
    void resetCurrentStyle();
    void handleABCompare();
    void captureSnapshot(Snapshot&);
    void restoreSnapshot(const Snapshot&);
    void setStatus(const juce::String&);
    static juce::String peakToDb(float peak);
    void drawModule(juce::Graphics&, juce::Rectangle<float>, const juce::String& number,
                    const juce::String& title, const juce::String& subtitle);

    GumzyyLookAndFeel lookAndFeel_;
    GumzyyCallFXAudioProcessor& processor_;
    juce::Image phoneBanner_;
    juce::Random random_;

    juce::Label brand_, product_, subtitle_, statusLabel_;
    std::array<juce::TextButton, 6> styleButtons_;
    juce::TextButton sizeButton_ { "SMALL" }, bypassButton_ { "BYPASS" };
    juce::TextButton abButton_ { "A/B" }, randomButton_ { "DICE" }, resetButton_ { "RESET" };

    juce::Slider hp_, lp_, input_, tone_, air_, shimmer_, width_, drive_, crush_;
    juce::Slider reverb_, delay_, delayTime_, feedback_, mix_, output_;
    juce::Label hpLabel_, lpLabel_, inputLabel_, toneLabel_, airLabel_, shimmerLabel_, widthLabel_;
    juce::Label driveLabel_, crushLabel_, reverbLabel_, delayLabel_, delayTimeLabel_, feedbackLabel_;
    juce::Label mixLabel_, outputLabel_;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment_;
    std::unique_ptr<SliderAttachment> hpAttachment_, lpAttachment_, inputAttachment_, toneAttachment_;
    std::unique_ptr<SliderAttachment> airAttachment_, shimmerAttachment_, widthAttachment_;
    std::unique_ptr<SliderAttachment> driveAttachment_, crushAttachment_;
    std::unique_ptr<SliderAttachment> reverbAttachment_, delayAttachment_, delayTimeAttachment_, feedbackAttachment_;
    std::unique_ptr<SliderAttachment> mixAttachment_, outputAttachment_;

    Snapshot snapshotA_ {}, snapshotB_ {};
    bool abReady_ = false;
    bool abHasB_ = false;
    bool showingA_ = false;
    bool compact_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GumzyyCallFXAudioProcessorEditor)
};
