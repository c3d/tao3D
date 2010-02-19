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

ImageTextureInfo::ImageTextureInfo()
// ----------------------------------------------------------------------------
//   Prepare to record texture IDs for the various images
// ----------------------------------------------------------------------------
    : textures()
{}


ImageTextureInfo::~ImageTextureInfo()
// ----------------------------------------------------------------------------
//   Release the GL texture
// ----------------------------------------------------------------------------
{
    texture_map::iterator i;
    glDisable(GL_TEXTURE_2D);
    for (i = textures.begin(); i != textures.end(); i++)
        glDeleteTextures(1, &(*i).second);
}


void ImageTextureInfo::bind(text file)
// ----------------------------------------------------------------------------
//   Bind the given GL texture
// ----------------------------------------------------------------------------
{
    GLuint textureId = textures[file];
    if (textureId == 0)
    {
        // Prune the map if it gets too big
        while (textures.size() > MAX_TEXTURES)
            textures.erase(textures.begin());

        // Read the image file and convert to proper GL image format
        QImage original(QString::fromStdString(file));
        QImage texture = QGLWidget::convertToGLFormat(original);

        // Generate the GL texture
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
                     texture.width(), texture.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());

        // Remember the texture for next time
        textures[file] = textureId;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
}



// ============================================================================
// 
//    PageInfo: representation of a page
// 
// ============================================================================

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


void PageInfo::resize(uint width, uint height)
// ----------------------------------------------------------------------------
//   Change the size of the frame buffer used for rendering
// ----------------------------------------------------------------------------
{
    // Delete anything we might have
    if (texture_fbo)
        delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;

    // Round up width and height to the next power of two
    uint w = 32, h = 32;
    while (w < width)
        w *= 2;
    while (h < height)
        h *= 2;

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
}


void PageInfo::begin()
// ----------------------------------------------------------------------------
//   Begin rendering in the given off-screen buffer
// ----------------------------------------------------------------------------
{
    // Clear the render FBO
    render_fbo->bind();

    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void PageInfo::end()
// ----------------------------------------------------------------------------
//   Finish rendering in an off-screen buffer
// ----------------------------------------------------------------------------
{
    render_fbo->release();

    // Blit the result in the texture if necessary
    if (render_fbo != texture_fbo)
    {
        QRect rect(0, 0, render_fbo->width(), render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(texture_fbo, rect,
                                              render_fbo, rect);
    }    
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
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
}



// ============================================================================
// 
//   PagePainter
// 
// ============================================================================

struct PagePainter : QPainter
// ----------------------------------------------------------------------------
//   Paint on a given page, given as a PageInfo
// ----------------------------------------------------------------------------
//   A PagePainter structure is a transient rendering mechanism for a PageInfo
{
    PagePainter(PageInfo *info);
    ~PagePainter();
    PageInfo *info;
    GLStateKeeper save;
};


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


SvgRendererInfo::SvgRendererInfo(QGLWidget *w, uint width, uint height)
// ----------------------------------------------------------------------------
//   Create a renderer with the right size
// ----------------------------------------------------------------------------
    : PageInfo(width, height), widget(w)
{}


SvgRendererInfo::~SvgRendererInfo()
// ----------------------------------------------------------------------------
//   When deleting the info, delete all renderers we have
// ----------------------------------------------------------------------------
{
    renderer_map::iterator i;
    glDisable(GL_TEXTURE_2D);
    for (i = renderers.begin(); i != renderers.end(); i++)
        delete (*i).second;
}


void SvgRendererInfo::bind (text file)
// ----------------------------------------------------------------------------
//    Activate a given SVG renderer
// ----------------------------------------------------------------------------
{
    QSvgRenderer *r = renderers[file];
    if (!r)
    {
        while (renderers.size() > MAX_TEXTURES)
        {
            renderer_map::iterator first = renderers.begin();
            delete (*first).second;
            renderers.erase(first);
        }

        r = new QSvgRenderer(QString::fromStdString(file), widget);
        r->connect(r, SIGNAL(repaintNeeded()), widget, SLOT(updateGL()));
        renderers[file] = r;
    }

    if (r)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_MULTISAMPLE);
        glDisable(GL_CULL_FACE);
        PagePainter painter(this);
        r->render(&painter);
    }

    PageInfo::bind();
}

TAO_END
