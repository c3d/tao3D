// ****************************************************************************
//  frame.cpp                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    Off-screen OpenGL rendering to a 2D texture
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

#include "frame.h"
#include "gl_keepers.h"
#include "widget.h"
#include <cairo.h>
#include <cairo-gl.h>
#include <pango/pangocairo.h>


TAO_BEGIN

Frame::Frame()
// ----------------------------------------------------------------------------
//    Create a frame of the given size
// ----------------------------------------------------------------------------
    : surface(NULL), context(NULL), layout(NULL), font(NULL)
{
    GLStateKeeper save;

    surface = cairo_gl_surface_create_for_current_gl_context();
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo surface");

    context = cairo_create(surface);
    if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        XL::Ooops("Unable to create Cairo context");

    layout = pango_cairo_create_layout(context);
    font = pango_font_description_from_string("Sans Bold 12");
    pango_layout_set_font_description(layout, font);
}


Frame::~Frame()
// ----------------------------------------------------------------------------
//   Delete the Cairo resources
// ----------------------------------------------------------------------------
{
    if (context)
        cairo_destroy(context);
    if (surface)
        cairo_surface_destroy(surface);

    pango_font_description_free(font);
    g_object_unref(layout);
}


void Frame::Resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the frame
// ----------------------------------------------------------------------------
{
    cairo_gl_surface_set_size(surface, w, h);
    pango_layout_set_width(layout, w * PANGO_SCALE);
    pango_layout_set_height(layout, h * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
}


void Frame::Color (double red, double green, double blue, double alpha)
// ----------------------------------------------------------------------------
//    Select the color for the current context
// ----------------------------------------------------------------------------
{
    cairo_set_source_rgba(context, red, green, blue, alpha);
}



void Frame::Clear()
// ----------------------------------------------------------------------------
//    Paint the entire surface with a uniform color
// ----------------------------------------------------------------------------
{
    cairo_save(context);
    {
        cairo_set_operator(context, CAIRO_OPERATOR_SOURCE);
        cairo_paint(context);
    }
    cairo_restore(context);
}


void Frame::MoveTo(double x, double y)
// ----------------------------------------------------------------------------
//   Move the cursor to the given Cairo coordinates
// ----------------------------------------------------------------------------
{
    cairo_move_to(context, x, y);
}


void Frame::Font(text s)
// ----------------------------------------------------------------------------
//   Select a given font name
// ----------------------------------------------------------------------------
{
    pango_font_description_free(font);
    font = pango_font_description_from_string(s.c_str());
    pango_layout_set_font_description(layout, font);

    cairo_select_font_face(context, s.c_str(),
                           CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
}


void Frame::FontSize(double s)
// ----------------------------------------------------------------------------
//    Set the font size
// ----------------------------------------------------------------------------
{
    cairo_set_font_size(context, s);
}


void Frame::Text(text s)
// ----------------------------------------------------------------------------
//   Show the given text on the context
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save;
    cairo_scale(context, 1, -1);
    cairo_show_text(context, s.c_str());
    cairo_scale(context, 1, -1);
}


void Frame::Rectangle(double x, double y, double w, double h)
// ----------------------------------------------------------------------------
//   Paint a rectangle using Cairo
// ----------------------------------------------------------------------------
{
    cairo_rectangle(context, x, y, w, h);
}


void Frame::Stroke()
// ----------------------------------------------------------------------------
//    Stroke the current path
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save(GL_TEXTURE_BIT|GL_ENABLE_BIT|GL_CURRENT_BIT);
    cairo_stroke(context);
}

void Frame::StrokePreserve()
// ----------------------------------------------------------------------------
//    Stroke the current path and preserve it.
// ----------------------------------------------------------------------------
{
    cairo_stroke_preserve(context);
}

void Frame::Fill()
// ----------------------------------------------------------------------------
//    Fill the current path.
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save(GL_TEXTURE_BIT|GL_ENABLE_BIT|GL_CURRENT_BIT);
    cairo_fill(context);

}
void Frame::FillPreserve()
// ----------------------------------------------------------------------------
//    Fill the current path and preserve it.
// ----------------------------------------------------------------------------
{
    cairo_fill_preserve(context);
}

void Frame::LayoutText(text s)
// ----------------------------------------------------------------------------
//   Add text to a layout
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save;
    glDisable(GL_DEPTH_TEST);
    cairo_scale(context, 1, -1);
    pango_layout_set_text(layout, s.c_str(), s.length());
    pango_cairo_update_layout(context, layout);
    pango_cairo_show_layout(context, layout);
    // pango_cairo_layout_path(context, layout);
    // cairo_stroke_preserve(context);
    // cairo_fill(context);
    cairo_scale(context, 1, -1);
}


void Frame::LayoutMarkup(text s)
// ----------------------------------------------------------------------------
//   Add text to a layout using the Pango markup language
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save;
    glDisable(GL_DEPTH_TEST);
    cairo_scale(context, 1, -1);
    pango_layout_set_markup(layout, s.c_str(), s.length());
    pango_cairo_update_layout(context, layout);
    pango_cairo_show_layout(context, layout);
    // pango_cairo_layout_path(context, layout);
    // cairo_stroke_preserve(context);
    // cairo_fill(context);
    cairo_scale(context, 1, -1);
}


