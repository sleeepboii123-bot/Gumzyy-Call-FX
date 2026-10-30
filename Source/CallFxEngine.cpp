#include "CallFxEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace gumzyy
{
namespace
{
constexpr float pi = 3.14159265358979323846f;
constexpr float epsilon = 1.0e-6f;
}

void CallFxEngine::Smoother::setTarget(float value, int samples) noexcept
{
    if (!std::isfinite(value)) return;
    if (std::abs(value - target) < 1.0e-6f) return;
    target = value;
    remaining = std::max(1, samples);
    step = (target - current) / static_cast<float>(remaining);
}

float CallFxEngine::Smoother::next() noexcept
{
    if (remaining > 0)
    {
        current += step;
        if (--remaining == 0) current = target;
    }
    return current;
}

void CallFxEngine::Comb::prepare(int samples)
{
    data.assign(static_cast<std::size_t>(std::max(2, samples)), 0.0f);
    index = 0;
}
void CallFxEngine::Comb::reset() noexcept
{
    std::fill(data.begin(), data.end(), 0.0f);
    index = 0;
}
float CallFxEngine::Comb::process(float input, float feedback) noexcept
{
    if (data.empty()) return input;
    const float delayed = data[static_cast<std::size_t>(index)];
    data[static_cast<std::size_t>(index)] = input + delayed * feedback;
    if (++index >= static_cast<int>(data.size())) index = 0;
    return delayed;
}

void CallFxEngine::Allpass::prepare(int samples)
{
    data.assign(static_cast<std::size_t>(std::max(2, samples)), 0.0f);
    index = 0;
}
void CallFxEngine::Allpass::reset() noexcept
{
    std::fill(data.begin(), data.end(), 0.0f);
    index = 0;
}
float CallFxEngine::Allpass::process(float input, float coefficient) noexcept
{
    if (data.empty()) return input;
    const float delayed = data[static_cast<std::size_t>(index)];
    const float output = delayed - input;
    data[static_cast<std::size_t>(index)] = input + delayed * coefficient;
    if (++index >= static_cast<int>(data.size())) index = 0;
    return output;
}

float CallFxEngine::fastTanh(float x) noexcept
{
    x = std::max(-3.0f, std::min(3.0f, x));
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

float CallFxEngine::onePoleAlpha(float cutoffHz, float sampleRate) noexcept
{
    const float fc = std::max(5.0f, std::min(cutoffHz, sampleRate * 0.45f));
    const float w = 2.0f * pi * fc;
    return w / (w + sampleRate);
}

float CallFxEngine::clampFinite(float value, float low, float high, float fallback) noexcept
{
    return std::isfinite(value) ? std::max(low, std::min(high, value)) : fallback;
}

void CallFxEngine::prepare(double sampleRate, int maximumBlockSize, int channels)
{
    sampleRate_ = std::max(8000.0, sampleRate);
    maximumBlockSize_ = std::max(1, maximumBlockSize);
    channels_ = std::max(1, std::min(2, channels));

    const int maxDelaySamples = static_cast<int>(sampleRate_ * 1.5) + maximumBlockSize_ + 8;
    for (int ch = 0; ch < 2; ++ch)
    {
        delayBuffer_[static_cast<std::size_t>(ch)].assign(static_cast<std::size_t>(maxDelaySamples), 0.0f);
        delayWrite_[static_cast<std::size_t>(ch)] = 0;
        const float combMs[4] = { 29.7f, 37.1f, 41.1f, 43.7f };
        for (int i = 0; i < 4; ++i)
            combs_[static_cast<std::size_t>(ch)][static_cast<std::size_t>(i)].prepare(
                static_cast<int>(sampleRate_ * combMs[i] * 0.001));
        allpasses_[static_cast<std::size_t>(ch)][0].prepare(static_cast<int>(sampleRate_ * 0.0051));
        allpasses_[static_cast<std::size_t>(ch)][1].prepare(static_cast<int>(sampleRate_ * 0.0017));
    }
    reset();
}

void CallFxEngine::reset()
{
    for (auto& channel : hpState_) channel.fill(0.0f);
    for (auto& channel : lpState_) channel.fill(0.0f);
    airLowState_.fill(0.0f);
    crushCounter_.fill(0);
    crushedHold_.fill(0.0f);
    for (auto& buffer : delayBuffer_) std::fill(buffer.begin(), buffer.end(), 0.0f);
    delayWrite_.fill(0);
    for (auto& channel : combs_) for (auto& comb : channel) comb.reset();
    for (auto& channel : allpasses_) for (auto& allpass : channel) allpass.reset();

    hp_.reset(80.0f); lp_.reset(18000.0f); air_.reset(3.5f); drive_.reset(10.0f);
    crush_.reset(0.0f); width_.reset(140.0f); reverb_.reset(30.0f); delay_.reset(18.0f);
    delayTime_.reset(280.0f); feedback_.reset(24.0f); mix_.reset(100.0f); outputGain_.reset(1.0f);
    inputPeak_ = outputPeak_ = 0.0f;
}

void CallFxEngine::process(float* const* channelData, int channels, int numSamples,
                           const CallFxParameters& p) noexcept
{
    if (channelData == nullptr || numSamples <= 0) return;
    const int activeChannels = std::max(1, std::min({ channels, channels_, 2 }));
    const int rampSamples = std::max(1, static_cast<int>(sampleRate_ * 0.020));

    hp_.setTarget(clampFinite(p.highPassHz, 20.0f, 1200.0f, 80.0f), rampSamples);
    lp_.setTarget(clampFinite(p.lowPassHz, 1000.0f, 20000.0f, 18000.0f), rampSamples);
    air_.setTarget(clampFinite(p.airDb, 0.0f, 12.0f, 0.0f), rampSamples);
    drive_.setTarget(clampFinite(p.drive, 0.0f, 100.0f, 0.0f), rampSamples);
    crush_.setTarget(clampFinite(p.crush, 0.0f, 100.0f, 0.0f), rampSamples);
    width_.setTarget(clampFinite(p.width, 0.0f, 200.0f, 100.0f), rampSamples);
    reverb_.setTarget(clampFinite(p.reverb, 0.0f, 100.0f, 0.0f), rampSamples);
    delay_.setTarget(clampFinite(p.delay, 0.0f, 100.0f, 0.0f), rampSamples);
    delayTime_.setTarget(clampFinite(p.delayTimeMs, 60.0f, 800.0f, 280.0f), rampSamples);
    feedback_.setTarget(clampFinite(p.feedback, 0.0f, 75.0f, 20.0f), rampSamples);
    mix_.setTarget(clampFinite(p.mix, 0.0f, 100.0f, 100.0f) * 0.01f, rampSamples);
    outputGain_.setTarget(std::pow(10.0f, clampFinite(p.outputDb, -18.0f, 12.0f, 0.0f) / 20.0f), rampSamples);

    float inPeak = 0.0f, outPeak = 0.0f;
    const int style = std::max(0, std::min(5, p.style));
    const float styleDrive[6] = { 0.72f, 1.00f, 1.12f, 1.28f, 1.10f, 1.24f };
    const float styleCrush[6] = { 0.0f, 0.0f, 5.0f, 12.0f, 2.0f, 18.0f };

    for (int n = 0; n < numSamples; ++n)
    {
        const float hpCut = hp_.next();
        const float lpCut = std::max(lp_.next(), hpCut * 1.35f);
        const float airDb = air_.next();
        const float driveAmount = drive_.next();
        const float crushAmount = std::min(100.0f, crush_.next() + styleCrush[style]);
        const float widthAmount = width_.next() * 0.01f;
        const float reverbAmount = reverb_.next() * 0.01f;
        const float delayAmount = delay_.next() * 0.01f;
        const float delayMs = delayTime_.next();
        const float feedbackAmount = feedback_.next() * 0.01f;
        const float wet = mix_.next();
        const float outputGain = outputGain_.next();

        const float hpAlpha = onePoleAlpha(hpCut, static_cast<float>(sampleRate_));
        const float lpAlpha = onePoleAlpha(lpCut, static_cast<float>(sampleRate_));
        const float airAlpha = onePoleAlpha(6200.0f, static_cast<float>(sampleRate_));
        const float airGain = (std::pow(10.0f, airDb / 20.0f) - 1.0f) * 0.62f;
        const float driveGain = 1.0f + driveAmount * 0.035f * styleDrive[style];
        const int bitDepth = std::max(4, 16 - static_cast<int>(std::round(crushAmount * 0.12f)));
        const float quantLevels = static_cast<float>((1u << bitDepth) - 1u);
        const int holdPeriod = 1 + static_cast<int>(std::round(crushAmount * 0.11f));
        const int delaySize = static_cast<int>(delayBuffer_[0].size());
        const int delaySamples = std::max(1, std::min(delaySize - 1,
            static_cast<int>(sampleRate_ * delayMs * 0.001f)));
        const float reverbFeedback = 0.69f + reverbAmount * 0.055f;

        float sample[2] = { 0.0f, 0.0f };
        float drySample[2] = { 0.0f, 0.0f };
        for (int ch = 0; ch < activeChannels; ++ch)
        {
            const float dry = channelData[ch][n];
            drySample[ch] = dry;
            inPeak = std::max(inPeak, std::abs(dry));
            float x = dry;

            // Two one-pole sections per side give steeper, stable analogue-style filters.
            auto& hs = hpState_[static_cast<std::size_t>(ch)];
            for (int stage = 0; stage < 2; ++stage)
            {
                hs[static_cast<std::size_t>(stage)] += hpAlpha * (x - hs[static_cast<std::size_t>(stage)]);
                x -= hs[static_cast<std::size_t>(stage)];
            }
            auto& ls = lpState_[static_cast<std::size_t>(ch)];
            for (int stage = 0; stage < 2; ++stage)
            {
                ls[static_cast<std::size_t>(stage)] += lpAlpha * (x - ls[static_cast<std::size_t>(stage)]);
                x = ls[static_cast<std::size_t>(stage)];
            }

            // Air shelf made from a zero-latency high-frequency residual.
            auto& airLow = airLowState_[static_cast<std::size_t>(ch)];
            airLow += airAlpha * (x - airLow);
            const float airHigh = x - airLow;
            x += airHigh * airGain;

            // Lightweight waveshaping: no look-ahead, oversampling or extra host latency.
            const float saturated = fastTanh(x * driveGain);
            const float driveBlend = std::min(1.0f, driveAmount * 0.01f * styleDrive[style]);
            x += (saturated - x) * driveBlend;

            // Sample-rate/bit-depth reduction is optional and only active when Crush is raised.
            if (crushAmount > 0.5f)
            {
                auto& counter = crushCounter_[static_cast<std::size_t>(ch)];
                auto& held = crushedHold_[static_cast<std::size_t>(ch)];
                if (counter == 0)
                {
                    held = std::round(x * quantLevels) / quantLevels;
                }
                if (++counter >= holdPeriod) counter = 0;
                x = held;
            }

            // Delay is an auxiliary wet path; the original signal is never delayed.
            auto& delayData = delayBuffer_[static_cast<std::size_t>(ch)];
            int readIndex = delayWrite_[static_cast<std::size_t>(ch)] - delaySamples;
            if (readIndex < 0) readIndex += delaySize;
            const float delayed = delayData[static_cast<std::size_t>(readIndex)];
            delayData[static_cast<std::size_t>(delayWrite_[static_cast<std::size_t>(ch)])] = x + delayed * feedbackAmount * 0.88f;
            if (++delayWrite_[static_cast<std::size_t>(ch)] >= delaySize) delayWrite_[static_cast<std::size_t>(ch)] = 0;
            x += delayed * delayAmount * 0.72f;

            // Four parallel combs plus two all-pass diffusers, all running sample-by-sample.
            float reverbSignal = 0.0f;
            auto& channelCombs = combs_[static_cast<std::size_t>(ch)];
            for (auto& comb : channelCombs) reverbSignal += comb.process(x, reverbFeedback);
            reverbSignal *= 0.25f;
            auto& channelAllpasses = allpasses_[static_cast<std::size_t>(ch)];
            reverbSignal = channelAllpasses[0].process(reverbSignal, 0.58f);
            reverbSignal = channelAllpasses[1].process(reverbSignal, 0.52f);
            x += reverbSignal * reverbAmount * 0.65f;
            sample[ch] = x;
        }

        if (activeChannels == 2)
        {
            const float mid = 0.5f * (sample[0] + sample[1]);
            const float side = 0.5f * (sample[0] - sample[1]) * widthAmount;
            sample[0] = mid + side;
            sample[1] = mid - side;
        }

        for (int ch = 0; ch < activeChannels; ++ch)
        {
            const float y = (drySample[ch] * (1.0f - wet) + sample[ch] * wet) * outputGain;
            channelData[ch][n] = y;
            outPeak = std::max(outPeak, std::abs(y));
        }
    }
    inputPeak_ = inPeak;
    outputPeak_ = outPeak;
}
} // namespace gumzyy
