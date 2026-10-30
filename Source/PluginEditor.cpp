#include "PluginEditor.h"
#include "BinaryData.h"
#include <algorithm>
#include <cmath>

namespace
{
const juce::Colour bg       { 0xff09080d };
const juce::Colour panel    { 0xff15121b };
const juce::Colour raised   { 0xff211a2a };
const juce::Colour edge     { 0xff48365b };
const juce::Colour violet   { 0xffa65cff };
const juce::Colour bright   { 0xffd8b7ff };
const juce::Colour white    { 0xfff2ecf8 };
const juce::Colour muted    { 0xffa99bb8 };
const juce::Colour meterBg  { 0xff08070b };

constexpr std::array<const char*, 15> parameterIds {
    "style", "hp", "lp", "input", "tone", "air", "width", "drive", "crush",
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
    const float d = static_cast<float>(juce::jmin(width, height)) - 18.0f;
    const float r = juce::jmax(8.0f, d * 0.5f);
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.40f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    g.setColour(juce::Colour(0xff07060a));
    g.fillEllipse(cx - r - 4.0f, cy - r - 4.0f, d + 8.0f, d + 8.0f);
    g.setColour(juce::Colour(0xff34303b));
    g.fillEllipse(cx - r, cy - r, d, d);
    g.setColour(juce::Colour(0xff111016));
    g.fillEllipse(cx - r + 7.0f, cy - r + 7.0f, d - 14.0f, d - 14.0f);
    g.setColour(juce::Colour(0xff5b4b69));
    g.drawEllipse(cx - r + 2.0f, cy - r + 2.0f, d - 4.0f, d - 4.0f, 1.0f);

    juce::Path track;
    track.addCentredArc(cx, cy, r + 5.0f, r + 5.0f, 0.0f, startAngle, endAngle, true);
    g.setColour(juce::Colour(0xff3b3046));
    g.strokePath(track, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (sliderPos > 0.001f)
    {
        juce::Path active;
        active.addCentredArc(cx, cy, r + 5.0f, r + 5.0f, 0.0f, startAngle, angle, true);
        g.setColour(violet);
        g.strokePath(active, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    const float pointer = r * 0.65f;
    g.setColour(bright);
    g.drawLine(cx + std::sin(angle) * 4.0f, cy - std::cos(angle) * 4.0f,
               cx + std::sin(angle) * pointer, cy - std::cos(angle) * pointer, 2.5f);
}

void GumzyyLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
    int, int, int, int, juce::ComboBox&)
{
    auto b = juce::Rectangle<float>(0.5f, 0.5f, static_cast<float>(width) - 1.0f, static_cast<float>(height) - 1.0f);
    g.setColour(isButtonDown ? juce::Colour(0xff3b2a50) : raised);
    g.fillRoundedRectangle(b, 8.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(b, 8.0f, 1.0f);
}

void GumzyyLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour&, bool highlighted, bool down)
{
    auto b = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool active = button.getToggleState();
    g.setColour(active ? juce::Colour(0xff47245f) : (down || highlighted ? juce::Colour(0xff352640) : raised));
    g.fillRoundedRectangle(b, 8.0f);
    g.setColour(active ? bright : edge);
    g.drawRoundedRectangle(b, 8.0f, active ? 1.6f : 1.0f);
    if (active)
    {
        g.setColour(violet.withAlpha(0.13f));
        g.fillRoundedRectangle(b.reduced(3.0f), 6.0f);
    }
}

GumzyyCallFXAudioProcessorEditor::GumzyyCallFXAudioProcessorEditor(GumzyyCallFXAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p),
      mascot_(juce::ImageCache::getFromMemory(BinaryData::gumzyy_png, BinaryData::gumzyy_pngSize))
{
    setOpaque(true);
    setResizable(true, true);
    setResizeLimits(900, 600, 1500, 980);
    getConstrainer()->setFixedAspectRatio(1260.0 / 820.0);
    setSize(1260, 820);

    brand_.setText("OFF SESSION | GUMZYY CALL FX", juce::dontSendNotification);
    brand_.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold | juce::Font::italic)));
    brand_.setColour(juce::Label::textColourId, white);
    brand_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(brand_);

