// ****************************************************************************
//  page.cpp                                                        XLR project
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

#include "page.h"
#include "gl_keepers.h"
#include <QtOpenGL>


TAO_BEGIN

PageInfo::PageInfo(uint w, uint h)
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects
// ----------------------------------------------------------------------------
    : render_fbo(NULL), texture_fbo(NULL)
{
    resize(w, h);
}


PageInfo::~PageInfo()
// ----------------------------------------------------------------------------
//   Delete the frame buffer object and GL tile
// ----------------------------------------------------------------------------
{
    delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;
}


void PageInfo::resize(uint w, uint h)
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
    {
        render_fbo = new QGLFramebufferObject(w, h);
        texture_fbo = render_fbo;
    }
    glShowErrors();
}


void PageInfo::begin()
// ----------------------------------------------------------------------------
//   Begin rendering in the given off-screen buffer
// ----------------------------------------------------------------------------
{
    // Clear the render FBO
    int ok = render_fbo->bind();
    if (!ok) std::cerr << "PageInfo::begin(): unexpected result\n";
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


void PageInfo::end()
// ----------------------------------------------------------------------------
//   Finish rendering in an off-screen buffer
// ----------------------------------------------------------------------------
{
    int ok = render_fbo->release();
    if (!ok) std::cerr << "PageInfo::end(): unexpected result\n";
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
    

void PageInfo::bind()
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
//   PagePainter
// 
// ============================================================================

PagePainter::PagePainter(PageInfo *info)
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


PagePainter::~PagePainter()
// ----------------------------------------------------------------------------
//   Finish the drawing on the current page
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
