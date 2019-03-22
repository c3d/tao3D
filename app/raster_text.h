#ifndef RASTER_TEXT_H
#define RASTER_TEXT_H
// *****************************************************************************
// raster_text.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    Show a 2D text overlay using a simple bitmap font. Text position is
//    defined in 3D.
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
// (C) 2011-2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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


#include "tao.h"
#include "tao_gl.h"
#include "coords.h"
#include <QMap>


class QGLContext;

TAO_BEGIN

struct RasterText
// ----------------------------------------------------------------------------
//   Display bitmap text in the GL view
// ----------------------------------------------------------------------------
{
    // Print text at current position
    static int printf(const char* format, ...);
    // Set current position (2D window coordinates)
    static void moveTo(coord x, coord y);
    // Purge instance associated to GL context
    static void purge(const QGLContext *context);

private:
    RasterText();
    ~RasterText();

    void                makeRasterFont();
    static RasterText * instance();

private:
    static struct Cleanup
    // ------------------------------------------------------------------------
    //   Automatically delete the RasterText instance
    // ------------------------------------------------------------------------
    {
        ~Cleanup()
        {
            foreach (RasterText *p, instances)
                delete p;
        }
    } cleanup;

private:
    GLuint              fontOffset;
    Point               pos;

private:
    static QMap<const QGLContext *, RasterText *> instances;
};

TAO_END

#endif // RASTER_TEXT_H
