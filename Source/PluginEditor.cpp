#include "PluginEditor.h"
#include "BinaryData.h"
#include <algorithm>
#include <cmath>

namespace
{
const juce::Colour bg          { 0xff08070c };
const juce::Colour chassis     { 0xff100d16 };
const juce::Colour panel       { 0xff15111c };
const juce::Colour panelTop    { 0xff21192b };
const juce::Colour raised      { 0xff211a2a };
const juce::Colour edge        { 0xff4a365e };
const juce::Colour violet      { 0xffa65cff };
const juce::Colour bright      { 0xffdfc4ff };
const juce::Colour white       { 0xfff4eff8 };
const juce::Colour muted       { 0xffa99bb8 };
const juce::Colour dim         { 0xff766a84 };
const juce::Colour meterBg     { 0xff08070b };
const juce::Colour danger      { 0xffff6e88 };

constexpr std::array<const char*, 16> parameterIds {
    "style", "hp", "lp", "input", "tone", "air", "shimmer", "width", "drive", "crush",
    "reverb", "delay", "delayTime", "feedback", "mix", "output"
};

float safeMeterLevel(float peak)
{
    if (!std::isfinite(peak) || peak <= 0.000001f)
        return 0.0f;
    const float db = 20.0f * std::log10(peak);
    return juce::jlimit(0.0f, 1.0f, (db + 54.0f) / 54.0f);
}
}

void GumzyyLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    const float side = static_cast<float>(juce::jmin(width, height)) - 30.0f;
    if (side < 18.0f) return;
    const float radius = side * 0.5f;
    const float cx = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float cy = static_cast<float>(y) + static_cast<float>(height) * 0.50f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Outer chassis / soft neon halo.
    g.setColour(violet.withAlpha(0.08f));
    g.fillEllipse(cx - radius - 8.0f, cy - radius - 8.0f, side + 16.0f, side + 16.0f);
    g.setColour(juce::Colour(0xff040308));
    g.fillEllipse(cx - radius - 3.0f, cy - radius - 3.0f, side + 6.0f, side + 6.0f);
    g.setColour(juce::Colour(0xff39313f));
    g.fillEllipse(cx - radius, cy - radius, side, side);
    g.setColour(juce::Colour(0xff111016));
    g.fillEllipse(cx - radius + 6.0f, cy - radius + 6.0f, side - 12.0f, side - 12.0f);
    g.setColour(juce::Colour(0xff524760));
    g.drawEllipse(cx - radius + 2.0f, cy - radius + 2.0f, side - 4.0f, side - 4.0f, 1.0f);

    juce::Path track;
    track.addCentredArc(cx, cy, radius + 6.0f, radius + 6.0f, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xff382c45));
    g.strokePath(track, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    if (sliderPos > 0.0005f)
    {
        juce::Path active;
        active.addCentredArc(cx, cy, radius + 6.0f, radius + 6.0f, 0.0f,
                             startAngle, angle, true);
        g.setColour(violet.withAlpha(0.22f));
        g.strokePath(active, juce::PathStrokeType(9.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
        g.setColour(violet);
        g.strokePath(active, juce::PathStrokeType(4.8f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    // Metallic pointer with a small luminous tip.
    const float pointer = radius * 0.68f;
    const float px = cx + std::sin(angle) * pointer;
    const float py = cy - std::cos(angle) * pointer;
    g.setColour(juce::Colour(0xfff2eaff));
    g.drawLine(cx + std::sin(angle) * 4.0f, cy - std::cos(angle) * 4.0f,
               px, py, 2.6f);
    g.setColour(bright);
    g.fillEllipse(px - 2.0f, py - 2.0f, 4.0f, 4.0f);
}

void GumzyyLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float, float, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 0.0f, 1.0f, style, slider);
        return;
    }
    const float trackH = 6.0f;
    const float cy = static_cast<float>(y) + static_cast<float>(height) * 0.43f;
    auto track = juce::Rectangle<float>(static_cast<float>(x) + 5.0f, cy - trackH * 0.5f,
                                        static_cast<float>(width) - 10.0f, trackH);
    g.setColour(meterBg);
    g.fillRoundedRectangle(track, 3.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(track, 3.0f, 0.8f);
    const float proportion = juce::jlimit(0.0f, 1.0f,
        (sliderPos - static_cast<float>(x) - 5.0f) / juce::jmax(1.0f, track.getWidth()));
    auto active = track.withWidth(track.getWidth() * proportion);
    g.setColour(violet.withAlpha(0.22f));
    g.fillRoundedRectangle(active.expanded(0.0f, 3.0f), 4.0f);
    g.setColour(violet);
    g.fillRoundedRectangle(active, 3.0f);
    const float knobX = track.getX() + proportion * track.getWidth();
    g.setColour(bright);
    g.fillEllipse(knobX - 7.0f, cy - 7.0f, 14.0f, 14.0f);
    g.setColour(violet);
    g.drawEllipse(knobX - 7.0f, cy - 7.0f, 14.0f, 14.0f, 1.5f);
}

void GumzyyLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour&, bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool active = button.getToggleState();
    const auto fill = active ? juce::Colour(0xff3b2252)
                             : (down ? juce::Colour(0xff30223d)
                                     : (highlighted ? juce::Colour(0xff271d31) : raised));
    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 9.0f);
    if (active)
    {
        g.setColour(violet.withAlpha(0.16f));
        g.fillRoundedRectangle(bounds.reduced(3.0f), 7.0f);
    }
    g.setColour(active ? bright : edge);
    g.drawRoundedRectangle(bounds, 9.0f, active ? 1.8f : 1.0f);
    if (active)
    {
        g.setColour(violet.withAlpha(0.75f));
        g.fillEllipse(bounds.getCentreX() - 3.0f, bounds.getBottom() - 7.0f, 6.0f, 3.0f);
    }
}

juce::Font GumzyyLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    const auto fontSize = juce::jlimit(10.0f, 16.0f, static_cast<float>(buttonHeight) * 0.36f);
    return juce::Font(juce::FontOptions(fontSize, juce::Font::bold));
}

void GumzyyLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    const bool active = button.getToggleState();
    const auto colourId = active ? juce::TextButton::textColourOnId
                                 : juce::TextButton::textColourOffId;
    auto textColour = button.findColour(colourId);
    if (!button.isEnabled())
        textColour = textColour.withAlpha(0.42f);

    g.setColour(textColour);
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(7, 2),
                     juce::Justification::centred, 1);
}

GumzyyCallFXAudioProcessorEditor::GumzyyCallFXAudioProcessorEditor(GumzyyCallFXAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p),
      phoneBanner_(juce::ImageCache::getFromMemory(BinaryData::phone_banner_png,
                                                   BinaryData::phone_banner_pngSize))
{
    setOpaque(true);
    setResizable(true, true);
    setResizeLimits(1080, 720, 1800, 1200);
    getConstrainer()->setFixedAspectRatio(1.5);
    setSize(1440, 960);

    auto styleLabel = [](juce::Label& label, const juce::String& text, float size,
                         juce::Colour colour, bool italic = false)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(size, juce::Font::bold | (italic ? juce::Font::italic : 0))));
        label.setColour(juce::Label::textColourId, colour);
        label.setJustificationType(juce::Justification::centredLeft);
    };
    styleLabel(brand_, "OFF SESSION  ✦", 24.0f, white, true);
    styleLabel(product_, "GUMZYY CALL FX", 27.0f, bright, true);
    styleLabel(subtitle_, "AD-LIB COLOR LAB  /  NYXV x OFF SESSION", 10.0f, muted);
    addAndMakeVisible(brand_);
    addAndMakeVisible(product_);
    addAndMakeVisible(subtitle_);

    const juce::StringArray styleNames { "ANGEL", "PHONE", "RADIO", "WALKIE", "MEGA", "BROKEN" };
    const juce::StringArray styleTips {
        "ANGEL - airy, wide, harmonic shimmer for floating ad-libs.",
        "PHONE - narrow telephone band with a focused midrange.",
        "RADIO - mid-forward saturation and vintage signal texture.",
        "WALKIE - aggressive narrow-band communication voice.",
        "MEGA - projected megaphone bite with controlled grit.",
        "BROKEN - digital degradation, crush and lo-fi movement."
    };
    for (int i = 0; i < static_cast<int>(styleButtons_.size()); ++i)
    {
        auto& button = styleButtons_[static_cast<std::size_t>(i)];
        button.setButtonText(styleNames[i]);
        button.setLookAndFeel(&lookAndFeel_);
        button.setClickingTogglesState(true);
        button.setColour(juce::TextButton::textColourOffId, white);
        button.setColour(juce::TextButton::textColourOnId, bright);
        button.setTooltip(styleTips[i] + " All controls stay editable and automatable.");
        button.onClick = [this, i]
        {
            setParameter("style", static_cast<float>(i));
            applyPreset(i);
            abReady_ = false;
            abHasB_ = false;
            updateStyleButtons();
            setStatus(styleButtons_[static_cast<std::size_t>(i)].getButtonText() + " chain loaded - tweak to taste");
        };
        addAndMakeVisible(button);
    }

    auto prepareUtilityButton = [this](juce::TextButton& button, const juce::String& tooltip)
    {
        button.setLookAndFeel(&lookAndFeel_);
        button.setColour(juce::TextButton::textColourOffId, white);
        button.setColour(juce::TextButton::textColourOnId, bright);
        button.setTooltip(tooltip);
        addAndMakeVisible(button);
    };
    prepareUtilityButton(abButton_, "Save and switch between two complete effect settings.");
    abButton_.onClick = [this] { handleABCompare(); };
    prepareUtilityButton(randomButton_, "Generate a controlled variation that stays musical for the selected ad-lib style.");
    randomButton_.onClick = [this] { randomizeCurrentStyle(); };
    prepareUtilityButton(resetButton_, "Restore the selected style's factory starting chain.");
    resetButton_.onClick = [this] { resetCurrentStyle(); };
    prepareUtilityButton(sizeButton_, "Switch between the full BIG console and the compact SMALL layout.");
    sizeButton_.onClick = [this]
    {
        compact_ = !compact_;
        sizeButton_.setButtonText(compact_ ? "BIG" : "SMALL");
        setSize(compact_ ? 1120 : 1440, compact_ ? 747 : 960);
    };
    prepareUtilityButton(bypassButton_, "Temporarily bypass all GUMZYY processing.");
    bypassButton_.setClickingTogglesState(true);
    bypassAttachment_ = std::make_unique<ButtonAttachment>(processor_.parameters, "bypass", bypassButton_);

    addKnob(hp_, hpLabel_, "HP", " Hz", "High-pass filter - clear rumble and proximity build-up.", 80.0);
    addKnob(lp_, lpLabel_, "LP", " Hz", "Low-pass filter - shape the small-speaker or vintage-radio top end.", 18000.0);
    addKnob(input_, inputLabel_, "INPUT", " dB", "Pre-effect gain. Push the voice into the colour stage.", 0.0);
    addKnob(tone_, toneLabel_, "TONE", "%", "Broad low-to-high tilt. Left is darker, right is brighter.", 0.0);
    addKnob(air_, airLabel_, "AIR", " dB", "Zero-latency high-frequency shelf for breath and presence.", 3.5);
    addKnob(shimmer_, shimmerLabel_, "SHIMMER", "%", "Harmonic high-band exciter for angelic sparkle. Not a pitch shifter.", 0.0);
    addKnob(width_, widthLabel_, "WIDTH", "%", "Mid/side stereo width. Check wide settings in mono.", 140.0);
    addKnob(drive_, driveLabel_, "DRIVE", "%", "Saturation and density. More drive creates stronger harmonics.", 10.0);
    addKnob(crush_, crushLabel_, "CRUSH", "%", "Bit-depth and sample-rate reduction for radio and broken textures.", 0.0);
    addKnob(reverb_, reverbLabel_, "REVERB", "%", "Algorithmic room / tail level. Wet-path only.", 30.0);
    addKnob(delay_, delayLabel_, "DELAY", "%", "Echo send level. Wet-path only, dry stays immediate.", 18.0);
    addKnob(delayTime_, delayTimeLabel_, "TIME", " ms", "Delay time from 60 to 800 milliseconds.", 280.0);
    addKnob(feedback_, feedbackLabel_, "FEEDBACK", "%", "Amount of each echo fed back into the delay line.", 24.0);

    hp_.setRange(20.0, 1200.0, 1.0); lp_.setRange(1000.0, 20000.0, 1.0);
    input_.setRange(-18.0, 18.0, 0.01); tone_.setRange(-100.0, 100.0, 0.1);
    air_.setRange(0.0, 12.0, 0.01); shimmer_.setRange(0.0, 100.0, 0.1);
    width_.setRange(0.0, 200.0, 0.1); drive_.setRange(0.0, 100.0, 0.1);
    crush_.setRange(0.0, 100.0, 0.1); reverb_.setRange(0.0, 100.0, 0.1);
    delay_.setRange(0.0, 100.0, 0.1); delayTime_.setRange(60.0, 800.0, 1.0);
    feedback_.setRange(0.0, 75.0, 0.1);

    mixLabel_.setText("WET / DRY", juce::dontSendNotification);
    mixLabel_.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    mixLabel_.setColour(juce::Label::textColourId, muted);
    mixLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(mixLabel_);
    mix_.setLookAndFeel(&lookAndFeel_);
    mix_.setSliderStyle(juce::Slider::LinearHorizontal);
    mix_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 22);
    mix_.setRange(0.0, 100.0, 0.1);
    mix_.setTextValueSuffix(" %");
    mix_.setColour(juce::Slider::textBoxTextColourId, white);
    mix_.setColour(juce::Slider::textBoxBackgroundColourId, raised);
    mix_.setColour(juce::Slider::textBoxOutlineColourId, edge);
    mix_.setTooltip("Blend unprocessed voice with the complete wet effect chain.");
    addAndMakeVisible(mix_);

    addKnob(output_, outputLabel_, "OUTPUT", " dB", "Final level trim after wet/dry mixing.", 0.0);
    output_.setRange(-18.0, 12.0, 0.01);

    auto& apvts = processor_.parameters;
    hpAttachment_ = std::make_unique<SliderAttachment>(apvts, "hp", hp_);
    lpAttachment_ = std::make_unique<SliderAttachment>(apvts, "lp", lp_);
    inputAttachment_ = std::make_unique<SliderAttachment>(apvts, "input", input_);
    toneAttachment_ = std::make_unique<SliderAttachment>(apvts, "tone", tone_);
    airAttachment_ = std::make_unique<SliderAttachment>(apvts, "air", air_);
    shimmerAttachment_ = std::make_unique<SliderAttachment>(apvts, "shimmer", shimmer_);
    widthAttachment_ = std::make_unique<SliderAttachment>(apvts, "width", width_);
    driveAttachment_ = std::make_unique<SliderAttachment>(apvts, "drive", drive_);
    crushAttachment_ = std::make_unique<SliderAttachment>(apvts, "crush", crush_);
    reverbAttachment_ = std::make_unique<SliderAttachment>(apvts, "reverb", reverb_);
    delayAttachment_ = std::make_unique<SliderAttachment>(apvts, "delay", delay_);
    delayTimeAttachment_ = std::make_unique<SliderAttachment>(apvts, "delayTime", delayTime_);
    feedbackAttachment_ = std::make_unique<SliderAttachment>(apvts, "feedback", feedback_);
    mixAttachment_ = std::make_unique<SliderAttachment>(apvts, "mix", mix_);
    outputAttachment_ = std::make_unique<SliderAttachment>(apvts, "output", output_);

    statusLabel_.setText("READY - PICK A CALL STYLE AND SHAPE YOUR AD-LIB", juce::dontSendNotification);
    statusLabel_.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    statusLabel_.setColour(juce::Label::textColourId, bright);
    statusLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel_);

    updateStyleButtons();
    startTimerHz(30);
}

