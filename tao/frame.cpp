// ****************************************************************************
//  frame.cpp                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    Off-screen OpenGL rendering to a 2D texture. A 2D texture of the depth
//    map can also be returned.
//
//
//
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "frame.h"
#include "gl_keepers.h"
#include "widget.h"
#include "application.h"
#include "tao_utf8.h"
#include "preferences_pages.h"


TAO_BEGIN

FrameInfo::FrameInfo(uint w, uint h, GLenum f)
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects
// ----------------------------------------------------------------------------
    : w(w), h(h), format(f), depthTextureID(0),
      context(NULL), renderFBO(NULL), textureFBO(NULL),
      layout(NULL), clearColor(1, 1, 1, 0)
{
    IFTRACE(fbo)
        std::cerr << "[FrameInfo] Constructor " << this
                  << " size "<< w << "x" << h << "\n";
}


FrameInfo::FrameInfo(const FrameInfo &o)
// ----------------------------------------------------------------------------
//   Copy constructor - Don't copy the framebuffers
// ----------------------------------------------------------------------------
    : XL::Info(o), w(o.w), h(o.h), format(o.format), depthTextureID(0),
      context(NULL), renderFBO(NULL), textureFBO(NULL),
      layout(NULL), clearColor(o.clearColor)
{
    IFTRACE(fbo)
        std::cerr << "[FrameInfo] Copy " << this
                  << " from " << &o
                  << " size "<< w << "x" << h << "\n";
}


FrameInfo::~FrameInfo()
// ----------------------------------------------------------------------------
//   Delete the frame buffer objects and GL tiles
// ----------------------------------------------------------------------------
{
    IFTRACE(fbo)
        std::cerr << "[FrameInfo] Destructor " << this << "\n";
    delete layout;
    purge();
}


void FrameInfo::resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the frame buffer used for rendering
// ----------------------------------------------------------------------------
{
    assert(QGLContext::currentContext() ||
           !"FrameInfo::resize without an OpenGL context???");
    
    // Don't change anything if size stays the same
    if (renderFBO && renderFBO->width()==int(w) && renderFBO->height()==int(h))
        return;
    
    IFTRACE(fbo)
        std::cerr << "[FrameInfo] Resize " << w << "x" << h << "\n";

    GraphicSave *save = GL.Save();

    // Delete anything we might have
    purge();

    // If the size is too big, we shouldn't use multisampling, it crashes
    // on MacOSX
    const uint SAMPLES = 4;
    const uint maxTextureSize = GL.MaxTextureSize();
    bool canMultiSample = QGLFramebufferObject::hasOpenGLFramebufferBlit() &&
                          TaoApp->hasFBOMultisample;
    if (w >= maxTextureSize || h >= maxTextureSize)
    {
        IFTRACE(fbo)
            std::cerr << "[FrameInfo] Disable multisample, too big "
                      << maxTextureSize << "\n";
        canMultiSample = false;
    }

    // Select whether we draw directly in texture or blit to it
    // If we can blit and suceed in creating a multisample buffer,
    // we first draw in a multisample buffer
    // with 4 samples per pixel. This cannot be used directly as texture.
    QGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(this->format);

    // Enable mipmap for fbo
    if(PerformancesPage::texture2DMipmap())
        format.setMipmap(true);

    format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
    if (canMultiSample)
    {
        QGLFramebufferObjectFormat mformat(format);
        mformat.setSamples(SAMPLES);
        renderFBO = new QGLFramebufferObject(w, h, mformat);
        // REVISIT: we pass format to have a depth buffer attachment.
        // This is required only when we want to later use depthTexture().
        // TODO: specify at object creation?
        textureFBO = new QGLFramebufferObject(w, h, format);
    }
    else
    {
        renderFBO = new QGLFramebufferObject(w, h, format);
        textureFBO = renderFBO;
    }

    // The value of the min and mag filters of the underlying texture
    // are forced to GL_NEAREST by Qt, which are not the GL defaults.
    // => We must synchronize our cache or some later GL.TexParameter calls
    // may be ignored.
    GLint min, mag;
    GLuint tex = textureFBO->texture();
    glBindTexture(GL_TEXTURE_2D, tex);
    // Query the actual values to be safe
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &min);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &mag);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL.BindTexture(GL_TEXTURE_2D, tex);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    if(PerformancesPage::texture2DMipmap())
        GL.GenerateMipmap(GL_TEXTURE_2D);
    GL.Sync(STATE_textures);
    GL.BindTexture(GL_TEXTURE_2D, 0);

    // depthTextureID is optional, resize if we have one
    if (depthTextureID)
        resizeDepthTexture(w, h);
    
    // Store what context we created things in
    this->w = w; this->h = h;
    this->context = (QGLContext *) QGLContext::currentContext();
    IFTRACE(fbo)
        std::cerr << "[FrameInfo] Resized " << w << "x" << h
                  << " in " << context << "\n";

    GL.Restore(save);
    
    // Clear the contents of the newly created frame buffer
    begin(true);
    end();
}


