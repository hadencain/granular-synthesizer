#include "GranularEngine.h"
#include "GrainEnvelope.h"
#include <cmath>

void GranularEngine::prepare(double sr, int /*blockSize*/)
{
    sampleRate = sr;
    reset();
}

void GranularEngine::reset()
{
    for (auto& g : grainPool) g.reset();
    nextGrainIndex    = 0;
    schedulerCountdown = 0.0;
    playheadPos       = 0.0;
    scanPhase         = 0.0;
    pendulumDir       = 1;
}

void GranularEngine::resetPlayhead() noexcept
{
    playheadPos  = 0.0;
    scanPhase    = 0.0;
    pendulumDir  = 1;
}

int GranularEngine::getActiveGrainCount() const noexcept
{
    int count = 0;
    for (const auto& g : grainPool)
        if (g.active) ++count;
    return count;
}

void GranularEngine::collectGrainPositions(juce::Array<float>& out, int bufferLengthSamples) const noexcept
{
    if (bufferLengthSamples <= 0) return;
    for (const auto& g : grainPool)
    {
        if (g.active)
        {
            const float pos = static_cast<float>(g.readPosition) / static_cast<float>(bufferLengthSamples);
            out.add(juce::jlimit(0.0f, 1.0f, pos));
        }
    }
}

Grain* GranularEngine::allocateGrain() noexcept
{
    // Round-robin: skip active grains up to one full cycle
    for (int attempt = 0; attempt < MAX_GRAINS; ++attempt)
    {
        Grain& g = grainPool[nextGrainIndex];
        nextGrainIndex = (nextGrainIndex + 1) % MAX_GRAINS;
        if (!g.active) return &g;
    }
    // Pool exhausted: steal the oldest active grain
    Grain* oldest = &grainPool[0];
    for (auto& g : grainPool)
        if (g.active && g.age > oldest->age) oldest = &g;
    oldest->reset();
    return oldest;
}

double GranularEngine::computeInteronsetSamples(const GranularParams& p) const noexcept
{
    // Use the larger of the density-derived period and the explicit interonset param
    const float densityDerived = (p.grainDensity > 0.0f)
        ? (1000.0f / p.grainDensity) : p.interonsetMs;
    const float base = densityDerived;
    const float jitterMs = static_cast<float>(rng.nextFloat()) *
                           juce::jlimit(0.0f, 100.0f, base * 0.3f);
    return static_cast<double>((base + jitterMs) / 1000.0f) * sampleRate;
}

void GranularEngine::advancePlayhead(const GranularParams& p, int numSamples, double bufLen)
{
    if (bufLen <= 0.0 || p.freeze) return;

    const double loopStart = p.loopStart * bufLen;
    const double loopEnd   = juce::jmax(loopStart + 1.0, p.loopEnd * bufLen);
    const double loopRange = loopEnd - loopStart;

    const double rateHz = juce::jmax(0.001, static_cast<double>(
        p.scanRateHz + p.modScanRate * 10.0f));
    const double advancePerSample = rateHz / sampleRate;

    const auto shape = static_cast<ScanShape>(p.scanShape);

    switch (shape)
    {
        case ScanShape::Forward:
            playheadPos += advancePerSample * static_cast<double>(numSamples) * loopRange;
            if (playheadPos > loopEnd) playheadPos = loopStart;
            break;

        case ScanShape::Reverse:
            playheadPos -= advancePerSample * static_cast<double>(numSamples) * loopRange;
            if (playheadPos < loopStart) playheadPos = loopEnd;
            break;

        case ScanShape::Pendulum:
            playheadPos += pendulumDir * advancePerSample *
                           static_cast<double>(numSamples) * loopRange;
            if (playheadPos >= loopEnd)  { playheadPos = loopEnd;   pendulumDir = -1; }
            if (playheadPos <= loopStart){ playheadPos = loopStart; pendulumDir =  1; }
            break;

        case ScanShape::Sine:
        {
            scanPhase += advancePerSample * static_cast<double>(numSamples) * 2.0 * juce::MathConstants<double>::pi;
            if (scanPhase > juce::MathConstants<double>::twoPi)
                scanPhase -= juce::MathConstants<double>::twoPi;
            playheadPos = loopStart + (std::sin(scanPhase) * 0.5 + 0.5) * loopRange;
            break;
        }

        case ScanShape::RandomWalk:
        {
            const double step = (rng.nextDouble() * 2.0 - 1.0) *
                                advancePerSample * static_cast<double>(numSamples) * loopRange;
            playheadPos = juce::jlimit(loopStart, loopEnd, playheadPos + step);
            break;
        }
    }

    // Clamp to loop region
    playheadPos = juce::jlimit(loopStart, loopEnd, playheadPos);
}

