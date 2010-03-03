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
#include <QtOpenGL>
#include <cairo.h>


TAO_BEGIN

Frame::Frame(uint width, uint height)
// ----------------------------------------------------------------------------
//    Create a frame of the given size
// ----------------------------------------------------------------------------
    : width(width), height(height),
      surface(NULL), data(NULL), context(NULL),
      textureId(0)
{
    uint channels = 4;
    uint stride = channels * width;
    data = new byte[width * height * channels];
    surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32,
                                                  width, height, stride);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        throw &surface;

    context = cairo_create(surface);
    if (cairo_status(context) != CAIRO_STATUS_SUCCESS)
        throw &context;

    // Set some initial parameters
    cairo_set_operator(context, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(context, 0, 0, 0, 0);
    cairo_paint(context);

    // Generate the GL texture
    glGenTextures(1, &textureId);
}


Frame::~Frame()
// ----------------------------------------------------------------------------
//   Delete the Cairo resources
// ----------------------------------------------------------------------------
{
    glDeleteTextures(1, &textureId);

    if (context)
        cairo_destroy(context);
    if (surface)
        cairo_surface_destroy(surface);
    if (data)
        delete[] data;    
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
    cairo_paint(context);
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
    cairo_show_text(context, s.c_str());
}


void Frame::Rectangle(double x, double y, double w, double h)
// ----------------------------------------------------------------------------
//   Paint a rectangle using Cairo
// ----------------------------------------------------------------------------
{
    cairo_rectangle(context, x, y, w, h);
}


void Frame::Bind()
// ----------------------------------------------------------------------------
//    Bind to the GL texture
// ----------------------------------------------------------------------------
{
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
}


void Frame::Paint(double x, double y, double w, double h)
// ----------------------------------------------------------------------------
//    Paint the resulting texture over the given rectangle
// ----------------------------------------------------------------------------
{
    Bind();
    glColor4f(1,1,1,1);
    glBegin(GL_QUADS);
    {
        glVertex2f(x,   y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x,   y+h);
    }
    glEnd();
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
    glDisable(GL_CULL_FACE);

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
    glEnable(GL_CULL_FACE);
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
    glClearColor(0.0, 0.0, 0.3, 0.0);
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