GumzyyCallFXAudioProcessorEditor::~GumzyyCallFXAudioProcessorEditor()
{
    stopTimer();
    clearLookAndFeel();
}

void GumzyyCallFXAudioProcessorEditor::clearLookAndFeel()
{
    for (auto& button : styleButtons_) button.setLookAndFeel(nullptr);
    sizeButton_.setLookAndFeel(nullptr);
    bypassButton_.setLookAndFeel(nullptr);
    abButton_.setLookAndFeel(nullptr);
    randomButton_.setLookAndFeel(nullptr);
    resetButton_.setLookAndFeel(nullptr);
    juce::Slider* sliders[] = { &hp_, &lp_, &input_, &tone_, &air_, &shimmer_, &width_, &drive_, &crush_,
                                &reverb_, &delay_, &delayTime_, &feedback_, &mix_, &output_ };
    for (auto* slider : sliders) slider->setLookAndFeel(nullptr);
}

void GumzyyCallFXAudioProcessorEditor::addKnob(juce::Slider& slider, juce::Label& label,
    const juce::String& title, const juce::String& suffix, const juce::String& tooltip, double defaultValue)
{
    label.setText(title, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, muted);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    slider.setLookAndFeel(&lookAndFeel_);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::textBoxTextColourId, white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, raised);
    slider.setColour(juce::Slider::textBoxOutlineColourId, edge);
    slider.setTooltip(tooltip + " Double-click the knob to reset it.");
    slider.setDoubleClickReturnValue(true, defaultValue);
    addAndMakeVisible(slider);
}