static float quantizePitch(float pitchSt, int mode)
{
    if (mode == 0) return pitchSt;                  // Off
    if (mode == 1) return std::round(pitchSt);      // Chromatic

    static const float major[] = { 0, 2, 4, 5, 7, 9, 11 };
    static const float minor[] = { 0, 2, 3, 5, 7, 8, 10 };
    const float* scale = (mode == 2) ? major : minor; // 2=Major, 3=Minor, 4+=Off

    if (mode > 3) return pitchSt; // Custom not yet implemented

    const float octave = std::floor(pitchSt / 12.0f) * 12.0f;
    const float pc     = pitchSt - octave;

    float best = scale[0], bestDist = std::abs(pc - best);
    for (int i = 1; i < 7; ++i)
    {
        const float d = std::abs(pc - scale[i]);
        if (d < bestDist) { bestDist = d; best = scale[i]; }
    }
    if (std::abs(pc - 12.0f) < bestDist)
        return octave + 12.0f;

    return octave + best;
}

void GranularEngine::spawnGrain(const GranularParams& p,
                                const GrainBuffer& buf,
                                double bufLen)
{
    if (bufLen <= 0.0) return;

    Grain* g = allocateGrain();
    if (g == nullptr) return;

    // Size with per-grain randomization
    const float sizeMs = juce::jmax(1.0f,
        p.grainSizeMs + p.modGrainSize * 500.0f +
        (rng.nextFloat() * 2.0f - 1.0f) * p.randomizeSizeMs);
    g->grainSizeSamples = static_cast<int>(sizeMs / 1000.0f * sampleRate);
    g->age = 0;

    // Position with spray
    const double sprayRange = (p.sprayMs + p.modSpray * 500.0f) / 1000.0 * sampleRate;
    const double spray = (rng.nextDouble() * 2.0 - 1.0) * sprayRange;
    const double basePos = (p.position + p.modPosition) * bufLen;
    g->readPosition = juce::jlimit(0.0, bufLen - 1.0, playheadPos + basePos + spray);

    // Pitch
    float pitchSt = p.pitchShiftSemitones + p.detuneCents / 100.0f
                  + p.modPitch * 48.0f
                  + (rng.nextFloat() * 2.0f - 1.0f) * p.pitchRandomSemitones
                  + p.voiceDetuneOffset / 100.0f;

    // Add MIDI note offset relative to middle C (60)
    pitchSt += (p.midiNote - 60.0f);

    pitchSt = quantizePitch(pitchSt, p.pitchQuantize);

    g->pitchRatio = std::pow(2.0, static_cast<double>(pitchSt) / 12.0)
                  * static_cast<double>(p.playbackRate);

    // Amplitude
    const float ampRandDb = (rng.nextFloat() * 2.0f - 1.0f) * p.ampRandomDb;
    const float ampRandLin = std::pow(10.0f, ampRandDb / 20.0f);
    g->amplitude = juce::jlimit(0.0f, 1.0f,
        (p.amplitude + p.modAmplitude) * p.velocity * ampRandLin);

    // Pan
    float panVal = juce::jlimit(-1.0f, 1.0f,
        p.pan + p.modPan + (rng.nextFloat() * 2.0f - 1.0f) * p.panRandom);
    g->pan = panVal;
    // Equal-power panning
    const float angle = (panVal + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
    g->panL = std::cos(angle);
    g->panR = std::sin(angle);

    // Direction
    const auto dir = static_cast<GrainDirection>(p.grainDirection);
    if (dir == GrainDirection::Forward)       g->reverse = false;
    else if (dir == GrainDirection::Reverse)  g->reverse = true;
    else if (dir == GrainDirection::Bidirectional)
        g->reverse = (g->age % 2 == 0);
    else // Random
        g->reverse = rng.nextBool();

    g->envShape = p.grainEnvelopeShape;
    g->buffer   = &buf;
    g->active   = true;
}

void GranularEngine::process(juce::AudioBuffer<float>& outputBuffer,
                             int startSample, int numSamples,
                             const GranularParams& params,
                             const GrainBuffer& buffer)
{
    const double bufLen = buffer.getTotalLengthSamples();

    advancePlayhead(params, numSamples, bufLen);

    float* outL = outputBuffer.getWritePointer(0, startSample);
    float* outR = outputBuffer.getWritePointer(1, startSample);

    for (int s = 0; s < numSamples; ++s)
    {
        // --- Scheduler ---
        if (--schedulerCountdown <= 0.0)
        {
            if (params.grainProbability >= 1.0f || rng.nextFloat() < params.grainProbability)
                spawnGrain(params, buffer, bufLen);
            schedulerCountdown = computeInteronsetSamples(params);
        }

        // --- Render active grains ---
        float sumL = 0.0f, sumR = 0.0f;

        for (auto& g : grainPool)
        {
            if (!g.active) continue;

            const float env = GrainEnvelope::getSample(
                GrainEnvelope::fromInt(g.envShape), g.normalizedAge());

            auto [sL, sR] = buffer.readInterpolated(g.readPosition, g.reverse);
            sL *= g.amplitude * env * g.panL;
            sR *= g.amplitude * env * g.panR;
            sumL += sL;
            sumR += sR;

            // Advance read position
            g.readPosition += g.pitchRatio * (g.reverse ? -1.0 : 1.0);
            ++g.age;

            if (g.age >= g.grainSizeSamples)
                g.reset();
        }

        outL[s] += sumL;
        outR[s] += sumR;
    }
}
