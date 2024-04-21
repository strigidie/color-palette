#include "Core.hpp"

// -----------------------------------------------------------------------------
// The rule of five
// -----------------------------------------------------------------------------
Color::Color(void) : _hsl({0.0, 0.0, 0.0}), _rgb({0, 0, 0}) { }
Color::~Color() { }

// -----------------------------------------------------------------------------
// Set methods
// -----------------------------------------------------------------------------
void Color::setRGB(const ColorRGB_t& rgb)
{
    _rgb = {rgb.red, rgb.blue, rgb.green};
    _convertRGBtoHSL();
}

void Color::setHSL(const ColorHSL_t& hsl)
{
    _hsl = {hsl.hue, hsl.saturation, hsl.lightness};
    _convertHSLtoRGB();
}

// -----------------------------------------------------------------------------
// Get methods
// -----------------------------------------------------------------------------
const ColorRGB_t& Color::getRGB(void) const
{
    return _rgb;
}

const ColorHSL_t& Color::getHSL(void) const
{
    return _hsl;
}

void Color::_convertRGBtoHSL()
{
    // Normalize the values
    const float rn = static_cast<float>(_rgb.red) / UINT8_MAX;
    const float bn = static_cast<float>(_rgb.blue) / UINT8_MAX;
    const float gn = static_cast<float>(_rgb.green) / UINT8_MAX;

    // The max and min values
    const float max = fmax(rn, fmax(bn, gn));
    const float min = fmin(rn, fmin(bn, gn));

    // Delta of the max and min values
    const float delta = max - min;

    // Hue calculation
    if (delta > 0)
    {
        if (max == rn)
            _hsl.hue = 60 * fmod((gn - bn) / delta, 6.0);
        else if (max == gn)
            _hsl.hue = 60 * ((bn - rn) / delta + 2.0);
        else if (max == bn)
            _hsl.hue = 60 * ((rn - gn) / delta + 4.0);
    }
    else
        _hsl.hue = 0.0;
    
    // Lightness calcaulation
    _hsl.lightness = (max + min) / 2;
    
    // Saturation calculation
    if (_hsl.lightness > 0.0 && _hsl.lightness < 1.0)
        _hsl.saturation = delta / (1 - fabs(2 * _hsl.lightness - 1));
    else
        _hsl.saturation = 0;
}

void Color::_convertHSLtoRGB(void)
{
    // Chroma calcucation
    const float c = (1 - fabs(2 * _hsl.lightness - 1)) * _hsl.saturation;

    // Calculation of a RGB point along the bottom three faces of the RGB cube
    const float h = _hsl.hue / 60;
    const float x = c * (1 - fabs(fmod(h, 2) - 1));

    float tr, tg, tb;
    if (h >= 0 && h < 1)
    {
        tr = c;
        tg = x;
        tb = 0;
    }
    else if (h >= 1 && h < 2)
    {
        tr = x;
        tg = c;
        tb = 0;
    }
    else if (h >= 2 && h < 3)
    {
        tr = 0;
        tg = c;
        tb = x;
    }
    else if (h >= 3 && h < 4)
    {
        tr = 0;
        tg = x;
        tb = c;
    }
    else if (h >= 4 && h < 5)
    {
        tr = x;
        tg = 0;
        tb = c;
    }
    else if (h >= 5 && h < 6)
    {
        tr = c;
        tg = 0;
        tb = x;
    }

    // RGB calculation
    const float m = _hsl.lightness - (c / 2);
    _rgb = {
        static_cast<uint8_t>((tr + m) * 255),
        static_cast<uint8_t>((tg + m) * 255),
        static_cast<uint8_t>((tb + m) * 255)
    };
}