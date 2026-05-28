#include "FilterCurveDisplay.h"

FilterCurveDisplay::FilterCurveDisplay(juce::AudioProcessorValueTreeState& a) : apvts(a)
{
    startTimerHz(20);
}

FilterCurveDisplay::~FilterCurveDisplay() { stopTimer(); }

void FilterCurveDisplay::timerCallback()
{
    const float cutoff = apvts.getParameterRange("filter_cutoff_hz")
                             .convertFrom0to1(apvts.getParameter("filter_cutoff_hz")->getValue());
    const float res    = apvts.getParameterRange("filter_resonance")
                             .convertFrom0to1(apvts.getParameter("filter_resonance")->getValue());
    const int   type   = juce::roundToInt(
                             apvts.getParameterRange("filter_type")
                                 .convertFrom0to1(apvts.getParameter("filter_type")->getValue()));

    if (std::abs(cutoff - cachedCutoff) > 0.5f ||
        std::abs(res    - cachedRes)    > 0.001f ||
        type != cachedType)
    {
        cachedCutoff = cutoff;
        cachedRes    = res;
        cachedType   = type;
        rebuildPath();
        repaint();
    }
}

void FilterCurveDisplay::rebuildPath()
{
    const float w = static_cast<float>(getWidth());
    const float h = static_cast<float>(getHeight());
    if (w < 2.0f || h < 2.0f) return;

    using Coeffs = juce::dsp::IIR::Coefficients<float>;
    juce::ReferenceCountedObjectPtr<Coeffs> coeffs;

    const float cutoff = juce::jlimit(20.0f, 20000.0f, cachedCutoff);
    const float q      = juce::jlimit(0.1f,  18.0f,    cachedRes);

    switch (cachedType)
    {
        case 1:  coeffs = Coeffs::makeHighPass  (kSampleRate, cutoff, q); break;
        case 2:  coeffs = Coeffs::makeBandPass  (kSampleRate, cutoff, q); break;
        case 3:  coeffs = Coeffs::makeNotch      (kSampleRate, cutoff, q); break;
        default: coeffs = Coeffs::makeLowPass   (kSampleRate, cutoff, q); break;
    }

    curvePath.clear();

    constexpr double kLogMin = std::log10(20.0);
    constexpr double kLogMax = std::log10(20000.0);

    for (int i = 0; i < kNumPoints; ++i)
    {
        const double t    = static_cast<double>(i) / (kNumPoints - 1);
        const double freq = std::pow(10.0, kLogMin + t * (kLogMax - kLogMin));
        const double mag  = coeffs->getMagnitudeForFrequency(freq, kSampleRate);
        const double db   = juce::Decibels::gainToDecibels(static_cast<float>(mag), -60.0f);

        // Map dB [-30, +12] to pixel y
        const float normY = static_cast<float>(juce::jmap(db, -30.0, 12.0, 1.0, 0.0));
        const float px    = t * w;
        const float py    = normY * h;

        if (i == 0) curvePath.startNewSubPath(px, py);
        else        curvePath.lineTo(px, py);
    }

    // Close to bottom for fill
    curvePath.lineTo(w, h);
    curvePath.lineTo(0.0f, h);
    curvePath.closeSubPath();
}

void FilterCurveDisplay::resized()
{
    rebuildPath();
}

void FilterCurveDisplay::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(bounds);
    g.setColour(juce::Colour(0xff282828));
    g.drawRect(bounds, 1.0f);

    // 0 dB reference line
    const float zeroY = static_cast<float>(juce::jmap(0.0, -30.0, 12.0, 1.0, 0.0)) * bounds.getHeight();
    g.setColour(juce::Colour(0xff303030));
    g.drawHorizontalLine(juce::roundToInt(zeroY), bounds.getX(), bounds.getRight());

    if (!curvePath.isEmpty())
    {
        g.setColour(juce::Colour(0x22a0c8ff));
        g.fillPath(curvePath);

        juce::PathStrokeType stroke(1.2f);
        g.setColour(juce::Colour(0xffa0c8ff));
        g.strokePath(curvePath, stroke);
    }
}
