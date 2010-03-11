#ifndef FRAME_H
#define FRAME_H
// ****************************************************************************
//  frame.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//     Frames are 2D drawing surfaces, most often rectangular
//     Frames can be placed in other frames in a hierarchy.
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

#include "tao.h"
#include "tree.h"
#include "gl_keepers.h"
#include <map>
#include <QtOpenGL>
#include <cairo.h>
#include <pango/pango.h>

TAO_BEGIN

struct Frame : XL::Info
// ----------------------------------------------------------------------------
//   A 2D (rectangular) drawing structure
// ----------------------------------------------------------------------------
{
    Frame();
    ~Frame();

    // Setup
    void             Resize(uint w, uint h);

    // Drawing
    void             Color(double red, double grn, double blu, double alpha=1);
    void             Clear();
    void             MoveTo(double x, double y);
    void             Font(text s);
    void             FontSize(double s);
    void             Text(text s);
    void             Rectangle(double x, double y, double w, double h);
    void             Stroke();
    void             LayoutMarkup(text s);
    void             LayoutText(text s);

    // End of drawing, paint the frame
    void             Paint(double x, double y, double w, double h);

private:
    cairo_surface_t      *surface;
    cairo_t              *context;
    PangoLayout          *layout;
    PangoFontDescription *font;

    // No support for copying frames around
    Frame(const Frame &o) {}
};


struct FrameInfo : XL::Info
// ----------------------------------------------------------------------------
//    Information about a given frame being rendered in a dynamic texture
// ----------------------------------------------------------------------------
//    A FrameInfo structure tends to be permanently attached to a tree
//    to record information that we don't want to re-create / destroy
//    at each and every evaluation of the tree
{
    typedef FrameInfo *data_t;

    FrameInfo(uint width = 512, uint height = 512);
    ~FrameInfo();

    void resize(uint width, uint height);
    void begin();
    void end();
    void bind();

    QGLFramebufferObject *render_fbo;
    QGLFramebufferObject *texture_fbo;
};


struct FramePainter : QPainter
// ----------------------------------------------------------------------------
//   Paint on a given frame, given as a FrameInfo
// ----------------------------------------------------------------------------
//   A FramePainter structure is a transient rendering mechanism for a FrameInfo
{
    FramePainter(FrameInfo *info);
    ~FramePainter();
    FrameInfo *info;
    GLStateKeeper save;
};

TAO_END

#endif // FRAME_H
