#ifndef COORDS4D_H
#define COORDS4D_H
// *****************************************************************************
// coords4d.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//     Basic operations on 4D coordinates (4-vectors, 4-points)
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "coords.h"
#include "coords3d.h"
#include <iostream>

#define TAO_EPSILON 0.0001

TAO_BEGIN

// ============================================================================
//
//   Types defined in this file
//
// ============================================================================

struct Point4;                  // 4D point
struct Vector4;                 // 4D vector

// ============================================================================
//
//   Point4 class
//
// ============================================================================

struct Point4
// ----------------------------------------------------------------------------
//    A 4D dimensional point
// ----------------------------------------------------------------------------
{
    Point4(coord X = 0, coord Y = 0, coord Z = 0, coord W=0): x(X), y(Y), z(Z), w(W) {}
    Point4(const Point4 &o): x(o.x), y(o.y), z(o.z), w(o.w) {}
    Point4(const Point3 &o): x(o.x), y(o.y), z(o.z), w(0) {}
    Point4(const Point &p): x(p.x), y(p.y), z(0), w(0) {}
    Point4 &Set(coord X, coord Y, coord Z, coord W) { x=X; y=Y; z=Z; w=W; return *this; }
    Point4& operator = (const Point4& o)
    {
        x = o.x;
        y = o.y;
        z = o.z;
        w = o.w;
        return *this;
    }
    bool operator == (const Point4&o) const
    {
        return ((fabs(x - o.x) < TAO_EPSILON) &&
                (fabs(y - o.y) < TAO_EPSILON) &&
                (fabs(z - o.z) < TAO_EPSILON) &&
                (fabs(w - o.w) < TAO_EPSILON));
    }
    bool operator != (const Point4&o) const
    {
        return ! operator ==(o);
    }

    Point4& operator +=(const Vector4& o);
    Point4& operator -=(const Vector4& o);

public:
    coord x, y, z, w;
};


// ============================================================================
//
//   Vector4 class
//
// ============================================================================

struct Vector4 : Point4
// ----------------------------------------------------------------------------
//    A four-dimensional vector
// ----------------------------------------------------------------------------
{
    Vector4(coord x = 0.0, coord y = 0.0, coord z = 0.0, coord w = 0.0): Point4(x,y,z,w) {}
    Vector4(const Vector4 &o): Point4(o) {}
    Vector4(const Vector3 &o): Point4(o) {}
    Vector4(const Vector &o): Point4(o) {}
    Vector4(const Point4 &o): Point4(o) {}
    Vector4(const Point3 &o): Point4(o) {}
    Vector4(const Point &o): Point4(o) {}

    Vector4& operator +=(const Vector4& o)
    {
        x += o.x;
        y += o.y;
        z += o.z;
        w += o.w;
        return *this;
    }

    Vector4& operator -=(const Vector4& o)
    {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        w += o.w;
        return *this;
    }

    Vector4& operator *=(scale s)
    {
        x *= s;
        y *= s;
        z *= s;
        w += s;
        return *this;
    }

    Vector4& operator /=(scale s)
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    scale Dot(const Vector4& o)
    {
        return x*o.x+y*o.y+z*o.z+w*o.w;
    }

    scale Length() const
    {
        return sqrt(x*x+y*y+z*z+w*w);
    }

    Vector4& Normalize()
    {
        if (Length() != 0)
            *this /= Length();
        return *this;
    }
};


// ============================================================================
//
//   Inline Point4 and Vector4  operations not defined in class
//
// ============================================================================

inline Point4& Point4::operator +=(const Vector4& o)
{
    x += o.x;
    y += o.y;
    z += o.z;
    w += o.w;
    return *this;
}

inline Point4& Point4::operator -=(const Vector4& o)
{
    x -= o.x;
    y -= o.y;
    z -= o.z;
    w -= o.w;
    return *this;
}

inline Point4 operator +(const Point4& l, const Vector4 &r)
{
    Point4 result(l);
    result += r;
    return result;
}

inline Vector4 operator -(const Point4& l, const Point4 &r)
{
    Vector4 result(l.x-r.x, l.y-r.y, l.z-r.z, l.w-r.w);
    return result;
}

inline Vector4 operator +(const Vector4& l, const Vector4 &r)
{
    Vector4 result(l);
    result += r;
    return result;
}

inline Vector4 operator -(const Vector4& l, const Vector4 &r)
{
    Vector4 result(l);
    result -= r;
    return result;
}

inline Vector4 operator *(const Vector4& l, scale s)
{
    Vector4 result(l);
    result *= s;
    return result;
}

inline Vector4 operator *(scale s, const Vector4& l)
{
    Vector4 result(l);
    result *= s;
    return result;
}

inline Vector4 operator /(const Vector4& l, scale s)
{
    Vector4 result(l);
    result /= s;
    return result;
}

inline coord operator* (const Vector4& l, const Vector4& r)
{
    Vector4 result(l);
    return result.Dot(r);
}


TAO_END

#endif // COORDS4D_H
