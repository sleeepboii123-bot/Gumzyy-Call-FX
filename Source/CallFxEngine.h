#pragma once

#include <array>
#include <vector>

namespace gumzyy
{
struct CallFxParameters
{
    int style = 0;             // 0 Angel, 1 Phone, 2 Radio, 3 Walkie, 4 Megaphone, 5 Broken
    float highPassHz = 80.0f;
    float inputDb = 0.0f;        // -18..+18 dB before tone and saturation
    float tone = 0.0f;          // -100..+100, broad low/high tilt
    float lowPassHz = 18000.0f;
    float airDb = 3.5f;
    float drive = 10.0f;        // 0..100
    float crush = 0.0f;         // 0..100
    float width = 140.0f;       // 0..200
    float reverb = 30.0f;       // 0..100
    float delay = 18.0f;        // 0..100
    float delayTimeMs = 280.0f; // 60..800
    float feedback = 24.0f;     // 0..75
    float mix = 100.0f;         // 0..100
    float outputDb = 0.0f;
};

class CallFxEngine final
{
public:
    void prepare(double sampleRate, int maximumBlockSize, int channels);
    void reset();
    void process(float* const* channelData, int channels, int numSamples,
                 const CallFxParameters& parameters) noexcept;

    int latencySamples() const noexcept { return 0; }
    float inputPeak() const noexcept { return inputPeak_; }
    float outputPeak() const noexcept { return outputPeak_; }

private:
    struct Smoother
    {
        float current = 0.0f, target = 0.0f, step = 0.0f;
        int remaining = 0;
        void reset(float value) noexcept { current = target = value; step = 0.0f; remaining = 0; }
        void setTarget(float value, int samples) noexcept;
        float next() noexcept;
    };

    struct Comb
    {
        std::vector<float> data;
        int index = 0;
        void prepare(int samples);
        void reset() noexcept;
        float process(float input, float feedback) noexcept;
    };

    struct Allpass
    {
        std::vector<float> data;
        int index = 0;
        void prepare(int samples);
        void reset() noexcept;
        float process(float input, float coefficient) noexcept;
    };

    static float fastTanh(float x) noexcept;
    static float onePoleAlpha(float cutoffHz, float sampleRate) noexcept;
    static float clampFinite(float value, float low, float high, float fallback) noexcept;

    double sampleRate_ = 48000.0;
    int maximumBlockSize_ = 512;
    int channels_ = 2;
    std::array<std::array<float, 2>, 2> hpState_ {};
    std::array<std::array<float, 2>, 2> lpState_ {};
    std::array<float, 2> airLowState_ {};
    std::array<float, 2> toneLowState_ {};
    std::array<int, 2> crushCounter_ {};
    std::array<float, 2> crushedHold_ {};

    std::array<std::vector<float>, 2> delayBuffer_;
    std::array<int, 2> delayWrite_ {};
    std::array<std::array<Comb, 4>, 2> combs_;
    std::array<std::array<Allpass, 2>, 2> allpasses_;

    Smoother hp_, lp_, inputGain_, tone_, air_, drive_, crush_, width_, reverb_, delay_, delayTime_, feedback_, mix_, outputGain_;
    float inputPeak_ = 0.0f, outputPeak_ = 0.0f;
};
} // namespace gumzyy
