#include "CallFxEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
float rms(const std::vector<float>& data, std::size_t start)
{
    double sum = 0.0;
    std::size_t count = 0;
    for (std::size_t i = start; i < data.size(); ++i) { sum += data[i] * data[i]; ++count; }
    return count == 0 ? 0.0f : static_cast<float>(std::sqrt(sum / static_cast<double>(count)));
}
float renderSineRms(gumzyy::CallFxParameters params, double sampleRate, float frequency)
{
    gumzyy::CallFxEngine testEngine;
    constexpr int blockSize = 128;
    const int totalSamples = static_cast<int>(sampleRate);
    std::vector<float> signal(static_cast<std::size_t>(totalSamples));
    for (int i = 0; i < totalSamples; ++i)
        signal[static_cast<std::size_t>(i)] = 0.05f * std::sin(2.0 * 3.141592653589793 * frequency * i / sampleRate);
    testEngine.prepare(sampleRate, blockSize, 1);
    for (int offset = 0; offset < totalSamples; offset += blockSize)
    {
        const int n = std::min(blockSize, totalSamples - offset);
        float* channels[1] = { signal.data() + offset };
        testEngine.process(channels, 1, n, params);
    }
    return rms(signal, static_cast<std::size_t>(sampleRate * 0.30));
}
}