void Frame::Paint(double x, double y, double w, double h)
// ----------------------------------------------------------------------------
//    Paint the resulting texture over the given rectangle
// ----------------------------------------------------------------------------
{
    cairo_surface_flush(surface);
}

void Frame::Arc(double xCenter,
                double yCenter,
                double radius,
                double angleStart,
                double angleStop,
                bool   isPositive)
// ----------------------------------------------------------------------------
//   Add an arc to the current path.
// ----------------------------------------------------------------------------
{
    if (isPositive)
        cairo_arc(context, xCenter, yCenter, radius, angleStart, angleStop);
    else
        cairo_arc_negative(context, xCenter, yCenter, radius, angleStart, angleStop);

}

void Frame::CurveTo(double x1,
                    double y1,
                    double x2,
                    double y2,
                    double x3,
                    double y3)
// ----------------------------------------------------------------------------
//   Add a curve to the current path.
// ----------------------------------------------------------------------------
{
    cairo_curve_to(context, x1, y1, x2, y2, x3, y3);
}

void Frame::LineTo(double x,
                   double y)
// ----------------------------------------------------------------------------
//   Add a line to the current path.
// ----------------------------------------------------------------------------
{
    cairo_line_to( context, x, y);
}

void Frame::ClosePath()
{
    cairo_close_path(context);
}
void Frame::CleanPath()
// ----------------------------------------------------------------------------
//    Reset the current path.
// ----------------------------------------------------------------------------
{
    cairo_new_path(context);
}

// ============================================================================
//
//   FrameInfo class
//
// ============================================================================

FrameInfo::FrameInfo(uint w, uint h)
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects
// ----------------------------------------------------------------------------
    : render_fbo(NULL), texture_fbo(NULL)
{
    resize(w, h);
}


FrameInfo::~FrameInfo()
// ----------------------------------------------------------------------------
//   Delete the frame buffer object and GL tile
// ----------------------------------------------------------------------------
{
    delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;
}


void FrameInfo::resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the frame buffer used for rendering
// ----------------------------------------------------------------------------
{
    // Don't change anything if size stays the same
    if (render_fbo && render_fbo->width() == w && render_fbo->height() == h)
        return;

    // Delete anything we might have
    if (texture_fbo)
        delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;

    // Select whether we draw directly in texture or blit to it
    // If we can blit, we first draw in a multisample buffer
    // with 4 samples per pixel. This cannot be used directly as texture.
#ifndef CONFIG_MINGW
    // FrameBuffer objects don't work well under VMware, although
    // they seem to work native
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        QGLFramebufferObjectFormat format;
#ifndef CONFIG_LINUX
        // Setting this crashes in the first framebuffer object ctor
        format.setSamples(4);
#endif
        format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

        render_fbo = new QGLFramebufferObject(w, h, format);
        texture_fbo = new QGLFramebufferObject(w, h);
    }
    else
#endif // CONFIG_MINGW
    {
        render_fbo = new QGLFramebufferObject(w, h);
        texture_fbo = render_fbo;
    }
    glShowErrors();
}


void FrameInfo::begin()
// ----------------------------------------------------------------------------
//   Begin rendering in the given off-screen buffer
// ----------------------------------------------------------------------------
{
    // Clear the render FBO
    int ok = render_fbo->bind();
    if (!ok) std::cerr << "FrameInfo::begin(): unexpected result\n";
    glShowErrors();

    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
#ifdef GL_MULTISAMPLE   // Not supported on Windows
    glDisable(GL_MULTISAMPLE);
#endif

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void FrameInfo::end()
// ----------------------------------------------------------------------------
//   Finish rendering in an off-screen buffer
// ----------------------------------------------------------------------------
{
    int ok = render_fbo->release();
    if (!ok) std::cerr << "FrameInfo::end(): unexpected result\n";
    glShowErrors();

    // Blit the result in the texture if necessary
    if (render_fbo != texture_fbo)
    {
        QRect rect(0, 0, render_fbo->width(), render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(texture_fbo, rect,
                                              render_fbo, rect);
    }
    glShowErrors();
}


void FrameInfo::bind()
// ----------------------------------------------------------------------------
//   Bind the GL texture associated to the off-screen buffer
// ----------------------------------------------------------------------------
{
    glBindTexture(GL_TEXTURE_2D, texture_fbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
#ifdef GL_MULTISAMPLE   // Not supported on Windows
    glEnable(GL_MULTISAMPLE);
#endif
}



// ============================================================================
//
//   FramePainter
//
// ============================================================================

FramePainter::FramePainter(FrameInfo *info)
// ----------------------------------------------------------------------------
//   Begin drawing in the current context
// ----------------------------------------------------------------------------
    : QPainter(), info(info), save()
{
    // Draw without any transformation (reset the coordinates system)
    glLoadIdentity();

    // Clear the render FBO
    info->render_fbo->bind();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    info->render_fbo->release();

    begin(info->render_fbo);
}


FramePainter::~FramePainter()
// ----------------------------------------------------------------------------
//   Finish the drawing on the current frame
// ----------------------------------------------------------------------------
{
    end();

    // Blit the result in the texture if necessary
    if (info->render_fbo != info->texture_fbo)
    {
        QRect rect(0,0,info->render_fbo->width(),info->render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(info->texture_fbo, rect,
                                              info->render_fbo, rect);
    }
}

TAO_END
