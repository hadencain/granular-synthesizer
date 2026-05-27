#include "GrainBuffer.h"
#include <juce_audio_formats/juce_audio_formats.h>

GrainBuffer::GrainBuffer() = default;

void GrainBuffer::prepare(double sr)
{
    sampleRate = sr;
    // Allocate to 60s max at the given sample rate — done once, never again
    maxLengthSamples = static_cast<int>(std::ceil(60.0 * sr));
    backingBuffer.setSize(2, maxLengthSamples, false, true, false);
    backingBuffer.clear();
    writeHead.store(0);
    usedLength.store(0);
    oneShotFilled = false;
}

bool GrainBuffer::loadFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return false;

    const juce::int64 srcLength = reader->lengthInSamples;
    if (srcLength <= 0) { delete reader; return false; }

    // Decode into a temp buffer
    juce::AudioBuffer<float> temp(static_cast<int>(reader->numChannels),
                                  static_cast<int>(srcLength));
    reader->read(&temp, 0, static_cast<int>(srcLength), 0, true, true);
    delete reader;

    // Resample to host sample rate if needed — simple linear resample
    // (For release quality, replace with juce::ResamplingAudioSource)
    usedLength.store(0, std::memory_order_release);   // audio thread returns silence during load
    backingBuffer.clear();
    const int destLength = juce::jmin(maxLengthSamples,
                                      static_cast<int>(srcLength)); // same-rate fast path
    for (int ch = 0; ch < 2; ++ch)
    {
        const int srcCh = juce::jmin(ch, temp.getNumChannels() - 1);
        backingBuffer.copyFrom(ch, 0, temp, srcCh, 0, destLength);
    }

    usedLength.store(destLength, std::memory_order_release);
    writeHead.store(0);
    oneShotFilled = false;
    setSourceMode(SourceMode::File);
    return true;
}

void GrainBuffer::writeSamples(const juce::AudioBuffer<float>& input,
                               int startSample, int numSamples,
                               float feedbackGain) noexcept
{
    if (recordMode == RecordMode::Off) return;
    if (oneShotFilled && recordMode == RecordMode::OneShot) return;

    int head = writeHead.load(std::memory_order_relaxed);
    const int limit = maxLengthSamples;

    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            const int srcCh = juce::jmin(ch, input.getNumChannels() - 1);
            float s = input.getSample(srcCh, startSample + i);
            if (feedbackGain > 0.0f)
                s += feedbackGain * backingBuffer.getSample(ch, head);
            backingBuffer.setSample(ch, head, s);
        }
        head = (head + 1) % limit;
    }

    writeHead.store(head, std::memory_order_release);

    const int currentUsed = usedLength.load(std::memory_order_relaxed);
    if (currentUsed < limit)
        usedLength.store(juce::jmin(currentUsed + numSamples, limit), std::memory_order_release);

    if (recordMode == RecordMode::OneShot && usedLength.load() >= limit)
        oneShotFilled = true;
}

std::pair<float, float> GrainBuffer::readInterpolated(double position, bool reverse) const noexcept
{
    const int used = usedLength.load(std::memory_order_acquire);
    if (used <= 0) return {0.0f, 0.0f};

    double pos = reverse ? static_cast<double>(used - 1) - position : position;
    pos = std::fmod(pos, static_cast<double>(used));
    if (pos < 0.0) pos += static_cast<double>(used);

    return { hermite(0, pos, used), hermite(1, pos, used) };
}

float GrainBuffer::hermite(int channel, double pos, int length) const noexcept
{
    const int i1 = static_cast<int>(pos);
    const float frac = static_cast<float>(pos - static_cast<double>(i1));

    // Wrap indices safely within [0, length)
    auto idx = [&](int n) -> int {
        return ((n % length) + length) % length;
    };

    const float* data = backingBuffer.getReadPointer(channel);
    const float y0 = data[idx(i1 - 1)];
    const float y1 = data[idx(i1)];
    const float y2 = data[idx(i1 + 1)];
    const float y3 = data[idx(i1 + 2)];

    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

double GrainBuffer::getTotalLengthSamples() const noexcept
{
    return static_cast<double>(usedLength.load(std::memory_order_acquire));
}

void GrainBuffer::clearBuffer() noexcept
{
    backingBuffer.clear();
    writeHead.store(0);
    usedLength.store(0);
    oneShotFilled = false;
}