void FrameInfo::resizeDepthTexture(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the depth texture
// ----------------------------------------------------------------------------
{
    if (depthTextureID)
    {
        if (this->w == w && this->h == h)
            return;

        // Delete current depth texture
        if (depthTextureID)
        {
            GL.DeleteTextures(1, &depthTextureID);
            depthTextureID = 0;
        }
    }

    // Create the depth texture
    GL.GenTextures(1, &depthTextureID);
    GL.BindTexture(GL_TEXTURE_2D, depthTextureID);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL.TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
                  GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}


void FrameInfo::begin(bool clearContents)
// ----------------------------------------------------------------------------
//   Begin rendering in the given off-screen buffer
// ----------------------------------------------------------------------------
{
    // Clear the render FBO
    checkGLContext();

    GL.Sync(STATE_textures);
    GL.Invalidate(STATE_viewport);
    int ok = renderFBO->bind();
    if (!ok) std::cerr << "FrameInfo::begin(): unexpected result\n";

    // Synchronize cached state
    GL.SetCached_bufferMode(GL_COLOR_ATTACHMENT0);
    GL.Disable(GL_STENCIL_TEST);

    if (clearContents)
        clear();
    else if (renderFBO != textureFBO)
        backBlit();
}


void FrameInfo::end()
// ----------------------------------------------------------------------------
//   Finish rendering in an off-screen buffer
// ----------------------------------------------------------------------------
{
    // Blit the result in the texture FBO and in depth texture if necessary
    checkGLContext();
    if (renderFBO != textureFBO)
        blit();
    if (depthTextureID)
        copyToDepthTexture();

    int ok = renderFBO->release();
    if (!ok) std::cerr << "FrameInfo::end(): unexpected result\n";

    // Synchronize cached state
    GLint buf;
    glGetIntegerv(GL_DRAW_BUFFER, &buf);
    GL.SetCached_bufferMode(buf);
}


GLuint FrameInfo::bind()
// ----------------------------------------------------------------------------
//   Bind the GL texture associated to the off-screen buffer
// ----------------------------------------------------------------------------
{
    checkGLContext();
    GLuint texId = textureFBO->texture();
    GL.BindTexture(GL_TEXTURE_2D, texId);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.Enable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        GL.Enable(GL_MULTISAMPLE);
    return texId;
}


void FrameInfo::clear()
// ----------------------------------------------------------------------------
//   Clear the contents of a frame buffer object
// ----------------------------------------------------------------------------
{
    GL.ClearColor(clearColor.red, clearColor.green, clearColor.blue,
                 clearColor.alpha);
    GL.Clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}


void FrameInfo::blit()
// ----------------------------------------------------------------------------
//   Blit from multisampled FBO to non-multisampled FBO
// ----------------------------------------------------------------------------
{
    assert (QGLContext::currentContext() == context ||
            !"FrameInfo::blit invoked with invalid GL context");
    assert (textureFBO != renderFBO ||
            !"FrameInfo::blit invoked without any need for it.");

    GLenum buffers = depthTextureID
        ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        : GL_COLOR_BUFFER_BIT;
    uint width = renderFBO->width();
    uint height = renderFBO->height();
    QRect rect(0, 0, width, height);
    QGLFramebufferObject::blitFramebuffer(textureFBO, rect,
                                          renderFBO, rect,
                                          buffers);
    if (PerformancesPage::texture2DMipmap())
    {
        GraphicSave* save = GL.Save();

        // Generate mipmap for fbo
        GL.Enable(GL_TEXTURE_2D);
        GL.BindTexture(GL_TEXTURE_2D, textureFBO->texture());
        GL.GenerateMipmap(GL_TEXTURE_2D);
        GL.Restore(save);
    }
}


void FrameInfo::backBlit()
// ----------------------------------------------------------------------------
//   Blit from multisampled FBO to non-multisampled FBO
// ----------------------------------------------------------------------------
{
    assert (QGLContext::currentContext() == context ||
            !"FrameInfo::backBlit invoked with invalid GL context");
    assert (textureFBO != renderFBO ||
            !"FrameInfo::backBlit invoked without any need for it.");

    GLenum buffers = depthTextureID
        ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        : GL_COLOR_BUFFER_BIT;
    uint width = renderFBO->width();
    uint height = renderFBO->height();
    QRect rect(0, 0, width, height);
    QGLFramebufferObject::blitFramebuffer(renderFBO, rect,
                                          textureFBO, rect,
                                          buffers);
}


void FrameInfo::purge()
// ----------------------------------------------------------------------------
//   Purge frame buffer objects and delete them
// ----------------------------------------------------------------------------
{
    if (context)
    {
        if (depthTextureID &&
            context == QGLContext::currentContext())
        {
            // Delete the texture only if the GL context has not changed.
            // If it has changed, do NOT try to restore the previous context as
            // it may have been invalidated (see #3017).
            GL.DeleteTextures(1, &depthTextureID);
        }

        // ~QGLFrameBufferObject checks the GL context so it's OK to delete
        // the pointers unconditionaly
        delete renderFBO;
        if (textureFBO != renderFBO)
        {
            GLuint tex = textureFBO->texture();
            delete textureFBO;
            GL.Cache.DeleteTextures(1, &tex);
        }

        IFTRACE(fbo)
            std::cerr << "[FrameInfo] Purged " << context << " current "
                      << QGLContext::currentContext() << "\n";
        renderFBO = textureFBO = NULL;
        depthTextureID = 0;
        context = NULL;
    }
}


GLuint FrameInfo::texture()
// ----------------------------------------------------------------------------
//   Return the GL texture associated to the off-screen buffer
// ----------------------------------------------------------------------------
{
    checkGLContext();
    return textureFBO->texture();
}


GLuint FrameInfo::depthTexture()
// ----------------------------------------------------------------------------
//   Return the GL texture associated to the depth buffer attachment
// ----------------------------------------------------------------------------
{
    checkGLContext();
    if (!depthTextureID)
    {
        resizeDepthTexture(w, h);
        if (renderFBO != textureFBO)
            blit();

        copyToDepthTexture();
    }

    return depthTextureID;
}


void FrameInfo::checkGLContext()
// ----------------------------------------------------------------------------
//   Make sure FBOs have been allocated for the current GL context
// ----------------------------------------------------------------------------
{
    if (context != QGLContext::currentContext())
    {
        IFTRACE(fbo)
            std::cerr << "[FrameInfo] Changing context from " << context
                      << " to " << QGLContext::currentContext()
                      << "\n";
        purge();
        resize(w, h);
    }
    assert(context == QGLContext::currentContext() ||
           !"FrameInfo::checkGLContext was unable to set GL context???");
}


QImage FrameInfo::toImage()
// ----------------------------------------------------------------------------
//   Return the frame buffer contents as a QImage
// ----------------------------------------------------------------------------
{
    checkGLContext();
    return renderFBO->toImage();
}


void FrameInfo::copyToDepthTexture()
// ----------------------------------------------------------------------------
//   Copy the textureFBO depth buffer into our depth texture
// ----------------------------------------------------------------------------
{
    GLint fbname = 0;
    GraphicSave* save = GL.Save();

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbname);
    glBindFramebuffer(GL_FRAMEBUFFER, textureFBO->handle());
    GL.BindTexture(GL_TEXTURE_2D, depthTextureID);
    GL.CopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbname);

    GL.Restore(save);
}


