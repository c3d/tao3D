#ifndef MATRIX_H
#define MATRIX_H
// *****************************************************************************
// matrix.h                                                        Tao3D project
// *****************************************************************************
//
// File description:
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
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2012,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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

#include "coords3d.h"
#include "coords.h"
#include <iostream>
#include "string.h"

TAO_BEGIN

struct Matrix4
// ----------------------------------------------------------------------------
//    A matrix 4x4.
// ----------------------------------------------------------------------------
{
    Matrix4(bool loadIdentity = true)
        : type (UNKNOWN)
    {
        if(loadIdentity)
            LoadIdentity();
    }

    Matrix4(const coord *data)
        : type(UNKNOWN)
    {
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                m[col][row] = *data++;
    }

    Matrix4(coord m11, coord m12, coord m13, coord m14,
            coord m21, coord m22, coord m23, coord m24,
            coord m31, coord m32, coord m33, coord m34,
            coord m41, coord m42, coord m43, coord m44)
        : type(UNKNOWN)
    {
        m[0][0] = m11; m[0][1] = m21; m[0][2] = m31; m[0][3] = m41;
        m[1][0] = m12; m[1][1] = m22; m[1][2] = m32; m[1][3] = m42;
        m[2][0] = m13; m[2][1] = m23; m[2][2] = m33; m[2][3] = m43;
        m[3][0] = m14; m[3][1] = m24; m[3][2] = m34; m[3][3] = m44;
    }


    Matrix4(const Matrix4& o)
        : type(o.type)
    {
        memcpy(m, o.m, sizeof m);
    }


    coord* Data(bool changed = true)
    {
       // We don't know the result matrix (possibly modified by user)
       // But if specified, then do not changed type
       if(changed)
           type = UNKNOWN;

       return m[0];
    }

    inline coord& operator()(int row, int column);

    inline Matrix4 &LoadIdentity();

    double Determinant()
    {
        double det;
        det  = m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                          m[2][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) +
                          m[3][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]));
        det -= m[1][0] * (m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
                          m[2][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
                          m[3][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2]));
        det += m[2][0] * (m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
                          m[1][1] * (m[0][2] * m[3][3] - m[0][3] * m[3][2]) +
                          m[3][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2]));
        det -= m[3][0] * (m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
                          m[1][1] * (m[0][2] * m[2][3] - m[0][3] * m[2][2]) +
                          m[2][1] * (m[0][2] * m[1][3] - m[0][3] * m[1][2]));

        return det;
    }

    Matrix4& operator +=(const Matrix4& o)
    {
        // First column
        m[0][0] += o.m[0][0];
        m[0][1] += o.m[0][1];
        m[0][2] += o.m[0][2];
        m[0][3] += o.m[0][3];

        // Second column
        m[1][0] += o.m[1][0];
        m[1][1] += o.m[1][1];
        m[1][2] += o.m[1][2];
        m[1][3] += o.m[1][3];

        // Third column
        m[2][0] += o.m[2][0];
        m[2][1] += o.m[2][1];
        m[2][2] += o.m[2][2];
        m[2][3] += o.m[2][3];

        // Fourth column
        m[3][0] += o.m[3][0];
        m[3][1] += o.m[3][1];
        m[3][2] += o.m[3][2];
        m[3][3] += o.m[3][3];

        // Update matrix type
        type = UNKNOWN;

        return *this;
    }

    Matrix4& operator -=(const Matrix4& o)
    {
        // First column
        m[0][0] -= o.m[0][0];
        m[0][1] -= o.m[0][1];
        m[0][2] -= o.m[0][2];
        m[0][3] -= o.m[0][3];

        // Second column
        m[1][0] -= o.m[1][0];
        m[1][1] -= o.m[1][1];
        m[1][2] -= o.m[1][2];
        m[1][3] -= o.m[1][3];

        // Third column
        m[2][0] -= o.m[2][0];
        m[2][1] -= o.m[2][1];
        m[2][2] -= o.m[2][2];
        m[2][3] -= o.m[2][3];

        // Fourth column
        m[3][0] -= o.m[3][0];
        m[3][1] -= o.m[3][1];
        m[3][2] -= o.m[3][2];
        m[3][3] -= o.m[3][3];

        // Update matrix type
        type = UNKNOWN;

        return *this;
    }

    Matrix4& operator *=(const Matrix4& o)
    {
        // Do not need to compute multiplication
        // if one matrix is identity
        if (type == IDENTITY) {
            *this = o;
            return (*this);
        }
        else if (o.type == IDENTITY)
        {
            return (*this);
        }
        else
        {
            Matrix4 tmp(false);

            // First column
            tmp.m[0][0] = m[0][0] * o.m[0][0] + m[1][0] * o.m[0][1] + m[2][0] * o.m[0][2] + m[3][0] * o.m[0][3];
            tmp.m[0][1] = m[0][1] * o.m[0][0] + m[1][1] * o.m[0][1] + m[2][1] * o.m[0][2] + m[3][1] * o.m[0][3];
            tmp.m[0][2] = m[0][2] * o.m[0][0] + m[1][2] * o.m[0][1] + m[2][2] * o.m[0][2] + m[3][2] * o.m[0][3];
            tmp.m[0][3] = m[0][3] * o.m[0][0] + m[1][3] * o.m[0][1] + m[2][3] * o.m[0][2] + m[3][3] * o.m[0][3];

            // Second column
            tmp.m[1][0] = m[0][0] * o.m[1][0] + m[1][0] * o.m[1][1] + m[2][0] * o.m[1][2] + m[3][0] * o.m[1][3];
            tmp.m[1][1] = m[0][1] * o.m[1][0] + m[1][1] * o.m[1][1] + m[2][1] * o.m[1][2] + m[3][1] * o.m[1][3];
            tmp.m[1][2] = m[0][2] * o.m[1][0] + m[1][2] * o.m[1][1] + m[2][2] * o.m[1][2] + m[3][2] * o.m[1][3];
            tmp.m[1][3] = m[0][3] * o.m[1][0] + m[1][3] * o.m[1][1] + m[2][3] * o.m[1][2] + m[3][3] * o.m[1][3];


            // Third column
            tmp.m[2][0] = m[0][0] * o.m[2][0] + m[1][0] * o.m[2][1] + m[2][0] * o.m[2][2] + m[3][0] * o.m[2][3];
            tmp.m[2][1] = m[0][1] * o.m[2][0] + m[1][1] * o.m[2][1] + m[2][1] * o.m[2][2] + m[3][1] * o.m[2][3];
            tmp.m[2][2] = m[0][2] * o.m[2][0] + m[1][2] * o.m[2][1] + m[2][2] * o.m[2][2] + m[3][2] * o.m[2][3];
            tmp.m[2][3] = m[0][3] * o.m[2][0] + m[1][3] * o.m[2][1] + m[2][3] * o.m[2][2] + m[3][3] * o.m[2][3];

            // Fourth column
            tmp.m[3][0] = m[0][0] * o.m[3][0] + m[1][0] * o.m[3][1] + m[2][0] * o.m[3][2] + m[3][0] * o.m[3][3];
            tmp.m[3][1] = m[0][1] * o.m[3][0] + m[1][1] * o.m[3][1] + m[2][1] * o.m[3][2] + m[3][1] * o.m[3][3];
            tmp.m[3][2] = m[0][2] * o.m[3][0] + m[1][2] * o.m[3][1] + m[2][2] * o.m[3][2] + m[3][2] * o.m[3][3];
            tmp.m[3][3] = m[0][3] * o.m[3][0] + m[1][3] * o.m[3][1] + m[2][3] * o.m[3][2] + m[3][3] * o.m[3][3];

            // Update matrix type
            type = UNKNOWN;
            *this = tmp;

            return (*this);
        }
    }

    Matrix4& operator *=(scale s)
    {
        // First column
        m[0][0] *= s;
        m[0][1] *= s;
        m[0][2] *= s;
        m[0][3] *= s;

        // Second column
        m[1][0] *= s;
        m[1][1] *= s;
        m[1][2] *= s;
        m[1][3] *= s;

        // Third column
        m[2][0] *= s;
        m[2][1] *= s;
        m[2][2] *= s;
        m[2][3] *= s;

        // Fourth column
        m[3][0] *= s;
        m[3][1] *= s;
        m[3][2] *= s;
        m[3][3] *= s;

        // Update matrix type
        type = UNKNOWN;

        return *this;
    }

    Matrix4& operator /=(scale s)
    {
        // First column
        m[0][0] /= s;
        m[0][1] /= s;
        m[0][2] /= s;
        m[0][3] /= s;

        // Second column
        m[1][0] /= s;
        m[1][1] /= s;
        m[1][2] /= s;
        m[1][3] /= s;

        // Third column
        m[2][0] /= s;
        m[2][1] /= s;
        m[2][2] /= s;
        m[2][3] /= s;

        // Fourth column
        m[3][0] /= s;
        m[3][1] /= s;
        m[3][2] /= s;
        m[3][3] /= s;

        // Update matrix type
        type = UNKNOWN;

        return *this;
    }


    Matrix4& operator =(const Matrix4& o)
    {
        // First column
        m[0][0] = o.m[0][0];
        m[0][1] = o.m[0][1];
        m[0][2] = o.m[0][2];
        m[0][3] = o.m[0][3];

        // Second column
        m[1][0] = o.m[1][0];
        m[1][1] = o.m[1][1];
        m[1][2] = o.m[1][2];
        m[1][3] = o.m[1][3];

        // Third column
        m[2][0] = o.m[2][0];
        m[2][1] = o.m[2][1];
        m[2][2] = o.m[2][2];
        m[2][3] = o.m[2][3];

        // Fourth column
        m[3][0] = o.m[3][0];
        m[3][1] = o.m[3][1];
        m[3][2] = o.m[3][2];
        m[3][3] = o.m[3][3];

        // Update matrix type
        type = o.type;

        return *this;
    }

    bool operator==(const Matrix4 &o)
    {
        if (type != o.type)
            return false;
        for (uint r = 0; r < 4; r++)
            for (uint c = 0; c < 4; c++)
                if (m[r][c] != o.m[r][c])
                    return false;
        return true;
    }

    bool operator!=(const Matrix4 &o)
    {
        return !operator==(o);
    }

    Matrix4 &Translate(coord tx, coord ty, coord tz)
    {
        if (type == IDENTITY)
        {
            // If identity, just need to affect the translation vector
            m[3][0] = tx;
            m[3][1] = ty;
            m[3][2] = tz;
            type = TRANSLATION;
        }
        else if (type == TRANSLATION)
        {
            // If translation, just need to add to the previous vector
            m[3][0] += tx;
            m[3][1] += ty;
            m[3][2] += tz;
        }
        else if (type == SCALING)
        {
            // If scaling, just need to multiply with the previous vector
            m[3][0] = m[0][0] * tx;
            m[3][1] = m[1][1] * ty;
            m[3][2] = m[2][2] * tz;
            type |= TRANSLATION;
        }
        else if (type == (SCALING | TRANSLATION))
        {
            // If scaling, multiply with to the previous vector then add to this one
            m[3][0] += m[0][0] * tx;
            m[3][1] += m[1][1] * ty;
            m[3][2] += m[2][2] * tz;
        }
        else
        {
            m[3][0] += m[0][0] * tx + m[1][0] * ty + m[2][0] * tz;
            m[3][1] += m[0][1] * tx + m[1][1] * ty + m[2][1] * tz;
            m[3][2] += m[0][2] * tx + m[1][2] * ty + m[2][2] * tz;
            m[3][3] += m[0][3] * tx + m[1][3] * ty + m[2][3] * tz;

            if (type == ROTATION)
                type |= TRANSLATION;
            else if (type != (ROTATION | TRANSLATION))
                type = UNKNOWN;
        }
        return *this;
    }

    Matrix4 &Translate(Vector3& v)
    {
        return Translate(v.x, v.y, v.z);
    }

    Matrix4 &Scale(coord sx, coord sy, coord sz)
    {
        if (type == IDENTITY)
        {
            // If identity, just need to affect the scaling vector
            m[0][0] = sx;
            m[1][1] = sy;
            m[2][2] = sz;
            type = SCALING;
        }
        else if (type == TRANSLATION)
        {
            // If translation, just need to affect the scaling vector
            m[0][0] = sx;
            m[1][1] = sy;
            m[2][2] = sz;
            type |= SCALING;
        }
        else if (type == SCALING || type == (SCALING | TRANSLATION))
        {
            // If scaling, just need to multiply to the previous
            m[0][0] *= sx;
            m[1][1] *= sy;
            m[2][2] *= sz;
        }
        else if(type == ROTATION)
        {
            m[0][0] *= sx; m[0][1] *= sx; m[0][2] *= sx;
            m[1][0] *= sy; m[1][1] *= sy; m[1][2] *= sy;
            m[2][0] *= sz; m[2][1] *= sz; m[2][2] *= sz;
        }
        else
        {
            m[0][0] *= sx; m[0][1] *= sx; m[0][2] *= sx; m[0][3] *= sx;
            m[1][0] *= sy; m[1][1] *= sy; m[1][2] *= sy; m[1][3] *= sy;
            m[2][0] *= sz; m[2][1] *= sz; m[2][2] *= sz; m[2][3] *= sz;
            type = UNKNOWN;
        }
        return *this;
    }

    Matrix4 &Scale(Vector3 v)
    {
        return Scale(v.x, v.y, v.z);
    }

    Matrix4 &Rotate(coord a, coord rx, coord ry, coord rz)
    {
        double length = sqrt(rx * rx + ry * ry + rz * rz);

        // Rotate if only the axis is not null
        if (length > 0.0)
        {
           double ca = cos(a * M_PI / 180.0);
           double sa = sin(a * M_PI / 180.0);
           double ica = 1.0 - ca;
           coord xx, yy, zz, xy, yz, zx, xs, ys, zs;
           Matrix4 rot(false);

           // Normalize axis
           rx /= length;
           ry /= length;
           rz /= length;

           xx = rx * rx;
           yy = ry * ry;
           zz = rz * rz;
           xy = rx * ry;
           yz = ry * rz;
           zx = rz * rx;
           xs = rx * sa;
           ys = ry * sa;
           zs = rz * sa;

           // First column
           rot.m[0][0] = (ica * xx) + ca;
           rot.m[1][0] = (ica * xy) - zs;
           rot.m[2][0] = (ica * zx) + ys;
           rot.m[3][0] = 0.0;

           // Second column
           rot.m[0][1] = (ica * xy) + zs;
           rot.m[1][1] = (ica * yy) + ca;
           rot.m[2][1] = (ica * yz) - xs;
           rot.m[3][1] = 0.0;

           // Third column
           rot.m[0][2] = (ica * zx) - ys;
           rot.m[1][2] = (ica * yz) + xs;
           rot.m[2][2] = (ica * zz) + ca;
           rot.m[3][2] = 0.0;

           // Fourth column
           rot.m[0][3] = 0.0;
           rot.m[1][3] = 0.0;
           rot.m[2][3] = 0.0;
           rot.m[3][3] = 1.0;

           int current = type;
           *this *= rot;

           // Update matrix type
           if (current == IDENTITY)
               type = ROTATION;
           else if(current != UNKNOWN)
               type = current | ROTATION;
        }
        return *this;
    }

    Matrix4 &Rotate(coord a, Vector3& v)
    {
        return Rotate(a, v.x, v.y, v.z);
    }

    Matrix4 &Rotate(Quaternion& q)
    {
        // Rotate if only the quaternion is not null
        if (q.Length() > 0.0)
        {
            Matrix4 rot(false);
            coord xx, yy, zz, xy, yz, zx, xs, ys, zs;

            xx = q.vector.x * q.vector.x;
            yy = q.vector.y * q.vector.y;
            zz = q.vector.z * q.vector.z;
            xy = q.vector.x * q.vector.y;
            yz = q.vector.y * q.vector.z;
            zx = q.vector.z * q.vector.x;
            xs = q.vector.x * q.scalar;
            ys = q.vector.y * q.scalar;
            zs = q.vector.z * q.scalar;

            // First column
            rot.m[0][0] = 1.0 - 2.0 * (yy + zz);
            rot.m[1][0] = 2.0 * (xy - zs);
            rot.m[2][0] = 2.0 * (zx + ys);
            rot.m[3][0] = 0.0;

            // Second column
            rot.m[0][1] = 2.0 * (xy + zs);
            rot.m[1][1] = 1.0 - 2.0 * (xx + zz);
            rot.m[2][1] = 2.0 * (yz - xs);
            rot.m[3][1] = 0.0;

            // Third column
            rot.m[0][2] = 2.0 * (zx - ys);
            rot.m[1][2] = 2.0 * (yz + xs);
            rot.m[2][2] = 1.0 - 2.0 * (xx + yy);
            rot.m[3][2] = 0.0;

            // Fourth column
            rot.m[0][3] = 0.0;
            rot.m[1][3] = 0.0;
            rot.m[2][3] = 0.0;
            rot.m[3][3] = 1.0;

            int current = type;
            *this *= rot;

            // Update matrix type
            if (current == IDENTITY)
                type = ROTATION;
            else
                if(current != UNKNOWN)
                    type = current | ROTATION;
        }
        return *this;
    }

