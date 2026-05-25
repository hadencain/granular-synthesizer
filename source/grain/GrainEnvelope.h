#pragma once
#include <cmath>

class GrainEnvelope
{
public:
    enum class Shape { Hann = 0, Gaussian, Trapezoid, Rectangle, Triangular, Tukey };

    // phase in [0, 1] — normalized position within grain lifetime
    static float getSample(Shape shape, float phase) noexcept
    {
        switch (shape)
        {
            case Shape::Hann:       return hann(phase);
            case Shape::Gaussian:   return gaussian(phase);
            case Shape::Trapezoid:  return trapezoid(phase);
            case Shape::Rectangle:  return 1.0f;
            case Shape::Triangular: return triangular(phase);
            case Shape::Tukey:      return tukey(phase, 0.5f);
            default:                return hann(phase);
        }
    }

    static Shape fromInt(int i) noexcept
    {
        if (i >= 0 && i <= 5) return static_cast<Shape>(i);
        return Shape::Hann;
    }

private:
    static constexpr float kPi = 3.14159265358979323846f;
    static constexpr float kTwoPi = 2.0f * kPi;

    static float hann(float p) noexcept
    {
        return 0.5f * (1.0f - std::cos(kTwoPi * p));
    }

    static float gaussian(float p) noexcept
    {
        // sigma = 0.4; centre at 0.5
        constexpr float sigma = 0.4f;
        const float x = (p - 0.5f) / sigma;
        return std::exp(-0.5f * x * x);
    }

    static float trapezoid(float p) noexcept
    {
        // 10% fade-in and fade-out, flat top in between
        constexpr float fadeWidth = 0.1f;
        if (p < fadeWidth)          return p / fadeWidth;
        if (p > 1.0f - fadeWidth)   return (1.0f - p) / fadeWidth;
        return 1.0f;
    }

    static float triangular(float p) noexcept
    {
        return 1.0f - std::abs(2.0f * p - 1.0f);
    }

    static float tukey(float p, float alpha) noexcept
    {
        // alpha controls the fraction of the window spent in cosine taper
        if (alpha <= 0.0f) return 1.0f;
        if (alpha >= 1.0f) return hann(p);

        const float halfAlpha = alpha * 0.5f;
        if (p < halfAlpha)
            return 0.5f * (1.0f - std::cos(kPi * p / halfAlpha));
        if (p > 1.0f - halfAlpha)
            return 0.5f * (1.0f - std::cos(kPi * (1.0f - p) / halfAlpha));
        return 1.0f;
    }
};
