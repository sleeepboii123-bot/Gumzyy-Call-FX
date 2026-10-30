#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>

namespace
{
const juce::Colour bg { 0xff09080d };
const juce::Colour panel { 0xff15121b };
const juce::Colour raised { 0xff211a2a };
const juce::Colour edge { 0xff48365b };
const juce::Colour violet { 0xffa65cff };
const juce::Colour bright { 0xffd8b7ff };
const juce::Colour white { 0xfff2ecf8 };
const juce::Colour muted { 0xffa99bb8 };
}

void GumzyyLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    const float d = static_cast<float>(juce::jmin(width, height)) - 18.0f;
    const float r = d * 0.5f;
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
    juce::Path arrow;
    const float cx = width - 18.0f, cy = height * 0.5f;
    arrow.startNewSubPath(cx - 4.0f, cy - 2.0f);
    arrow.lineTo(cx, cy + 3.0f);
    arrow.lineTo(cx + 4.0f, cy - 2.0f);
    g.setColour(bright);
    g.strokePath(arrow, juce::PathStrokeType(1.8f));
}

void GumzyyLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
    const juce::Colour&, bool highlighted, bool down)
{
    auto b = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool active = button.getToggleState();
    g.setColour(active ? juce::Colour(0xff47245f) : (down || highlighted ? juce::Colour(0xff352640) : raised));
    g.fillRoundedRectangle(b, 8.0f);
    g.setColour(active ? bright : edge);
    g.drawRoundedRectangle(b, 8.0f, 1.0f);
}

GumzyyCallFXAudioProcessorEditor::GumzyyCallFXAudioProcessorEditor(GumzyyCallFXAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p),
      mascot_(juce::ImageCache::getFromMemory(BinaryData::gumzyy_png, BinaryData::gumzyy_pngSize))
{
    setOpaque(true);
    setResizable(true, true);
    setResizeLimits(800, 560, 1420, 930);
    getConstrainer()->setFixedAspectRatio(1160.0 / 760.0);
    setSize(1160, 760);

    brand_.setText("OFF SESSION  ✦  GUMZYY CALL FX", juce::dontSendNotification);
    brand_.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold | juce::Font::italic)));
    brand_.setColour(juce::Label::textColourId, white);
    brand_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(brand_);

    subtitle_.setText("AD-LIB COLOR LAB  /  NYXV × OFF SESSION", juce::dontSendNotification);
    subtitle_.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    subtitle_.setColour(juce::Label::textColourId, bright);
    subtitle_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subtitle_);

    modeLabel_.setText("CALL STYLE", juce::dontSendNotification);
    modeLabel_.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    modeLabel_.setColour(juce::Label::textColourId, muted);
    addAndMakeVisible(modeLabel_);

    style_.addItemList({ "ANGEL", "PHONE", "RADIO", "WALKIE", "MEGAPHONE", "BROKEN" }, 1);
    style_.setLookAndFeel(&lookAndFeel_);
    style_.setColour(juce::ComboBox::textColourId, white);
    style_.setColour(juce::ComboBox::outlineColourId, edge);
    addAndMakeVisible(style_);
    styleAttachment_ = std::make_unique<ComboAttachment>(processor_.parameters, "style", style_);
    style_.onChange = [this] { applyPreset(style_.getSelectedItemIndex()); };

    sizeButton_.setLookAndFeel(&lookAndFeel_);
    sizeButton_.setColour(juce::TextButton::textColourOffId, white);
    sizeButton_.onClick = [this]
    {
        compact_ = !compact_;
        sizeButton_.setButtonText(compact_ ? "BIG" : "SMALL");
        getConstrainer()->setFixedAspectRatio(compact_ ? (820.0 / 580.0) : (1160.0 / 760.0));
        setSize(compact_ ? 820 : 1160, compact_ ? 580 : 760);
    };
    addAndMakeVisible(sizeButton_);

    bypassButton_.setLookAndFeel(&lookAndFeel_);
    bypassButton_.setClickingTogglesState(true);
    bypassButton_.setColour(juce::TextButton::textColourOffId, white);
    bypassButton_.setColour(juce::TextButton::textColourOnId, bright);
    addAndMakeVisible(bypassButton_);
    bypassAttachment_ = std::make_unique<ButtonAttachment>(processor_.parameters, "bypass", bypassButton_);

    addKnob(hp_, hpLabel_, "HIGH PASS", " Hz");
    addKnob(lp_, lpLabel_, "LOW PASS", " Hz");
    addKnob(air_, airLabel_, "AIR", " dB");
    addKnob(width_, widthLabel_, "WIDTH", "%");
    addKnob(drive_, driveLabel_, "DRIVE", "%");
    addKnob(crush_, crushLabel_, "CRUSH", "%");
    addKnob(reverb_, reverbLabel_, "REVERB", "%");
    addKnob(delay_, delayLabel_, "DELAY", "%");
    addKnob(delayTime_, delayTimeLabel_, "TIME", " ms");
    addKnob(feedback_, feedbackLabel_, "FEEDBACK", "%");
    addKnob(mix_, mixLabel_, "MIX", "%");
    addKnob(output_, outputLabel_, "OUTPUT", " dB");

    hp_.setRange(20.0, 1200.0, 1.0); lp_.setRange(1000.0, 20000.0, 1.0);
    air_.setRange(0.0, 12.0, 0.01); width_.setRange(0.0, 200.0, 0.1);
    drive_.setRange(0.0, 100.0, 0.1); crush_.setRange(0.0, 100.0, 0.1);
    reverb_.setRange(0.0, 100.0, 0.1); delay_.setRange(0.0, 100.0, 0.1);
    delayTime_.setRange(60.0, 800.0, 1.0); feedback_.setRange(0.0, 75.0, 0.1);
    mix_.setRange(0.0, 100.0, 0.1); output_.setRange(-18.0, 12.0, 0.01);

    auto& apvts = processor_.parameters;
    hpAttachment_ = std::make_unique<SliderAttachment>(apvts, "hp", hp_);
    lpAttachment_ = std::make_unique<SliderAttachment>(apvts, "lp", lp_);
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

    statusLabel_.setText("ZERO-LOOKAHEAD SIGNAL PATH  ·  0 SAMPLES PLUGIN LATENCY", juce::dontSendNotification);
    statusLabel_.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    statusLabel_.setColour(juce::Label::textColourId, bright);
    statusLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel_);
}

