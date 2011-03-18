#ifndef RASTER_TEXT_H
#define RASTER_TEXT_H
// ****************************************************************************
//  raster_text.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    Show a 2D text overlay using a simple bitmap font. Text position is
//    defined in 3D.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************


#include "tao.h"
#include "tao_gl.h"
#include "coords.h"


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

private:
    RasterText();
    ~RasterText();

    void makeRasterFont();

private:
    static struct Cleanup
    // ------------------------------------------------------------------------
    //   Automatically delete the RasterText instance
    // ------------------------------------------------------------------------
    {
        ~Cleanup()
        {
            if (RasterText::instance)
                delete RasterText::instance;
        }
    } cleanup;

private:
    GLuint              fontOffset;
    Point               pos;

private:
    static RasterText * instance;
};

TAO_END

#endif // RASTER_TEXT_H