ModuleApi::fbo * FrameInfo::newFrameBufferObject(uint w, uint h)
// ----------------------------------------------------------------------------
//   Create a framebuffer object
// ----------------------------------------------------------------------------
{
    return (ModuleApi::fbo *)new FrameInfo(w, h);
}


ModuleApi::fbo * FrameInfo::newFrameBufferObjectWithFormat(uint w, uint h,
                                                           uint format)
// ----------------------------------------------------------------------------
//   Create a framebuffer object with a specified format
// ----------------------------------------------------------------------------
{
    return (ModuleApi::fbo *)new FrameInfo(w, h, format);
}


void FrameInfo::deleteFrameBufferObject(ModuleApi::fbo * obj)
// ----------------------------------------------------------------------------
//   Delete a framebuffer object
// ----------------------------------------------------------------------------
{
    delete (FrameInfo *)obj;
}


void FrameInfo::resizeFrameBufferObject(ModuleApi::fbo * obj,
                                        uint w, uint h)
// ----------------------------------------------------------------------------
//   Resize a framebuffer object
// ----------------------------------------------------------------------------
{
    ((FrameInfo *)obj)->resize(w, h);
}


void FrameInfo::bindFrameBufferObject(ModuleApi::fbo * obj)
// ----------------------------------------------------------------------------
//   Make framebuffer object the current rendering target
// ----------------------------------------------------------------------------
{
    ((FrameInfo *)obj)->begin();
}


