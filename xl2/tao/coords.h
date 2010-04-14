#ifndef COORDS_H
#define COORDS_H
// ****************************************************************************
//  coords.h                                                        Tao project
// ****************************************************************************
// 
//   File Description:
// 
//     Basic operations on 2D coordinates (2-vectors, 2-points)
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"
#include "tao.h"
#include <cmath>                // For sqrt()

TAO_BEGIN

// ============================================================================
// 
//   Types defined in this file
// 
// ============================================================================

typedef double coord;           // Coordinate type
typedef double scale;           // Scale type
struct Point;                   // 2D point
struct Vector;                  // 2D vector


// ============================================================================
// 
//   Point class
// 
// ============================================================================

struct Point
// ----------------------------------------------------------------------------
//    A two-dimensional point
// ----------------------------------------------------------------------------
{
    Point(coord X = 0, coord Y = 0): x(X), y(Y) {}
    Point(const Point &o): x(o.x), y(o.y) {}
    Point &Set(coord X, coord Y) { x = X; y = Y; return *this; }
    Point& operator = (const Point& o)
    {
        x = o.x;
        y = o.y;
        return *this;
    }
    bool operator == (const Point&o) const
    {
        return x == o.x && y == o.y;
    }
    bool operator != (const Point&o) const
    {
        return ! operator ==(o);
    }

    Point& operator +=(const Vector& o);
    Point& operator -=(const Vector& o);

public:
    coord x, y;
};



// ============================================================================
//
//   Vector class
//
// ============================================================================