    subtitle_.setText("AD-LIB COLOR LAB  /  NYXV x OFF SESSION", juce::dontSendNotification);
    subtitle_.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    subtitle_.setColour(juce::Label::textColourId, bright);
    subtitle_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subtitle_);

    const juce::StringArray styleNames { "ANGEL", "PHONE", "RADIO", "WALKIE", "MEGA", "BROKEN" };
    for (int i = 0; i < static_cast<int>(styleButtons_.size()); ++i)
    {
        auto& button = styleButtons_[static_cast<std::size_t>(i)];
        button.setButtonText(styleNames[i]);
        button.setLookAndFeel(&lookAndFeel_);
        button.setClickingTogglesState(true);
        button.setColour(juce::TextButton::textColourOffId, white);
        button.setColour(juce::TextButton::textColourOnId, bright);
        button.setTooltip("Load the " + styleNames[i] + " ad-lib starting chain. All controls remain editable.");
        button.onClick = [this, i]
        {
            setParameter("style", static_cast<float>(i));
            applyPreset(i);
            abReady_ = false;
            abHasB_ = false;
            updateStyleButtons();
            setStatus(styleButtons_[static_cast<std::size_t>(i)].getButtonText() + " chain loaded. Adjust the knobs.");
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

    prepareUtilityButton(abButton_, "Save and compare two complete effect settings.");
    abButton_.onClick = [this] { handleABCompare(); };

    prepareUtilityButton(randomButton_, "Create a controlled variation of the selected ad-lib style.");
    randomButton_.onClick = [this] { randomizeCurrentStyle(); };

    prepareUtilityButton(resetButton_, "Restore the starting settings for the selected style.");
    resetButton_.onClick = [this] { resetCurrentStyle(); };

    prepareUtilityButton(sizeButton_, "Switch between the full and compact interface.");
    sizeButton_.onClick = [this]
    {
        compact_ = !compact_;
        sizeButton_.setButtonText(compact_ ? "BIG" : "SMALL");
        getConstrainer()->setFixedAspectRatio(compact_ ? (940.0 / 620.0) : (1260.0 / 820.0));
        setSize(compact_ ? 940 : 1260, compact_ ? 620 : 820);
    };

    prepareUtilityButton(bypassButton_, "Temporarily bypass all GUMZYY processing.");
    bypassButton_.setClickingTogglesState(true);
    bypassAttachment_ = std::make_unique<ButtonAttachment>(processor_.parameters, "bypass", bypassButton_);

    addKnob(hp_, hpLabel_, "HIGH PASS", " Hz", "Remove low rumble and proximity build-up.", 80.0);
    addKnob(lp_, lpLabel_, "LOW PASS", " Hz", "Darken the ad-lib or create the narrow phone speaker band.", 18000.0);
    addKnob(input_, inputLabel_, "INPUT", " dB", "Drive the effect harder before tone and colour processing.", 0.0);
    addKnob(tone_, toneLabel_, "TONE", "%", "Tilt the overall vocal balance from darker to brighter.", 0.0);
    addKnob(air_, airLabel_, "AIR", " dB", "Add high-frequency air for bright, floating ad-lib layers.", 3.5);
    addKnob(width_, widthLabel_, "WIDTH", "%", "Control the stereo spread. Check wide settings in mono.", 140.0);
    addKnob(drive_, driveLabel_, "DRIVE", "%", "Add saturation and density to the voice.", 10.0);
    addKnob(crush_, crushLabel_, "CRUSH", "%", "Reduce bit depth and sample rate for radio and digital grit.", 0.0);
    addKnob(reverb_, reverbLabel_, "REVERB", "%", "Add a short or long ambience to the wet signal.", 30.0);
    addKnob(delay_, delayLabel_, "DELAY", "%", "Blend in tempo-free echo from the wet path.", 18.0);
    addKnob(delayTime_, delayTimeLabel_, "TIME", " ms", "Set the delay time from 60 to 800 milliseconds.", 280.0);
    addKnob(feedback_, feedbackLabel_, "FEEDBACK", "%", "Set how many delay repeats are fed back.", 24.0);
    addKnob(mix_, mixLabel_, "MIX", "%", "Blend dry voice with the processed ad-lib.", 100.0);
    addKnob(output_, outputLabel_, "OUTPUT", " dB", "Set final output level after the effect.", 0.0);

    hp_.setRange(20.0, 1200.0, 1.0); lp_.setRange(1000.0, 20000.0, 1.0);
    input_.setRange(-18.0, 18.0, 0.01); tone_.setRange(-100.0, 100.0, 0.1);
    air_.setRange(0.0, 12.0, 0.01); width_.setRange(0.0, 200.0, 0.1);
    drive_.setRange(0.0, 100.0, 0.1); crush_.setRange(0.0, 100.0, 0.1);
    reverb_.setRange(0.0, 100.0, 0.1); delay_.setRange(0.0, 100.0, 0.1);
    delayTime_.setRange(60.0, 800.0, 1.0); feedback_.setRange(0.0, 75.0, 0.1);
    mix_.setRange(0.0, 100.0, 0.1); output_.setRange(-18.0, 12.0, 0.01);

    auto& apvts = processor_.parameters;
    hpAttachment_ = std::make_unique<SliderAttachment>(apvts, "hp", hp_);
    lpAttachment_ = std::make_unique<SliderAttachment>(apvts, "lp", lp_);
    inputAttachment_ = std::make_unique<SliderAttachment>(apvts, "input", input_);
    toneAttachment_ = std::make_unique<SliderAttachment>(apvts, "tone", tone_);
    airAttachment_ = std::make_unique<SliderAttachment>(apvts, "air", air_);
    widthAttachment_ = std::make_unique<SliderAttachment>(apvts, "width", width_);
    driveAttachment_ = std::make_unique<SliderAttachment>(apvts, "drive", drive_);
    crushAttachment_ = std::make_unique<SliderAttachment>(apvts, "crush", crush_);
    reverbAttachment_ = std::make_unique<SliderAttachment>(apvts, "reverb", reverb_);
    delayAttachment_ = std::make_unique<SliderAttachment>(apvts, "delay", delay_);
    delayTimeAttachment_ = std::make_unique<SliderAttachment>(apvts, "delayTime", delayTime_);
    feedbackAttachment_ = std::make_unique<SliderAttachment>(apvts, "feedback", feedback_);
    mixAttachment_ = std::make_unique<SliderAttachment>(apvts, "mix", mix_);
    outputAttachment_ = std::make_unique<SliderAttachment>(apvts, "output", output_);

    statusLabel_.setText("0 SAMPLES ADDED", juce::dontSendNotification);
    statusLabel_.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    statusLabel_.setColour(juce::Label::textColourId, bright);
    statusLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel_);

    updateStyleButtons();
    startTimerHz(24);
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

    juce::Slider* sliders[] = { &hp_, &lp_, &input_, &tone_, &air_, &width_, &drive_, &crush_,
                                &reverb_, &delay_, &delayTime_, &feedback_, &mix_, &output_ };
    for (auto* slider : sliders) slider->setLookAndFeel(nullptr);
}