void FrameInfo::releaseFrameBufferObject(ModuleApi::fbo * obj)
// ----------------------------------------------------------------------------
//   Stop rendering into framebuffer object
// ----------------------------------------------------------------------------
{
    ((FrameInfo *)obj)->end();
}


unsigned int FrameInfo::frameBufferObjectToTexture(ModuleApi::fbo * obj)
// ----------------------------------------------------------------------------
//   Make framebuffer available as a texture
// ----------------------------------------------------------------------------
{
    unsigned int tex = ((FrameInfo *)obj)->bind();
    return tex;
}


unsigned int FrameInfo::frameBufferAttachmentToTexture(ModuleApi::fbo * obj,
                                                       int attachment)
// ----------------------------------------------------------------------------
//   Make framebuffer attachment available as a texture
// ----------------------------------------------------------------------------
{
    FrameInfo *f = (FrameInfo *)obj;

    switch (attachment)
    {
    case GL_COLOR_ATTACHMENT0:
        return f->texture();
    case GL_DEPTH_ATTACHMENT:
        return f->depthTexture();
    default:
        break;
    }
    return 0;
}


void * FrameInfo::imageFromFrameBufferObject(ModuleApi::fbo * obj)
// ----------------------------------------------------------------------------
//   Return FBO as an Image
// ----------------------------------------------------------------------------
{
    QImage *img = new QImage(((FrameInfo *)obj)->toImage());
    return (void *)img;
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
    GL.LoadIdentity();
    GL.LoadMatrix();

    // Clear the render FBO
    info->begin(true);

    // Begin drawing in the render FBO
    begin(info->renderFBO);
}


FramePainter::~FramePainter()
// ----------------------------------------------------------------------------
//   Finish the drawing on the current frame
// ----------------------------------------------------------------------------
{
    end();

    // Blit into the texture as necesary
    info->end();
}

TAO_END



// ****************************************************************************
//
//    Code generation from frame.tbl
//
// ****************************************************************************

#include "graphics.h"
#include "opcodes.h"
#include "options.h"
#include "widget.h"
#include "types.h"
#include "drawing.h"
#include "layout.h"
#include "module_manager.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

#include "widget.h"

using namespace XL;

#include "opcodes_declare.h"
#include "frame.tbl"

namespace Tao
{

#include "frame.tbl"


void EnterFrames()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "frame.tbl"
}


void DeleteFrames()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "frame.tbl"
}

}