void GumzyyCallFXAudioProcessorEditor::setParameter(const juce::String& id, float value)
{
    if (auto* parameter = processor_.parameters.getParameter(id))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

void GumzyyCallFXAudioProcessorEditor::applyPreset(int index)
{
    struct Preset
    {
        float hp, lp, input, tone, air, shimmer, drive, crush, width;
        float reverb, delay, time, feedback, mix, output;
    };
    static const Preset presets[] = {
        {  85, 18000,  0.0f,  24, 8.0f, 62,  5,  0, 158, 48, 22, 430, 26, 100,  0 }, // Angel
        { 300,  3400,  0.0f, -18, 0.0f,  0, 18,  0, 100,  0,  0, 260, 18, 100,  0 }, // Phone
        { 170,  6200, -1.0f, -10, 1.0f,  4, 32, 12, 100,  5,  0, 240, 18, 100, -1 }, // Radio
        { 470,  2850,  1.0f, -14, 0.0f,  0, 46, 34, 100,  0,  0, 180, 15, 100, -1 }, // Walkie
        { 220,  4400,  0.0f,  10, 0.0f,  8, 28,  9, 100,  3,  0, 250, 18, 100,  0 }, // Mega
        { 125,  7000, -2.0f,   8, 1.0f, 18, 54, 70, 118, 12, 32, 185, 32, 100, -2 }  // Broken
    };
    index = juce::jlimit(0, 5, index);
    const auto& v = presets[index];
    setParameter("hp", v.hp); setParameter("lp", v.lp); setParameter("input", v.input);
    setParameter("tone", v.tone); setParameter("air", v.air); setParameter("shimmer", v.shimmer);
    setParameter("drive", v.drive); setParameter("crush", v.crush); setParameter("width", v.width);
    setParameter("reverb", v.reverb); setParameter("delay", v.delay); setParameter("delayTime", v.time);
    setParameter("feedback", v.feedback); setParameter("mix", v.mix); setParameter("output", v.output);
}

void GumzyyCallFXAudioProcessorEditor::drawModule(juce::Graphics& g, juce::Rectangle<float> bounds,
    const juce::String& number, const juce::String& title, const juce::String& subtitle)
{
    g.setColour(chassis);
    g.fillRoundedRectangle(bounds.translated(0.0f, 3.0f), 13.0f);
    g.setColour(panel);
    g.fillRoundedRectangle(bounds, 13.0f);
    g.setColour(edge.withAlpha(0.9f));
    g.drawRoundedRectangle(bounds, 13.0f, 1.0f);
    auto strip = bounds.removeFromTop(4.0f);
    g.setColour(violet.withAlpha(0.7f));
    g.fillRoundedRectangle(strip, 2.0f);
    auto titleArea = juce::Rectangle<float>(bounds.getX() + 14.0f, bounds.getY() + 9.0f,
                                                  bounds.getWidth() - 28.0f, 19.0f);
    g.setColour(violet);
    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.drawText(number, titleArea.removeFromLeft(25.0f), juce::Justification::centredLeft);
    g.setColour(bright);
    g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    g.drawText(title, titleArea, juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
    g.drawText(subtitle, bounds.getX() + 14.0f, bounds.getY() + 30.0f,
               bounds.getWidth() - 28.0f, 15.0f, juce::Justification::centredLeft);
}

void GumzyyCallFXAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    auto outer = getLocalBounds().toFloat().reduced(9.0f);
    g.setColour(chassis);
    g.fillRoundedRectangle(outer, 18.0f);
    g.setColour(juce::Colour(0xff2b2135));
    g.drawRoundedRectangle(outer, 18.0f, 1.4f);
    g.setColour(violet.withAlpha(0.12f));
    g.drawRoundedRectangle(outer.reduced(4.0f), 15.0f, 1.0f);

    const float s = static_cast<float>(getWidth()) / 1440.0f;
    auto sx = [s](float v) { return v * s; };

    // Header chassis and the actual retro handset / GUMZYY illustration.
    auto header = juce::Rectangle<float>(sx(24), sx(17), sx(1392), sx(218));
    g.setColour(panel);
    g.fillRoundedRectangle(header, sx(14));
    g.setColour(edge);
    g.drawRoundedRectangle(header, sx(14), 1.0f);
    if (!phoneBanner_.isNull())
        g.drawImage(phoneBanner_, juce::Rectangle<float>(sx(300), sx(18), sx(940), sx(214)),
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);

    // The top-right utility bay is a deliberate part of the hardware-like faceplate.
    auto utilityBay = juce::Rectangle<float>(sx(1250), sx(26), sx(154), sx(194));
    g.setColour(juce::Colour(0xff0b0910));
    g.fillRoundedRectangle(utilityBay, sx(10));
    g.setColour(edge.withAlpha(0.65f));
    g.drawRoundedRectangle(utilityBay, sx(10), 1.0f);

    // Character selector rail.
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(sx(9.5f), juce::Font::bold)));
    g.drawText("CALL CHARACTERS", sx(30), sx(239), sx(180), sx(17), juce::Justification::centredLeft);
    g.setColour(dim);
    g.setFont(juce::Font(juce::FontOptions(sx(8.0f), juce::Font::bold)));
    g.drawText("ONE CLICK CHAINS  /  EVERY KNOB STAYS YOURS", sx(205), sx(240), sx(390), sx(16),
               juce::Justification::centredLeft);

    const float margin = sx(28.0f);
    const float gap = sx(14.0f);
    const float y = sx(328.0f);
    const float h = sx(462.0f);
    const float widths[] = { sx(330.0f), sx(330.0f), sx(260.0f), sx(422.0f) };
    float x = margin;
    drawModule(g, { x, y, widths[0], h }, "01", "VOICE / FILTER", "INPUT STAGE  -  SHAPE THE RAW AD-LIB"); x += widths[0] + gap;
    drawModule(g, { x, y, widths[1], h }, "02", "ANGEL / STEREO", "AIR, HARMONICS  -  MAKE IT FLOAT"); x += widths[1] + gap;
    drawModule(g, { x, y, widths[2], h }, "03", "COLOR / GRIT", "SATURATION  -  TEXTURE"); x += widths[2] + gap;
    drawModule(g, { x, y, widths[3], h }, "04", "SPACE / ECHO", "DEPTH  -  REPEATS  -  ATMOSPHERE");

    // Module micro-guides make each section understandable at a glance.
    g.setColour(dim);
    g.setFont(juce::Font(juce::FontOptions(sx(7.6f), juce::Font::bold)));
    g.drawText("CUT LOWS / CONTROL TOP / PUSH INPUT / TILT TONE",
               sx(42), sx(761), sx(300), sx(13), juce::Justification::centred);
    g.drawText("SHIMMER IS HARMONIC EXCITER - NO PITCH SHIFT",
               sx(382), sx(761), sx(300), sx(13), juce::Justification::centred);
    g.drawText("KEEP IT SUBTLE OR BREAK IT ON PURPOSE",
               sx(716), sx(761), sx(235), sx(13), juce::Justification::centred);
    g.drawText("DELAY + REVERB LIVE ON THE WET PATH",
               sx(984), sx(761), sx(390), sx(13), juce::Justification::centred);

    // Footer / mix desk.
    const auto footer = juce::Rectangle<float>(sx(28), sx(805), sx(1384), sx(134));
    g.setColour(panel);
    g.fillRoundedRectangle(footer, sx(12));
    g.setColour(edge);
    g.drawRoundedRectangle(footer, sx(12), 1.0f);

    auto drawMeter = [&g, s](float xPos, float yPos, float width, float level, const juce::String& title)
    {
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.8f * s, juce::Font::bold)));
        g.drawText(title, xPos, yPos, width, 14.0f * s, juce::Justification::centredLeft);
        auto track = juce::Rectangle<float>(xPos, yPos + 18.0f * s, width, 9.0f * s);
        g.setColour(meterBg);
        g.fillRoundedRectangle(track, 4.0f * s);
        const auto colour = level > 0.94f ? danger : violet;
        g.setColour(colour.withAlpha(0.16f));
        g.fillRoundedRectangle(track.expanded(0.0f, 2.0f * s), 4.0f * s);
        g.setColour(colour);
        g.fillRoundedRectangle(track.withWidth(track.getWidth() * level), 4.0f * s);
        g.setColour(edge);
        g.drawRoundedRectangle(track, 4.0f * s, 0.8f * s);
    };
    const float inPeak = safeMeterLevel(processor_.inputPeak());
    const float outPeak = safeMeterLevel(processor_.outputPeak());
    drawMeter(sx(48), sx(835), sx(185), inPeak, "INPUT  " + peakToDb(processor_.inputPeak()) + " dB");
    drawMeter(sx(1188), sx(835), sx(190), outPeak, "OUTPUT  " + peakToDb(processor_.outputPeak()) + " dB");

    // Latency indicator and small hardware markings.
    g.setColour(violet.withAlpha(0.18f));
    g.fillRoundedRectangle(juce::Rectangle<float>(sx(250), sx(832), sx(202), sx(28)), sx(7));
    g.setColour(bright);
    g.setFont(juce::Font(juce::FontOptions(sx(9.0f), juce::Font::bold)));
    g.drawText("0 SAMPLES  /  NO LOOK-AHEAD", sx(260), sx(837), sx(184), sx(17),
               juce::Justification::centred);
    g.setColour(dim);
    g.setFont(juce::Font(juce::FontOptions(sx(7.0f), juce::Font::bold)));
    g.drawText("LOW-LATENCY SIGNAL PATH", sx(260), sx(866), sx(184), sx(13), juce::Justification::centred);
}