struct Vector : Point
// ----------------------------------------------------------------------------
//    A two-dimensional vector
// ----------------------------------------------------------------------------
{
    Vector(coord x = 0.0, coord y = 0.0): Point(x,y) {}
    Vector(const Vector &o): Point(o) {}

    Vector& operator +=(const Vector& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    Vector& operator -=(const Vector& o)
    {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    Vector& operator *=(scale s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    
    Vector& operator /=(scale s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    scale Length() const
    {
        return sqrt(x*x+y*y);
    }

    Vector& Normalize()
    {
        *this /= Length();
        return *this;
    }
};



// ============================================================================
//
//   Inline Point and Vector operations not defined in class
//
// ============================================================================

inline Point& Point::operator +=(const Vector& o)
{
    x += o.x;
    y += o.y;
    return *this;
}

inline Point& Point::operator -=(const Vector& o)
{
    x -= o.x;
    y -= o.y;
    return *this;
}

inline Point operator +(const Point& l, const Vector &r)
{
    Point result(l);
    result += r;
    return result;
}

inline Vector operator -(const Point& l, const Point &r)
{
    Vector result(l.x-r.x, l.y-r.y);
    return result;
}

inline Vector operator +(const Vector& l, const Vector &r)
{
    Vector result(l);
    result += r;
    return result;
}
inline Vector operator -(const Vector& l, const Vector &r)
{
    Vector result(l);
    result -= r;
    return result;
}

inline Vector operator *(const Vector& l, scale s)
{
    Vector result(l);
    result *= s;
    return result;
}

inline Vector operator *(scale s, const Vector& l)
{
    Vector result(l);
    result *= s;
    return result;
}

inline Vector operator /(const Vector& l, scale s)
{
    Vector result(l);
    result /= s;
    return result;
}

inline coord operator* (const Vector& l, const Vector& r)
{
    return l.x*r.x + l.y*r.y;
}



// ============================================================================
//
//    Box class
//
// ============================================================================

struct Box
// ----------------------------------------------------------------------------
//   A simple 2D box, used most often as a bounding box
// ----------------------------------------------------------------------------
{
    Box(): lower(inf, inf), upper(-inf, -inf) {}
    Box(const Point &l, const Point &u): lower(l), upper(u) {}
    Box(const Point &l, const Vector &s): lower(l), upper(l+s) {}
    Box(coord x, coord y, coord w, coord h): lower(x, y), upper(x+w, y+h) {}
    Box(const Box &o): lower(o.lower), upper(o.upper) {}

    Box & operator = (const Box &o)
    {
        lower = o.lower;
        upper = o.upper;
        return *this;
    }

    bool operator == (const Box &o) const
    {
        return lower == o.lower && upper == o.upper;
    }

    bool operator != (const Box &o) const
    {
        return ! operator==(o);
    }

    bool operator <= (const Box &o) const
    {
        // Check if box is included in other box (not a total order)
        return lower.x >= o.lower.x
            && lower.y >= o.lower.y
            && upper.x <= o.upper.x
            && upper.y <= o.upper.y;
    }

    bool operator < (const Box &o) const
    {
        // Check if box is included strictly in other box
        return lower.x > o.lower.x
            && lower.y > o.lower.y
            && upper.x < o.upper.x
            && upper.y < o.upper.y;
    }

    bool operator >= (const Box &o) const
    {
        return o.operator <= (*this);
    }

    bool operator > (const Box &o) const
    {
        return o.operator < (*this);
    }

    Box & operator += (const Vector &o)
    {
        lower += o;
        upper += o;
        return *this;
    }

    Box & operator -= (const Vector &o)
    {
        lower -= o;
        upper -= o;
        return *this;
    }

    Box & operator *= (scale s)
    {
        // Scale box around its center
        Point center = Center();
        lower = center + (lower - center) * s;
        upper = center + (upper - center) * s;
        return *this;
    }

    Box &operator |= (const Box &o)
    {
        // Return union of the two boxes (smallest box containing both)
        if (o.lower.x < lower.x)        lower.x = o.lower.x;
        if (o.lower.y < lower.y)        lower.y = o.lower.y;

        if (o.upper.x > upper.x)        upper.x = o.upper.x;
        if (o.upper.y > upper.y)        upper.y = o.upper.y;

        return *this;
    }

    Box &operator &= (const Box &o)
    {
        // Return intersection of the two boxes (biggest box contained in both)
        if (o.lower.x > lower.x)        lower.x = o.lower.x;
        if (o.lower.y > lower.y)        lower.y = o.lower.y;

        if (o.upper.x < upper.x)        upper.x = o.upper.x;
        if (o.upper.y < upper.y)        upper.y = o.upper.y;

        return *this;
    }

    Box &Empty()
    {
        lower = Point( inf,  inf);
        upper = Point(-inf, -inf);
        return *this;
    }

    bool IsEmpty() const
    {
        return lower.x >= upper.x
            || lower.y >= upper.y;
    }

    Box &Normalize()
    {
        if (lower.x > upper.x)  std::swap(lower.x, upper.x);
        if (lower.y > upper.y)  std::swap(lower.y, upper.y);
        return *this;
    }

    Point UpperLeft() const
    {
        return Point(lower.x, upper.y);
    }
    Point UpperRight() const
    {
        return upper;
    }
    Point LowerLeft() const
    {
        return lower;
    }
    Point LowerRight() const
    {
        return Point(upper.x, lower.y);
    }
    Point Center() const
    {
        return Point((lower.x + upper.x)/2, (lower.y + upper.y)/2);
    }
    coord Width() const      { return upper.x - lower.x; }
    coord Height() const     { return upper.y - lower.y; }
    coord Left() const       { return lower.x; }
    coord Right() const      { return upper.x; }
    coord Top() const        { return upper.y; }
    coord Bottom() const     { return lower.y; }

public:
    Point lower, upper;
    static const coord inf = 1e32;
};



// ============================================================================
// 
//   Inline operations not defined in class
// 
// ============================================================================

inline Box operator+ (const Box &b, const Vector &v)
{
    Box result(b);
    result += v;
    return result;
}


inline Box operator* (const Box &b, scale s)
// ----------------------------------------------------------------------------
//   Translate the box by a given vector
// ----------------------------------------------------------------------------
{
    Box result(b);
    result *= s;
    return result;
}


inline Box operator* (scale s, const Box &b)
// ----------------------------------------------------------------------------
//   Translate the box by a given vector
// ----------------------------------------------------------------------------
{
    Box result(b);
    result *= s;
    return result;
}


inline Point operator/ (const Point &p, const Box &b)
// ----------------------------------------------------------------------------
//   Return the point as scaled within the bounding box
// ----------------------------------------------------------------------------
{
    coord dx = b.upper.x - b.lower.x; dx += dx == 0.0;
    coord dy = b.upper.y - b.lower.y; dy += dy == 0.0;
    return Point((p.x - b.lower.x) / dx,
                 (p.y - b.lower.y) / dy);
}

TAO_END

#endif // COORD_H
