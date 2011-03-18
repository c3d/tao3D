#ifndef COLOR_H
#define COLOR_H
// ****************************************************************************
//  color.h                                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//    Representation of colors (essentially as 4-dimensional vectors)
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"

TAO_BEGIN

struct Color
// ----------------------------------------------------------------------------
//    A three-dimensional vector
// ----------------------------------------------------------------------------
{
    typedef float percent;

    Color(percent r = 0.0, percent g = 0.0, percent b = 0.0, percent a = 0.0)
        : red(r), green(g), blue(b), alpha(a) {}
    Color(const Color &o)
        : red(o.red), green(o.green), blue(o.blue), alpha(o.alpha) {}

    Color& operator =(const Color& o)
    {
        red   = o.red;
        blue  = o.blue;
        green = o.green;
        alpha = o.alpha;
        return *this;
    }

    Color& Set(percent r, percent g, percent b, percent a = 1)
    {
        red   = r;
        blue  = b;
        green = g;
        alpha = a;
        return *this;
    }

    Color& operator +=(const Color& o)
    {
        red   += o.red;
        blue  += o.blue;
        green += o.green;
        alpha += o.alpha;
        return *this;
    }

    Color& operator -=(const Color& o)
    {
        red   -= o.red;
        blue  -= o.blue;
        green -= o.green;
        alpha -= o.alpha;
        return *this;
    }

    Color& operator *=(scale s)
    {
        red   *= s;
        blue  *= s;
        green *= s;
        alpha *= s;
        return *this;
    }
    
    Color& operator /=(scale s)
    {
        red   /= s;
        blue  /= s;
        green /= s;
        alpha /= s;
        return *this;
    }

    scale Intensity() const
    {
        return sqrt(red*red+green*green+blue*blue);
    }

    Color& Normalize()
    {
        if (red < 0)    red = 0;
        if (green < 0)  green = 0;
        if (blue < 0)   blue = 0;
        if (alpha < 0)  alpha = 0;
        if (red > 1)    red = 1;
        if (green > 1)  green = 1;
        if (blue > 1)   blue = 1;
        if (alpha > 1)  alpha = 1;
        return *this;
    }

public:
    percent red, green, blue, alpha;
};



// ============================================================================
//
//   Inline Color operations not defined in class
//
// ============================================================================

inline Color operator +(const Color& l, const Color &r)
{
    Color result(l);
    result += r;
    return result;
}

inline Color operator -(const Color& l, const Color &r)
{
    Color result(l);
    result -= r;
    return result;
}

inline Color operator *(const Color& l, scale s)
{
    Color result(l);
    result *= s;
    return result;
}

inline Color operator *(scale s, const Color& l)
{
    Color result(l);
    result *= s;
    return result;
}

inline Color operator /(const Color& l, scale s)
{
    Color result(l);
    result /= s;
    return result;
}

TAO_END

#endif // COLOR_H