void GumzyyCallFXAudioProcessorEditor::resized()
{
    const float s = static_cast<float>(getWidth()) / 1440.0f;
    auto S = [s](float v) { return static_cast<int>(std::round(v * s)); };

    brand_.setBounds(S(42), S(49), S(248), S(31));
    brand_.setFont(juce::Font(juce::FontOptions(24.0f * s, juce::Font::bold | juce::Font::italic)));
    product_.setBounds(S(42), S(83), S(255), S(34));
    product_.setFont(juce::Font(juce::FontOptions(27.0f * s, juce::Font::bold | juce::Font::italic)));
    subtitle_.setBounds(S(44), S(120), S(250), S(24));
    subtitle_.setFont(juce::Font(juce::FontOptions(10.0f * s, juce::Font::bold)));

    sizeButton_.setBounds(S(1263), S(38), S(128), S(31));
    bypassButton_.setBounds(S(1263), S(77), S(128), S(31));
    abButton_.setBounds(S(1263), S(119), S(61), S(31));
    randomButton_.setBounds(S(1330), S(119), S(61), S(31));
    resetButton_.setBounds(S(1263), S(161), S(128), S(31));

    const int styleX = S(28), styleY = S(263), styleH = S(53), styleGap = S(10);
    const int styleW = (S(1384) - styleGap * 5) / 6;
    for (int i = 0; i < 6; ++i)
    {
        auto& b = styleButtons_[static_cast<std::size_t>(i)];
        b.setBounds(styleX + i * (styleW + styleGap), styleY, styleW, styleH);
    }

    const float moduleY = 328.0f;
    const float moduleH = 462.0f;
    const float margin = 28.0f;
    const float gap = 14.0f;
    const float moduleX[] = { margin, margin + 330.0f + gap, margin + 660.0f + 2.0f * gap,
                              margin + 920.0f + 3.0f * gap };
    const float moduleW[] = { 330.0f, 330.0f, 260.0f, 422.0f };
    const float controlsTop = moduleY + 62.0f;
    const float controlsBottom = moduleY + moduleH - 38.0f;
    const float rowGap = 12.0f;

    struct KnobItem { juce::Slider* slider; juce::Label* label; int module; int slot; };
    const KnobItem items[] = {
        { &hp_, &hpLabel_, 0, 0 }, { &lp_, &lpLabel_, 0, 1 },
        { &input_, &inputLabel_, 0, 2 }, { &tone_, &toneLabel_, 0, 3 },
        { &air_, &airLabel_, 1, 0 }, { &shimmer_, &shimmerLabel_, 1, 1 }, { &width_, &widthLabel_, 1, 2 },
        { &drive_, &driveLabel_, 2, 0 }, { &crush_, &crushLabel_, 2, 1 },
        { &reverb_, &reverbLabel_, 3, 0 }, { &delay_, &delayLabel_, 3, 1 },
        { &delayTime_, &delayTimeLabel_, 3, 2 }, { &feedback_, &feedbackLabel_, 3, 3 }
    };

    for (const auto& item : items)
    {
        const float mx = moduleX[item.module];
        const float mw = moduleW[item.module];
        const bool threeAcross = item.module == 1;
        const bool twoAcrossSingleRow = item.module == 2;
        const int columns = threeAcross ? 3 : 2;
        const int row = (item.module == 0 || item.module == 3) ? item.slot / 2 : 0;
        const int col = (item.module == 0 || item.module == 3) ? item.slot % 2 : item.slot;
        const float innerX = mx + 10.0f;
        const float innerW = mw - 20.0f;
        const float cellW = innerW / static_cast<float>(columns);
        const float cellH = (item.module == 0 || item.module == 3)
            ? (controlsBottom - controlsTop - rowGap) * 0.5f
            : controlsBottom - controlsTop;
        const float cellX = innerX + col * cellW;
        const float cellY = controlsTop + row * (cellH + rowGap);
        const float knobW = std::min(cellW - 5.0f, threeAcross ? 112.0f : (twoAcrossSingleRow ? 122.0f : 166.0f));
        const float knobH = std::min(cellH - 21.0f, (item.module == 0 || item.module == 3) ? 150.0f : 190.0f);
        item.label->setBounds(S(cellX), S(cellY + 2.0f), S(cellW), S(18.0f));
        item.slider->setBounds(S(cellX + (cellW - knobW) * 0.5f), S(cellY + 20.0f), S(knobW), S(knobH));
        item.label->setFont(juce::Font(juce::FontOptions(9.5f * s, juce::Font::bold)));
    }

    statusLabel_.setBounds(S(48), S(889), S(410), S(20));
    mixLabel_.setBounds(S(535), S(821), S(130), S(17));
    mix_.setBounds(S(535), S(841), S(355), S(49));
    outputLabel_.setBounds(S(948), S(817), S(110), S(16));
    output_.setBounds(S(936), S(831), S(128), S(92));
    mixLabel_.setFont(juce::Font(juce::FontOptions(9.5f * s, juce::Font::bold)));
}

