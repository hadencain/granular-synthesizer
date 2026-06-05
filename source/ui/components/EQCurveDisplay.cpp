#include "EQCurveDisplay.h"
#include <cmath>

EQCurveDisplay::EQCurveDisplay(juce::AudioProcessorValueTreeState& a) : apvts(a)
{
    startTimerHz(30);
}

EQCurveDisplay::~EQCurveDisplay()
{
    stopTimer();
}

void EQCurveDisplay::timerCallback()
{
    rebuildPath();
    repaint();
}

void EQCurveDisplay::resized()
{
    pathDirty = true;
}

float EQCurveDisplay::biquadMagnitudeDB(float freqHz, float sampleRate,
                                         float fcHz, float gainDB, float q) noexcept
{
    // Peaking EQ biquad magnitude response
    const double w0    = 2.0 * juce::MathConstants<double>::pi * fcHz / sampleRate;
    const double A     = std::pow(10.0, gainDB / 40.0);
    const double alpha = std::sin(w0) / (2.0 * q);
    const double cosW0 = std::cos(w0);

    const double b0 =  1.0 + alpha * A;
    const double b1 = -2.0 * cosW0;
    const double b2 =  1.0 - alpha * A;
    const double a0 =  1.0 + alpha / A;
    const double a1 = -2.0 * cosW0;
    const double a2 =  1.0 - alpha / A;

    const double w  = 2.0 * juce::MathConstants<double>::pi * freqHz / sampleRate;
    const std::complex<double> z1 = std::exp(std::complex<double>(0.0, -w));
    const std::complex<double> z2 = z1 * z1;

    const std::complex<double> num = b0 + b1 * z1 + b2 * z2;
    const std::complex<double> den = a0 + a1 * z1 + a2 * z2;
    const double mag = std::abs(num / den);

    return static_cast<float>(20.0 * std::log10(std::max(mag, 1e-6)));
}

void EQCurveDisplay::rebuildPath()
{
    const int w = getWidth();
    const int h = getHeight();
    if (w <= 0 || h <= 0) return;

    float fc[4], gain[4], q[4];
    for (int b = 0; b < 4; ++b)
    {
        const auto ns = juce::String(b + 1);
        fc  [b] = apvts.getRawParameterValue("fx_eq" + ns + "_freq")->load();
        gain[b] = apvts.getRawParameterValue("fx_eq" + ns + "_gain")->load();
        q   [b] = apvts.getRawParameterValue("fx_eq" + ns + "_q"   )->load();
    }

    curvePath.clear();
    const float hf = h * 0.5f;

    for (int i = 0; i < kNumPoints; ++i)
    {
        const float t    = static_cast<float>(i) / static_cast<float>(kNumPoints - 1);
        const float freq = kMinFreq * std::pow(kMaxFreq / kMinFreq, t);
        const float px   = t * static_cast<float>(w);

        float totalDB = 0.0f;
        for (int b = 0; b < 4; ++b)
            totalDB += biquadMagnitudeDB(freq, kSampleRate, fc[b], gain[b], q[b]);

        const float py = hf - totalDB * (hf / kMaxDB);

        if (i == 0) curvePath.startNewSubPath(px, py);
        else        curvePath.lineTo(px, py);
    }

    pathDirty = false;
}

juce::Point<float> EQCurveDisplay::bandDotPosition(int band) const noexcept
{
    const int w = getWidth();
    const int h = getHeight();
    const auto ns = juce::String(band + 1);
    const float fc   = apvts.getRawParameterValue("fx_eq" + ns + "_freq")->load();
    const float gain = apvts.getRawParameterValue("fx_eq" + ns + "_gain")->load();

    const float t  = std::log(fc / kMinFreq) / std::log(kMaxFreq / kMinFreq);
    const float px = t * static_cast<float>(w);
    const float py = h * 0.5f - gain * (h * 0.5f / kMaxDB);
    return { px, py };
}

float EQCurveDisplay::xToFreqNorm(float px) const noexcept
{
    const float t = juce::jlimit(0.0f, 1.0f, px / static_cast<float>(getWidth()));
    const float freq = kMinFreq * std::pow(kMaxFreq / kMinFreq, t);
    if (auto* p = apvts.getParameter("fx_eq1_freq"))
        return p->convertTo0to1(freq);
    return t;
}

float EQCurveDisplay::yToGainNorm(float py) const noexcept
{
    const float hf   = static_cast<float>(getHeight()) * 0.5f;
    const float gain = (hf - py) * (kMaxDB / hf);
    const float clamped = juce::jlimit(-kMaxDB, kMaxDB, gain);
    if (auto* p = apvts.getParameter("fx_eq1_gain"))
        return p->convertTo0to1(clamped);
    return (clamped + kMaxDB) / (2.0f * kMaxDB);
}

int EQCurveDisplay::hitTest(juce::Point<float> pt) const noexcept
{
    for (int b = 0; b < 4; ++b)
    {
        if (bandDotPosition(b).getDistanceFrom(pt) < 8.0f)
            return b;
    }
    return -1;
}

void EQCurveDisplay::setBandFreqNorm(int band, float norm)
{
    const auto id = "fx_eq" + juce::String(band + 1) + "_freq";
    if (auto* p = apvts.getParameter(id))
        p->setValueNotifyingHost(norm);
}

void EQCurveDisplay::setBandGainNorm(int band, float norm)
{
    const auto id = "fx_eq" + juce::String(band + 1) + "_gain";
    if (auto* p = apvts.getParameter(id))
        p->setValueNotifyingHost(norm);
}

void EQCurveDisplay::mouseDown(const juce::MouseEvent& e)
{
    dragBand   = hitTest(e.position);
    isDragging = dragBand >= 0;
}

void EQCurveDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDragging || dragBand < 0) return;
    setBandFreqNorm(dragBand, xToFreqNorm(e.position.x));
    setBandGainNorm(dragBand, yToGainNorm(e.position.y));
}

void EQCurveDisplay::mouseUp(const juce::MouseEvent&)
{
    isDragging = false;
    dragBand   = -1;
}

void EQCurveDisplay::mouseMove(const juce::MouseEvent& e)
{
    const bool nearDot = hitTest(e.position) >= 0;
    setMouseCursor(nearDot ? juce::MouseCursor::UpDownLeftRightResizeCursor
                           : juce::MouseCursor::NormalCursor);
}

void EQCurveDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const int  w = getWidth();
    const int  h = getHeight();

    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillRect(bounds);

    g.setColour(juce::Colour(0xff282828));
    g.drawHorizontalLine(h / 2, 0.0f, static_cast<float>(w));

    if (!curvePath.isEmpty())
    {
        juce::Path filled = curvePath;
        filled.lineTo(static_cast<float>(w), h * 0.5f);
        filled.lineTo(0.0f, h * 0.5f);
        filled.closeSubPath();
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        g.fillPath(filled);

        g.setColour(juce::Colour(0xffc8c8c8));
        g.strokePath(curvePath, juce::PathStrokeType(1.5f));
    }

    for (int b = 0; b < 4; ++b)
    {
        const auto dot = bandDotPosition(b);
        g.setColour(juce::Colour(0xff909090));
        g.fillEllipse(dot.x - 4.0f, dot.y - 4.0f, 8.0f, 8.0f);
        g.setColour(juce::Colour(0xff484848));
        g.drawEllipse(dot.x - 4.0f, dot.y - 4.0f, 8.0f, 8.0f, 1.0f);
    }

    g.setColour(juce::Colour(0xff282828));
    g.drawRect(bounds, 1.0f);
}