GumzyyCallFXAudioProcessorEditor::~GumzyyCallFXAudioProcessorEditor()
{
    clearLookAndFeel();
}

void GumzyyCallFXAudioProcessorEditor::clearLookAndFeel()
{
    style_.setLookAndFeel(nullptr);
    sizeButton_.setLookAndFeel(nullptr);
    bypassButton_.setLookAndFeel(nullptr);
    juce::Slider* sliders[] = { &hp_, &lp_, &air_, &width_, &drive_, &crush_, &reverb_, &delay_,
                                &delayTime_, &feedback_, &mix_, &output_ };
    for (auto* slider : sliders) slider->setLookAndFeel(nullptr);
}

void GumzyyCallFXAudioProcessorEditor::addKnob(juce::Slider& slider, juce::Label& label,
    const juce::String& title, const juce::String& suffix)
{
    label.setText(title, juce::dontSendNotification);
    label.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, muted);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);

    slider.setLookAndFeel(&lookAndFeel_);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::textBoxTextColourId, white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, raised);
    slider.setColour(juce::Slider::textBoxOutlineColourId, edge);
    addAndMakeVisible(slider);
}

void GumzyyCallFXAudioProcessorEditor::setParameter(const juce::String& id, float value)
{
    if (auto* parameter = processor_.parameters.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

void GumzyyCallFXAudioProcessorEditor::applyPreset(int index)
{
    // Presets are real APVTS parameter changes, so every knob remains automatable in the DAW.
    struct Preset { float hp, lp, air, drive, crush, width, reverb, delay, time, feedback, mix; };
    static const Preset presets[] = {
        {  80, 18000, 8.0f,  6,  0, 155, 55, 25, 420, 28, 100 }, // Angel
        { 300,  3400, 0.0f, 18,  0, 100,  0,  0, 260, 18, 100 }, // Phone
        { 180,  6000, 0.0f, 32, 12, 100,  5,  0, 240, 18, 100 }, // Radio
        { 500,  2900, 0.0f, 48, 36, 100,  0,  0, 180, 15, 100 }, // Walkie
        { 230,  4300, 0.0f, 28, 10, 100,  3,  0, 260, 18, 100 }, // Megaphone
        { 120,  6800, 1.0f, 55, 72, 115, 15, 35, 180, 34, 100 }  // Broken
    };
    index = juce::jlimit(0, 5, index);
    const auto& v = presets[index];
    setParameter("hp", v.hp); setParameter("lp", v.lp); setParameter("air", v.air);
    setParameter("drive", v.drive); setParameter("crush", v.crush); setParameter("width", v.width);
    setParameter("reverb", v.reverb); setParameter("delay", v.delay); setParameter("delayTime", v.time);
    setParameter("feedback", v.feedback); setParameter("mix", v.mix);
}

void GumzyyCallFXAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> b,
    const juce::String& title, const juce::String& accent)
{
    g.setColour(panel);
    g.fillRoundedRectangle(b, 12.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(b, 12.0f, 1.0f);
    g.setColour(violet.withAlpha(0.22f));
    g.fillRoundedRectangle(b.removeFromTop(3.0f), 2.0f);
    g.setColour(bright);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText(title, b.getX() + 12.0f, b.getY() + 7.0f, b.getWidth() - 24.0f, 18.0f,
               juce::Justification::centredLeft);
    g.setColour(violet.withAlpha(0.6f));
    g.drawText(accent, b.getX() + 12.0f, b.getY() + 25.0f, b.getWidth() - 24.0f, 12.0f,
               juce::Justification::centredLeft);
}

void GumzyyCallFXAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    auto full = getLocalBounds().toFloat().reduced(8.0f);
    g.setColour(juce::Colour(0xff20182a));
    g.drawRoundedRectangle(full, 16.0f, 1.5f);

    // Phone-call display panel and neon waveform.
    const float w = static_cast<float>(getWidth());
    const float displayW = compact_ ? 235.0f : 370.0f;
    const float displayX = (w - displayW) * 0.5f;
    auto display = juce::Rectangle<float>(displayX, 18.0f, displayW, compact_ ? 72.0f : 92.0f);
    g.setColour(juce::Colour(0xff050409));
    g.fillRoundedRectangle(display, 12.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(display, 12.0f, 1.0f);
    g.setColour(bright);
    g.setFont(juce::Font(juce::FontOptions(compact_ ? 9.0f : 10.0f, juce::Font::bold)));
    g.drawText("GUMZYY IS CALLING...", display.withY(display.getY() + 9.0f).withHeight(15.0f), juce::Justification::centred);

    const float waveY = display.getCentreY() + 11.0f;
    const int bars = compact_ ? 31 : 43;
    for (int i = 0; i < bars; ++i)
    {
        const float waveX = display.getX() + 16.0f + (display.getWidth() - 32.0f) * i / static_cast<float>(bars);
        const float phase = static_cast<float>(i) * 0.62f;
        const float envelope = 0.25f + 0.75f * std::abs(std::sin(phase * 0.37f) * std::cos(phase * 0.19f));
        const float barH = (compact_ ? 13.0f : 19.0f) * envelope;
        g.setColour(violet.withAlpha(0.65f + 0.3f * envelope));
        g.drawLine(waveX, waveY - barH, waveX, waveY + barH, compact_ ? 1.2f : 1.7f);
    }

    if (!mascot_.isNull())
    {
        auto mascotArea = juce::Rectangle<float>(w - (compact_ ? 116.0f : 170.0f), 12.0f,
                                                  compact_ ? 92.0f : 132.0f, compact_ ? 105.0f : 135.0f);
        g.drawImageWithin(mascot_, static_cast<int>(mascotArea.getX()), static_cast<int>(mascotArea.getY()),
                          static_cast<int>(mascotArea.getWidth()), static_cast<int>(mascotArea.getHeight()),
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }

    const float panelY = compact_ ? 146.0f : 160.0f;
    const float panelH = static_cast<float>(getHeight()) - panelY - (compact_ ? 94.0f : 112.0f);
    const float gap = compact_ ? 9.0f : 13.0f;
    const float margin = compact_ ? 15.0f : 22.0f;
    const float panelW = (w - margin * 2.0f - gap * 3.0f) / 4.0f;
    const char* names[] = { "01  FILTER", "02  ANGEL", "03  COLOR", "04  SPACE" };
    const char* details[] = { "SHAPE THE SIGNAL", "AIR & STEREO", "ANALOG / DIGITAL", "DEPTH & ECHO" };
    for (int i = 0; i < 4; ++i)
    {
        auto b = juce::Rectangle<float>(margin + i * (panelW + gap), panelY, panelW, panelH);
        drawPanel(g, b, names[i], details[i]);
    }

    auto footer = juce::Rectangle<float>(margin, static_cast<float>(getHeight()) - (compact_ ? 82.0f : 96.0f),
                                         w - margin * 2.0f, compact_ ? 64.0f : 76.0f);
    g.setColour(panel);
    g.fillRoundedRectangle(footer, 10.0f);
    g.setColour(edge);
    g.drawRoundedRectangle(footer, 10.0f, 1.0f);
}

void GumzyyCallFXAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const bool small = compact_;
    brand_.setBounds(22, 22, small ? 255 : 345, 34);
    subtitle_.setBounds(24, 57, small ? 250 : 330, 20);
    modeLabel_.setBounds(24, small ? 91 : 108, 90, 15);
    style_.setBounds(24, small ? 108 : 126, small ? 178 : 210, 30);
    sizeButton_.setBounds(w - (small ? 96 : 112), 18, small ? 72 : 86, 28);
    bypassButton_.setBounds(w - (small ? 112 : 132), 54, small ? 88 : 104, 27);

    const int margin = small ? 15 : 22;
    const int gap = small ? 9 : 13;
    const float panelW = static_cast<float>(w - margin * 2 - gap * 3) / 4.0f;
    const float panelY = small ? 146.0f : 160.0f;
    const float panelH = static_cast<float>(getHeight()) - panelY - (small ? 94.0f : 112.0f);

    struct Item { juce::Slider* slider; juce::Label* label; int panel; int slot; };
    const Item items[] = {
        { &hp_, &hpLabel_, 0, 0 }, { &lp_, &lpLabel_, 0, 1 },
        { &air_, &airLabel_, 1, 0 }, { &width_, &widthLabel_, 1, 1 },
        { &drive_, &driveLabel_, 2, 0 }, { &crush_, &crushLabel_, 2, 1 },
        { &reverb_, &reverbLabel_, 3, 0 }, { &delay_, &delayLabel_, 3, 1 },
        { &delayTime_, &delayTimeLabel_, 3, 2 }, { &feedback_, &feedbackLabel_, 3, 3 }
    };

    for (const auto& item : items)
    {
        const float panelX = static_cast<float>(margin) + item.panel * (panelW + gap);
        const int top = static_cast<int>(panelY + (small ? 48.0f : 55.0f));
        const int bottom = static_cast<int>(panelY + panelH - 6.0f);
        const float innerW = panelW - 12.0f;
        const float knobW = item.panel == 3 ? innerW * 0.5f : innerW * 0.5f;
        const float knobH = item.panel == 3 ? (bottom - top - 2.0f) * 0.5f : (bottom - top);
        int col = (item.panel == 3 ? (item.slot % 2) : item.slot);
        int row = (item.panel == 3 ? (item.slot / 2) : 0);
        const float x = panelX + 6.0f + col * knobW;
        const float y = static_cast<float>(top) + row * knobH;
        item.label->setBounds(static_cast<int>(x), static_cast<int>(y), static_cast<int>(knobW), 15);
        item.slider->setBounds(static_cast<int>(x + 2.0f), static_cast<int>(y + 13.0f),
                               static_cast<int>(knobW - 4.0f), static_cast<int>(knobH - 12.0f));
    }

    const int footerY = getHeight() - (small ? 82 : 96);
    statusLabel_.setBounds(margin + 12, footerY + 5, static_cast<int>((w - margin * 2) * 0.50f), 16);
    const int footerX = margin;
    mixLabel_.setBounds(footerX + static_cast<int>((w - margin * 2) * 0.57f), footerY + 3, 46, 14);
    mix_.setBounds(footerX + static_cast<int>((w - margin * 2) * 0.57f), footerY + 15, 94, small ? 42 : 54);
    outputLabel_.setBounds(footerX + static_cast<int>((w - margin * 2) * 0.77f), footerY + 3, 60, 14);
    output_.setBounds(footerX + static_cast<int>((w - margin * 2) * 0.77f), footerY + 15, 100, small ? 42 : 54);
}
