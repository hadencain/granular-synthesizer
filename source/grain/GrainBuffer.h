#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <utility>

class GrainBuffer
{
public:
    enum class SourceMode { File, LiveRing };
    enum class RecordMode { Off = 0, Continuous, OneShot, Gate };

    GrainBuffer();

    // Called from message thread — allocates backing buffer at max size
    void prepare(double sampleRate);

    // File mode: load and resample to host sample rate
    bool loadFile(const juce::File& file, juce::AudioFormatManager& formatManager);

    // Live ring: called from audio thread inside processBlock
    void writeSamples(const juce::AudioBuffer<float>& input,
                      int startSample, int numSamples,
                      float feedbackGain = 0.0f) noexcept;

    // Hermite-interpolated stereo read at fractional position [0, usedLength)
    // reverse: if true, reads backwards from (usedLength-1 - pos)
    std::pair<float, float> readInterpolated(double position, bool reverse) const noexcept;

    // Accessors
    double      getTotalLengthSamples() const noexcept;
    SourceMode  getSourceMode() const noexcept { return sourceMode.load(); }
    void        setSourceMode(SourceMode m) noexcept { sourceMode.store(m); }
    void        setRecordMode(RecordMode m) noexcept { recordMode = m; }
    RecordMode  getRecordMode() const noexcept { return recordMode; }
    void        clearBuffer() noexcept;

    double getSampleRate() const noexcept { return sampleRate; }

private:
    juce::AudioBuffer<float> backingBuffer;  // 2 ch, allocated once
    double sampleRate     = 44100.0;
    int    maxLengthSamples = 0;

    std::atomic<int>  writeHead    { 0 };
    std::atomic<int>  usedLength   { 0 };
    std::atomic<SourceMode> sourceMode { SourceMode::File };
    RecordMode recordMode = RecordMode::Off;
    bool oneShotFilled = false;

    // Read one channel sample with Hermite cubic interpolation, wrapping within [0, length)
    float hermite(int channel, double pos, int length) const noexcept;
};