void GumzyyCallFXAudioProcessorEditor::timerCallback()
{
    updateStyleButtons();
    repaint();
}

juce::String GumzyyCallFXAudioProcessorEditor::peakToDb(float peak)
{
    if (!std::isfinite(peak) || peak <= 0.000001f)
        return "-INF";
    return juce::String(20.0f * std::log10(peak), 1);
}

void GumzyyCallFXAudioProcessorEditor::updateStyleButtons()
{
    const auto* raw = processor_.parameters.getRawParameterValue("style");
    const int style = raw != nullptr ? juce::jlimit(0, 5, static_cast<int>(std::round(raw->load()))) : 0;
    for (int i = 0; i < 6; ++i)
        styleButtons_[static_cast<std::size_t>(i)].setToggleState(i == style, juce::dontSendNotification);
}

void GumzyyCallFXAudioProcessorEditor::randomizeCurrentStyle()
{
    const auto* raw = processor_.parameters.getRawParameterValue("style");
    const int style = raw != nullptr ? juce::jlimit(0, 5, static_cast<int>(std::round(raw->load()))) : 0;
    const float hpBase[] = { 85.0f, 300.0f, 170.0f, 470.0f, 220.0f, 125.0f };
    const float lpBase[] = { 18000.0f, 3400.0f, 6200.0f, 2850.0f, 4400.0f, 7000.0f };
    const float driveBase[] = { 5.0f, 18.0f, 32.0f, 46.0f, 28.0f, 54.0f };
    const float crushBase[] = { 0.0f, 0.0f, 12.0f, 34.0f, 9.0f, 70.0f };
    auto vary = [this](float base, float spread, float low, float high)
    {
        return juce::jlimit(low, high, base + random_.nextFloat() * spread * 2.0f - spread);
    };
    setParameter("hp", vary(hpBase[style], style == 1 ? 75.0f : 110.0f, 20.0f, 1200.0f));
    setParameter("lp", vary(lpBase[style], style == 1 ? 700.0f : 1800.0f, 1000.0f, 20000.0f));
    setParameter("input", vary(0.0f, 2.5f, -6.0f, 6.0f));
    setParameter("tone", vary(style == 0 ? 24.0f : -8.0f, 24.0f, -100.0f, 100.0f));
    setParameter("air", vary(style == 0 ? 7.0f : 0.5f, 2.0f, 0.0f, 12.0f));
    setParameter("shimmer", vary(style == 0 ? 58.0f : (style == 5 ? 22.0f : 4.0f), 14.0f, 0.0f, 100.0f));
    setParameter("drive", vary(driveBase[style], 10.0f, 0.0f, 100.0f));
    setParameter("crush", vary(crushBase[style], style == 5 ? 18.0f : 9.0f, 0.0f, 100.0f));
    setParameter("width", vary(style == 0 ? 155.0f : 100.0f, 18.0f, 0.0f, 200.0f));
    setParameter("reverb", vary(style == 0 ? 48.0f : 5.0f, 18.0f, 0.0f, 100.0f));
    setParameter("delay", vary(style == 0 ? 22.0f : 8.0f, 16.0f, 0.0f, 100.0f));
    setParameter("delayTime", vary(330.0f, 150.0f, 60.0f, 800.0f));
    setParameter("feedback", vary(22.0f, 12.0f, 0.0f, 75.0f));
    setParameter("mix", 100.0f);
    abReady_ = false;
    abHasB_ = false;
    setStatus("New " + styleButtons_[static_cast<std::size_t>(style)].getButtonText() + " variation generated");
}