void GumzyyCallFXAudioProcessorEditor::addKnob(juce::Slider& slider, juce::Label& label,
    const juce::String& title, const juce::String& suffix, const juce::String& tooltip, double defaultValue)
{
    label.setText(title, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, muted);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    slider.setLookAndFeel(&lookAndFeel_);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 18);
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
    // Ad-lib starting chains. Every value maps to a real DSP parameter and remains automatable.
    struct Preset
    {
        float hp, lp, input, tone, air, drive, crush, width;
        float reverb, delay, time, feedback, mix, output;
    };
    static const Preset presets[] = {
        {  85, 18000,  0.0f,  24, 8.0f,  5,  0, 158, 48, 22, 430, 26, 100,  0 }, // Angel stack
        { 300,  3400,  0.0f, -18, 0.0f, 18,  0, 100,  0,  0, 260, 18, 100,  0 }, // Phone insert
        { 170,  6200, -1.0f, -10, 1.0f, 32, 12, 100,  5,  0, 240, 18, 100, -1 }, // Radio tag
        { 470,  2850,  1.0f, -14, 0.0f, 46, 34, 100,  0,  0, 180, 15, 100, -1 }, // Walkie hype
        { 220,  4400,  0.0f,  10, 0.0f, 28,  9, 100,  3,  0, 250, 18, 100,  0 }, // Megaphone shout
        { 125,  7000, -2.0f,   8, 1.0f, 54, 70, 118, 12, 32, 185, 32, 100, -2 }  // Broken digital
    };
    index = juce::jlimit(0, 5, index);
    const auto& v = presets[index];
    setParameter("hp", v.hp); setParameter("lp", v.lp); setParameter("input", v.input);
    setParameter("tone", v.tone); setParameter("air", v.air); setParameter("drive", v.drive);
    setParameter("crush", v.crush); setParameter("width", v.width); setParameter("reverb", v.reverb);
    setParameter("delay", v.delay); setParameter("delayTime", v.time); setParameter("feedback", v.feedback);
    setParameter("mix", v.mix); setParameter("output", v.output);
}

void GumzyyCallFXAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> b,
    const juce::String& title, const juce::String& accent)
{
    g.setColour(panel);
    g.fillRoundedRectangle(b, 12.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(b, 12.0f, 1.0f);
    g.setColour(violet.withAlpha(0.28f));
    g.fillRoundedRectangle(b.removeFromTop(3.0f), 2.0f);
    g.setColour(bright);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText(title, b.getX() + 12.0f, b.getY() + 8.0f, b.getWidth() - 24.0f, 18.0f,
               juce::Justification::centredLeft);
    g.setColour(violet.withAlpha(0.8f));
    g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
    g.drawText(accent, b.getX() + 12.0f, b.getY() + 28.0f, b.getWidth() - 24.0f, 14.0f,
               juce::Justification::centredLeft);
}

void GumzyyCallFXAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    const auto full = getLocalBounds().toFloat().reduced(8.0f);
    g.setColour(juce::Colour(0xff20182a));
    g.drawRoundedRectangle(full, 16.0f, 1.5f);

    const float w = static_cast<float>(getWidth());
    const bool small = compact_;
    const float displayX = small ? 300.0f : 380.0f;
    const float displayW = small ? 245.0f : 365.0f;
    const auto display = juce::Rectangle<float>(displayX, 15.0f, displayW, 92.0f);
    g.setColour(juce::Colour(0xff050409));
    g.fillRoundedRectangle(display, 12.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(display, 12.0f, 1.0f);
    g.setColour(bright);
    g.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    g.drawText("GUMZYY IS CALLING...", display.withY(display.getY() + 8.0f).withHeight(14.0f),
               juce::Justification::centred);

    const float inLevel = safeMeterLevel(processor_.inputPeak());
    const float outLevel = safeMeterLevel(processor_.outputPeak());
    auto drawSignal = [&g](float x, float y, float width, float level, const juce::String& name)
    {
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(7.5f, juce::Font::bold)));
        g.drawText(name, x, y - 1.0f, 27.0f, 12.0f, juce::Justification::centredLeft);
        auto track = juce::Rectangle<float>(x + 29.0f, y + 1.0f, width - 29.0f, 7.0f);
        g.setColour(meterBg);
        g.fillRoundedRectangle(track, 3.0f);
        g.setColour(level > 0.92f ? juce::Colour(0xffff6f87) : violet);
        g.fillRoundedRectangle(track.withWidth(track.getWidth() * level), 3.0f);
        g.setColour(edge);
        g.drawRoundedRectangle(track, 3.0f, 0.7f);
    };
    drawSignal(display.getX() + 16.0f, display.getY() + 39.0f, display.getWidth() - 32.0f, inLevel, "IN");
    drawSignal(display.getX() + 16.0f, display.getY() + 58.0f, display.getWidth() - 32.0f, outLevel, "OUT");

    if (!mascot_.isNull())
    {
        const int mascotSize = small ? 72 : 88;
        const int mascotX = getWidth() - (small ? 88 : 104);
        g.drawImageWithin(mascot_, mascotX, 9, mascotSize, mascotSize,
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }

    const float margin = small ? 15.0f : 22.0f;
    const float gap = small ? 8.0f : 12.0f;
    const float panelY = small ? 181.0f : 184.0f;
    const float footerY = static_cast<float>(getHeight()) - (small ? 88.0f : 94.0f);
    const float panelH = footerY - panelY - 12.0f;
    const float panelW = (w - margin * 2.0f - gap * 3.0f) / 4.0f;
    const char* names[] = { "01  FILTER / VOICE", "02  ANGEL / STEREO", "03  COLOR / GRIT", "04  SPACE / ECHO" };
    const char* details[] = { "CLEAN THE INPUT AND TILT THE TONE", "AIR AND WIDTH FOR AD-LIB LAYERS",
                              "SATURATION AND DIGITAL TEXTURE", "REVERB TAILS AND DELAY REPEATS" };
    for (int i = 0; i < 4; ++i)
    {
        auto b = juce::Rectangle<float>(margin + i * (panelW + gap), panelY, panelW, panelH);
        drawPanel(g, b, names[i], details[i]);
        if (i == 1 || i == 2)
        {
            g.setColour(muted.withAlpha(0.72f));
            g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::italic)));
            const auto note = (i == 1) ? "AIRY LAYERS  /  CHECK MONO" : "MATCH OUTPUT LEVEL WHEN A/B-ING";
            g.drawText(note, b.reduced(12.0f).withY(b.getBottom() - 25.0f).withHeight(15.0f),
                       juce::Justification::centred);
        }
    }

    auto footer = juce::Rectangle<float>(margin, footerY, w - margin * 2.0f, getHeight() - footerY - 12.0f);
    g.setColour(panel);
    g.fillRoundedRectangle(footer, 10.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(footer, 10.0f, 1.0f);

    auto drawFooterMeter = [&g](float x, float y, float width, float level, const juce::String& title)
    {
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
        g.drawText(title, x, y - 1.0f, width, 12.0f, juce::Justification::centredLeft);
        auto track = juce::Rectangle<float>(x, y + 13.0f, width, 7.0f);
        g.setColour(meterBg);
        g.fillRoundedRectangle(track, 3.0f);
        g.setColour(level > 0.92f ? juce::Colour(0xffff6f87) : violet);
        g.fillRoundedRectangle(track.withWidth(track.getWidth() * level), 3.0f);
        g.setColour(edge);
        g.drawRoundedRectangle(track, 3.0f, 0.7f);
    };
    const float footerX = footer.getX() + 14.0f;
    drawFooterMeter(footerX + (small ? 174.0f : 258.0f), footer.getY() + 10.0f,
                    small ? 118.0f : 180.0f, inLevel, "IN  " + peakToDb(processor_.inputPeak()) + " dB");
    drawFooterMeter(footerX + (small ? 310.0f : 458.0f), footer.getY() + 10.0f,
                    small ? 118.0f : 180.0f, outLevel, "OUT  " + peakToDb(processor_.outputPeak()) + " dB");
}

void GumzyyCallFXAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const bool small = compact_;
    brand_.setBounds(22, 22, small ? 255 : 325, 32);
    subtitle_.setBounds(24, 55, small ? 250 : 310, 18);

    const int displayX = small ? 300 : 380;
    const int displayW = small ? 245 : 365;
    int utilityX = displayX + displayW + 12;
    const int utilityY = 28;
    const int utilityGap = small ? 5 : 7;
    const int widthsBig[] = { 54, 68, 62, 60, 76 };
    const int widthsSmall[] = { 43, 53, 52, 48, 60 };
    juce::TextButton* utilities[] = { &abButton_, &randomButton_, &resetButton_, &sizeButton_, &bypassButton_ };
    const int* widths = small ? widthsSmall : widthsBig;
    for (int i = 0; i < 5; ++i)
    {
        utilities[i]->setBounds(utilityX, utilityY, widths[i], small ? 30 : 34);
        utilityX += widths[i] + utilityGap;
    }

    const int margin = small ? 15 : 22;
    const int gap = small ? 8 : 12;
    const int styleY = 122;
    const int styleH = 42;
    const int styleW = (w - margin * 2 - gap * 5) / 6;
    for (int i = 0; i < 6; ++i)
        styleButtons_[static_cast<std::size_t>(i)].setBounds(margin + i * (styleW + gap), styleY, styleW, styleH);

    const float panelY = small ? 181.0f : 184.0f;
    const float footerY = static_cast<float>(h) - (small ? 88.0f : 94.0f);
    const float panelH = footerY - panelY - 12.0f;
    const float panelW = static_cast<float>(w - margin * 2 - gap * 3) / 4.0f;

    struct Item { juce::Slider* slider; juce::Label* label; int panel; int slot; };
    const Item items[] = {
        { &hp_, &hpLabel_, 0, 0 }, { &lp_, &lpLabel_, 0, 1 },
        { &input_, &inputLabel_, 0, 2 }, { &tone_, &toneLabel_, 0, 3 },
        { &air_, &airLabel_, 1, 0 }, { &width_, &widthLabel_, 1, 1 },
        { &drive_, &driveLabel_, 2, 0 }, { &crush_, &crushLabel_, 2, 1 },
        { &reverb_, &reverbLabel_, 3, 0 }, { &delay_, &delayLabel_, 3, 1 },
        { &delayTime_, &delayTimeLabel_, 3, 2 }, { &feedback_, &feedbackLabel_, 3, 3 }
    };

    for (const auto& item : items)
    {
        const float panelX = static_cast<float>(margin) + item.panel * (panelW + gap);
        const bool twoRows = (item.panel == 0 || item.panel == 3);
        const int row = twoRows ? item.slot / 2 : 0;
        const int col = twoRows ? item.slot % 2 : item.slot;
        const float innerX = panelX + 7.0f;
        const float innerW = panelW - 14.0f;
        const float controlTop = panelY + 56.0f;
        const float controlBottom = panelY + panelH - 18.0f;
        const float cellW = innerW * 0.5f;
        const float cellH = twoRows ? (controlBottom - controlTop) * 0.5f : (controlBottom - controlTop);
        const float x = innerX + col * cellW;
        const float y = controlTop + row * cellH;
        const int knobHeight = static_cast<int>(juce::jmin(cellH - 16.0f, twoRows ? 132.0f : 184.0f));
        item.label->setBounds(static_cast<int>(x), static_cast<int>(y), static_cast<int>(cellW), 17);
        item.slider->setBounds(static_cast<int>(x + 2.0f), static_cast<int>(y + 15.0f),
                               static_cast<int>(cellW - 4.0f), knobHeight);
    }

    const int footerY = h - (small ? 88 : 94);
    const int footerX = margin + 14;
    statusLabel_.setBounds(footerX, footerY + 8, small ? 160 : 238, 16);
    mixLabel_.setBounds(w - (small ? 205 : 236), footerY + 5, 46, 14);
    mix_.setBounds(w - (small ? 214 : 246), footerY + 16, small ? 82 : 100, small ? 50 : 58);
    outputLabel_.setBounds(w - (small ? 112 : 124), footerY + 5, 60, 14);
    output_.setBounds(w - (small ? 128 : 142), footerY + 16, small ? 82 : 100, small ? 50 : 58);
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
    setParameter("drive", vary(driveBase[style], 10.0f, 0.0f, 100.0f));
    setParameter("crush", vary(crushBase[style], style == 5 ? 18.0f : 9.0f, 0.0f, 100.0f));
    setParameter("width", vary(style == 0 ? 155.0f : 100.0f, 18.0f, 0.0f, 200.0f));
    setParameter("reverb", vary(style == 0 ? 48.0f : 5.0f, 18.0f, 0.0f, 100.0f));
    setParameter("delay", vary(style == 0 ? 22.0f : 8.0f, 16.0f, 0.0f, 100.0f));
    setParameter("delayTime", vary(330.0f, 150.0f, 60.0f, 800.0f));
    setParameter("feedback", vary(22.0f, 12.0f, 0.0f, 75.0f));
    setParameter("mix", 100.0f);
    setStatus("Random ad-lib variation created.");
    abReady_ = false;
    abHasB_ = false;
}

void GumzyyCallFXAudioProcessorEditor::resetCurrentStyle()
{
    const auto* raw = processor_.parameters.getRawParameterValue("style");
    const int style = raw != nullptr ? juce::jlimit(0, 5, static_cast<int>(std::round(raw->load()))) : 0;
    applyPreset(style);
    abReady_ = false;
    abHasB_ = false;
    setStatus("Style reset to starting settings.");
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
        showingA_ = false;
        setStatus("A saved. Tweak knobs, then compare.");
        return;
    }

    if (!abHasB_)
    {
        captureSnapshot(snapshotB_);
        abHasB_ = true;
        restoreSnapshot(snapshotA_);
        showingA_ = true;
        setStatus("Playing A.");
        return;
    }

    if (showingA_)
    {
        captureSnapshot(snapshotA_);
        restoreSnapshot(snapshotB_);
        showingA_ = false;
        setStatus("Playing B.");
    }
    else
    {
        captureSnapshot(snapshotB_);
        restoreSnapshot(snapshotA_);
        showingA_ = true;
        setStatus("Playing A.");
    }
}

void GumzyyCallFXAudioProcessorEditor::setStatus(const juce::String& text)
{
    statusLabel_.setText(text, juce::dontSendNotification);
}
