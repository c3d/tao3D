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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "frame.h"
#include "gl_keepers.h"
#include "widget.h"
#include "application.h"
#include "tao_utf8.h"

TAO_BEGIN

FrameInfo::FrameInfo(uint w, uint h, GLenum f)
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects
// ----------------------------------------------------------------------------
    : w(w), h(h), format(f), depthTextureID(0),
      context(NULL), renderFBO(NULL), textureFBO(NULL),
      refreshTime(-1), clearColor(1, 1, 1, 0)
{}


FrameInfo::FrameInfo(const FrameInfo &o)
// ----------------------------------------------------------------------------
//   Copy constructor - Don't copy the framebuffers
// ----------------------------------------------------------------------------
    : XL::Info(o), w(o.w), h(o.h), format(o.format), depthTextureID(0),
      context(NULL), renderFBO(NULL), textureFBO(NULL),
      refreshTime(o.refreshTime), clearColor(o.clearColor)
{}


FrameInfo::~FrameInfo()
// ----------------------------------------------------------------------------
//   Delete the frame buffer objects and GL tiles
// ----------------------------------------------------------------------------
{
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
    
    // Delete anything we might have
    purge();
    
    // Select whether we draw directly in texture or blit to it
    // If we can blit and suceed in creating a multisample buffer,
    // we first draw in a multisample buffer
    // with 4 samples per pixel. This cannot be used directly as texture.
    QGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(this->format);
    format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        const int SAMPLES = 4;
        QGLFramebufferObjectFormat mformat(format);
        mformat.setSamples(SAMPLES);
        renderFBO = new QGLFramebufferObject(w, h, mformat);
        QGLFramebufferObjectFormat actualFormat = renderFBO->format();
        int samples = actualFormat.samples();
        if (samples == SAMPLES)
        {
            // REVISIT: we pass format to have a depth buffer attachment.
            // This is required only when we want to later use depthTexture().
            // TODO: specify at object creation?
            textureFBO = new QGLFramebufferObject(w, h, format);
        }
        else
        {
            // Multisample framebuffer objects are not supported.
            // Normally we could just do: textureFBO = renderFBO, to use
            // the FBO as a texture.
            // But on Windows/VMWare, even when samples == 0 the
            // FBO cannot be used directly as a texture.
            // 2 options: (1) create a textureFBO and blit as if MS was
            // enabled, or (2) re-create render FBO without asking for
            // multisampling. (2) is obviously better.
            delete renderFBO;
            renderFBO = new QGLFramebufferObject(w, h, format);
            textureFBO = renderFBO;
        }
    }
    else
    {
        renderFBO = new QGLFramebufferObject(w, h, format);
        textureFBO = renderFBO;
    }
    
    // depthTextureID is optional, resize if we have one
    if (depthTextureID)
        resizeDepthTexture(w, h);
    
    // Store what context we created things in
    this->w = w; this->h = h;
    this->context = (QGLContext *) QGLContext::currentContext();
    
    // Clear the contents of the newly created frame buffer
    int ok = renderFBO->bind();
    if (!ok) std::cerr << "FrameInfo::resize(): unexpected result\n";
    clear();
    renderFBO->release();
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
        glDeleteTextures(1, &depthTextureID);
    }

    // Create the depth texture
    glGenTextures(1, &depthTextureID);
    glBindTexture(GL_TEXTURE_2D, depthTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}


void FrameInfo::begin(bool clearContents)
// ----------------------------------------------------------------------------
//   Begin rendering in the given off-screen buffer
// ----------------------------------------------------------------------------
{
    // Clear the render FBO
    checkGLContext();
    int ok = renderFBO->bind();
    if (!ok) std::cerr << "FrameInfo::begin(): unexpected result\n";

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);

    if (clearContents)
        clear();
}


void FrameInfo::end()
// ----------------------------------------------------------------------------
//   Finish rendering in an off-screen buffer
// ----------------------------------------------------------------------------
{
    checkGLContext();

    // Blit the result in the texture FBO and in depth texture if necessary
    if (renderFBO != textureFBO)
        blit();
    if (depthTextureID)
        copyToDepthTexture();

    int ok = renderFBO->release();
    if (!ok) std::cerr << "FrameInfo::end(): unexpected result\n";
}


GLuint FrameInfo::bind()
// ----------------------------------------------------------------------------
//   Bind the GL texture associated to the off-screen buffer
// ----------------------------------------------------------------------------
{
    checkGLContext();
    GLuint texId = textureFBO->texture();
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        glEnable(GL_MULTISAMPLE);
    return texId;
}


void FrameInfo::clear()
// ----------------------------------------------------------------------------
//   Clear the contents of a frame buffer object
// ----------------------------------------------------------------------------
{
    glClearColor(clearColor.red, clearColor.green, clearColor.blue,
                 clearColor.alpha);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
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

    glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFBO->handle());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, textureFBO->handle());
    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height,
                      buffers, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void FrameInfo::purge()
// ----------------------------------------------------------------------------
//   Purge frame buffer objects and delete them
// ----------------------------------------------------------------------------
{
    if (context)
    {
        if (context != QGLContext::currentContext())
            context->makeCurrent();

        delete renderFBO;
        if (textureFBO != renderFBO)
            delete textureFBO;
        if (depthTextureID)
            glDeleteTextures(1, &depthTextureID);

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
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbname);
    glBindFramebuffer(GL_FRAMEBUFFER, textureFBO->handle());
    glBindTexture(GL_TEXTURE_2D, depthTextureID);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbname);
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
    return ((FrameInfo *)obj)->bind();
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
    glLoadIdentity();

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