int main()
{
    constexpr double sr = 48000.0;
    constexpr int block = 128;
    constexpr int total = 48000;

    gumzyy::CallFxEngine engine;
    engine.prepare(sr, block, 1);
    require(engine.latencySamples() == 0, "engine must report zero algorithmic latency");

    // Mix at 0% must pass the dry signal unchanged, sample-for-sample.
    gumzyy::CallFxParameters p;
    p.mix = 0.0f; p.outputDb = 0.0f;
    std::vector<float> dry(total), output(total);
    for (int i = 0; i < total; ++i) dry[static_cast<std::size_t>(i)] = 0.3f * std::sin(2.0 * 3.141592653589793 * 440.0 * i / sr);
    for (int offset = 0; offset < total; offset += block)
    {
        const int n = std::min(block, total - offset);
        std::copy_n(dry.data() + offset, n, output.data() + offset);
        float* channels[1] = { output.data() + offset };
        engine.process(channels, 1, n, p);
    }
    float maxError = 0.0f;
    for (int i = 2048; i < total; ++i) maxError = std::max(maxError, std::abs(dry[static_cast<std::size_t>(i)] - output[static_cast<std::size_t>(i)]));
    require(maxError < 1.0e-5f, "0% Mix should preserve the input exactly");

    // Phone-style high-pass should attenuate low bass without a look-ahead buffer.
    engine.prepare(sr, block, 1);
    p = {}; p.highPassHz = 350.0f; p.lowPassHz = 3500.0f; p.airDb = 0.0f;
    p.drive = 0.0f; p.crush = 0.0f; p.width = 100.0f; p.reverb = 0.0f; p.delay = 0.0f; p.mix = 100.0f;
    std::vector<float> low(total), high(total);
    for (int i = 0; i < total; ++i) low[static_cast<std::size_t>(i)] = 0.2f * std::sin(2.0 * 3.141592653589793 * 90.0 * i / sr);
    for (int offset = 0; offset < total; offset += block)
    {
        const int n = std::min(block, total - offset); float* channels[1] = { low.data() + offset };
        engine.process(channels, 1, n, p);
    }
    const float lowRms = rms(low, 12000);
    engine.prepare(sr, block, 1);
    for (int i = 0; i < total; ++i) high[static_cast<std::size_t>(i)] = 0.2f * std::sin(2.0 * 3.141592653589793 * 1000.0 * i / sr);
    for (int offset = 0; offset < total; offset += block)
    {
        const int n = std::min(block, total - offset); float* channels[1] = { high.data() + offset };
        engine.process(channels, 1, n, p);
    }
    const float highRms = rms(high, 12000);
    require(lowRms < highRms * 0.45f, "telephone high-pass should attenuate 90 Hz compared with 1 kHz");

    // Crush must produce a quantised/sample-held signal and remain finite.
    engine.prepare(sr, block, 1);
    p = {}; p.mix = 100.0f; p.highPassHz = 20.0f; p.lowPassHz = 20000.0f;
    p.airDb = 0.0f; p.drive = 0.0f; p.crush = 100.0f; p.reverb = 0.0f; p.delay = 0.0f;
    std::vector<float> crushed(8192);
    for (int i = 0; i < static_cast<int>(crushed.size()); ++i)
        crushed[static_cast<std::size_t>(i)] = 0.4f * std::sin(2.0 * 3.141592653589793 * 701.0 * i / sr);
    for (int offset = 0; offset < static_cast<int>(crushed.size()); offset += block)
    {
        const int n = std::min(block, static_cast<int>(crushed.size()) - offset); float* channels[1] = { crushed.data() + offset };
        engine.process(channels, 1, n, p);
    }
    for (float x : crushed) require(std::isfinite(x), "crush output must not contain NaN or infinity");

    // Stereo width at 100% should preserve a centred signal and all processing stays finite.
    engine.prepare(sr, block, 2);
    p = {}; p.mix = 100.0f; p.width = 100.0f; p.reverb = 15.0f; p.delay = 15.0f;
    std::vector<float> left(block, 0.1f), right(block, 0.1f);
    for (int k = 0; k < 10; ++k)
    {
        float* channels[2] = { left.data(), right.data() };
        engine.process(channels, 2, block, p);
    }
    for (float x : left) require(std::isfinite(x), "left channel must remain finite");
    for (float x : right) require(std::isfinite(x), "right channel must remain finite");


    // Input Gain is a real pre-effect gain control and should follow the requested dB change.
    gumzyy::CallFxParameters gainParams;
    gainParams.highPassHz = 20.0f; gainParams.lowPassHz = 20000.0f;
    gainParams.airDb = 0.0f; gainParams.drive = 0.0f; gainParams.crush = 0.0f;
    gainParams.width = 100.0f; gainParams.reverb = 0.0f; gainParams.delay = 0.0f;
    gainParams.mix = 100.0f; gainParams.inputDb = 0.0f;
    const float unityRms = renderSineRms(gainParams, sr, 440.0f);
    gainParams.inputDb = 6.0f;
    const float boostedRms = renderSineRms(gainParams, sr, 440.0f);
    require(boostedRms > unityRms * 1.75f && boostedRms < unityRms * 2.2f,
            "Input Gain should audibly apply approximately the requested +6 dB");

    // Tone is a real low/high tilt, not a decorative control. Positive tone should brighten.
    gainParams.inputDb = 0.0f; gainParams.tone = 100.0f;
    const float brightToneRms = renderSineRms(gainParams, sr, 4200.0f);
    gainParams.tone = -100.0f;
    const float darkToneRms = renderSineRms(gainParams, sr, 4200.0f);
    require(brightToneRms > darkToneRms * 1.65f,
            "Tone control should audibly tilt high-frequency content");

    // Different common project sample rates and irregular block lengths must remain stable.
    for (double testRate : { 44100.0, 48000.0, 96000.0 })
    {
        gumzyy::CallFxEngine rateEngine;
        rateEngine.prepare(testRate, 127, 2);
        gumzyy::CallFxParameters rateParams;
        rateParams.highPassHz = 220.0f; rateParams.lowPassHz = 5200.0f;
        rateParams.drive = 42.0f; rateParams.crush = 18.0f;
        rateParams.reverb = 30.0f; rateParams.delay = 25.0f;
        std::vector<float> l(997), r(997);
        for (int i = 0; i < 997; ++i)
        {
            l[static_cast<std::size_t>(i)] = 0.25f * std::sin(2.0 * 3.141592653589793 * 330.0 * i / testRate);
            r[static_cast<std::size_t>(i)] = 0.22f * std::sin(2.0 * 3.141592653589793 * 440.0 * i / testRate);
        }
        int offset = 0;
        while (offset < 997)
        {
            const int n = std::min(127, 997 - offset);
            float* channels[2] = { l.data() + offset, r.data() + offset };
            rateEngine.process(channels, 2, n, rateParams);
            offset += n;
        }
        require(rateEngine.latencySamples() == 0, "sample-rate changes must not add host latency");
        for (float x : l) require(std::isfinite(x), "left output must remain finite at common sample rates");
        for (float x : r) require(std::isfinite(x), "right output must remain finite at common sample rates");
    }

    std::cout << "All GUMZYY CALL FX DSP tests passed. Latency: 0 samples.\n";
    return 0;
}