public:
    enum {
        UNKNOWN         = 0,   // Unknown matrix
        TRANSLATION     = 1,   // Simple translation matrix
        SCALING         = 2,   // Simple scaling matrix
        ROTATION        = 4,   // Simple rotation matrix
        IDENTITY        = 7    // Identity matrix (TRANSLATION|SCALING|ROTATION)
    };
    int       type;

private:
    coord     m[4][4];

};


// ============================================================================
//
//   Inline Matrix operations not defined in class
//
// ============================================================================

inline Matrix4 &Matrix4::LoadIdentity()
{
    if(type != IDENTITY)
    {
        m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
        m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
        m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
        m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;

        // Update matrix type
        type = IDENTITY;
    }
    return *this;
}


inline coord& Matrix4::operator()(int row, int col)
{
    return m[col][row];
}


inline Matrix4 operator +(const Matrix4& l, const Matrix4 &r)
{
    Matrix4 result(l);
    result += r;
    return result;
}

inline Matrix4 operator -(const Matrix4& l, const Matrix4 &r)
{
    Matrix4 result(l);
    result -= r;
    return result;
}

inline Matrix4 operator *(const Matrix4& l, const Matrix4 &r)
{
    Matrix4 result(l);
    result *= r;
    return result;
}

inline Matrix4 operator *(const Matrix4& l, scale s)
{
    Matrix4 result(l);
    result *= s;
    return result;
}

inline Matrix4 operator *(scale s, const Matrix4& l)
{
    Matrix4 result(l);
    result *= s;
    return result;
}

inline Matrix4 operator /(const Matrix4& l, scale s)
{
    Matrix4 result(l);
    result /= s;
    return result;
}

inline std::ostream& operator<< (std::ostream &out, Matrix4 m)
// ----------------------------------------------------------------------------
//   Display a Matrix 4x4
// ----------------------------------------------------------------------------
{
    out << "[";

    out << m(0, 0) << " "
        << m(1, 0) << " "
        << m(2, 0) << " "
        << m(3, 0) << std::endl;

    out << m(0, 1) << " "
        << m(1, 1) << " "
        << m(2, 1) << " "
        << m(3, 1) << std::endl;

    out << m(0, 2) << " "
        << m(1, 2) << " "
        << m(2, 2) << " "
        << m(3, 2) << std::endl;

    out << m(0, 3) << " "
        << m(1, 3) << " "
        << m(2, 3) << " "
        << m(3, 3);

    out << "]" << std::endl;

    return out;
}


TAO_END

#endif // MATRIX_H