void GumzyyCallFXAudioProcessorEditor::resetCurrentStyle()
{
    const auto* raw = processor_.parameters.getRawParameterValue("style");
    const int style = raw != nullptr ? juce::jlimit(0, 5, static_cast<int>(std::round(raw->load()))) : 0;
    applyPreset(style);
    abReady_ = false;
    abHasB_ = false;
    setStatus(styleButtons_[static_cast<std::size_t>(style)].getButtonText() + " factory chain restored");
}

void GumzyyCallFXAudioProcessorEditor::captureSnapshot(Snapshot& snapshot)
{
    for (std::size_t i = 0; i < parameterIds.size(); ++i)
        if (const auto* raw = processor_.parameters.getRawParameterValue(parameterIds[i]))
            snapshot[i] = raw->load(std::memory_order_relaxed);
}

void GumzyyCallFXAudioProcessorEditor::restoreSnapshot(const Snapshot& snapshot)
{
    for (std::size_t i = 0; i < parameterIds.size(); ++i)
        setParameter(parameterIds[i], snapshot[i]);
}

void GumzyyCallFXAudioProcessorEditor::handleABCompare()
{
    if (!abReady_)
    {
        captureSnapshot(snapshotA_);
        abReady_ = true;
        abHasB_ = false;
        showingA_ = true;
        setStatus("A captured. Change any knobs, then press A/B again to capture B");
        return;
    }
    if (!abHasB_)
    {
        captureSnapshot(snapshotB_);
        abHasB_ = true;
        restoreSnapshot(snapshotA_);
        showingA_ = true;
        setStatus("A/B ready. Hearing A - press to switch to B");
        return;
    }
    if (showingA_)
    {
        restoreSnapshot(snapshotB_);
        showingA_ = false;
        setStatus("A/B compare - hearing B");
    }
    else
    {
        restoreSnapshot(snapshotA_);
        showingA_ = true;
        setStatus("A/B compare - hearing A");
    }
}

void GumzyyCallFXAudioProcessorEditor::setStatus(const juce::String& text)
{
    statusLabel_.setText(text, juce::dontSendNotification);
}
