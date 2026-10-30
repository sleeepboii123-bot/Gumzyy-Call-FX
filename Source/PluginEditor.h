#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class GumzyyLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
};

class GumzyyCallFXAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit GumzyyCallFXAudioProcessorEditor(GumzyyCallFXAudioProcessor&);
    ~GumzyyCallFXAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void addKnob(juce::Slider&, juce::Label&, const juce::String&, const juce::String&);
    void applyPreset(int index);
    void setParameter(const juce::String& id, float value);
    void drawPanel(juce::Graphics&, juce::Rectangle<float>, const juce::String&, const juce::String& accent);
    void clearLookAndFeel();

    GumzyyLookAndFeel lookAndFeel_;
    GumzyyCallFXAudioProcessor& processor_;
    juce::Image mascot_;

    juce::Label brand_, subtitle_, modeLabel_, statusLabel_;
    juce::ComboBox style_;
    juce::TextButton sizeButton_ { "SMALL" }, bypassButton_ { "BYPASS" };
    juce::Slider hp_, lp_, air_, width_, drive_, crush_, reverb_, delay_, delayTime_, feedback_, mix_, output_;
    juce::Label hpLabel_, lpLabel_, airLabel_, widthLabel_, driveLabel_, crushLabel_;
    juce::Label reverbLabel_, delayLabel_, delayTimeLabel_, feedbackLabel_, mixLabel_, outputLabel_;

    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ComboAttachment> styleAttachment_;
    std::unique_ptr<ButtonAttachment> bypassAttachment_;
    std::unique_ptr<SliderAttachment> hpAttachment_, lpAttachment_, airAttachment_, widthAttachment_;
    std::unique_ptr<SliderAttachment> driveAttachment_, crushAttachment_, reverbAttachment_, delayAttachment_;
    std::unique_ptr<SliderAttachment> delayTimeAttachment_, feedbackAttachment_, mixAttachment_, outputAttachment_;

    bool compact_ = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GumzyyCallFXAudioProcessorEditor)
};
