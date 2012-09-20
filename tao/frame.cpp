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


TAO_BEGIN

FrameInfo::FrameInfo(uint w, uint h, GLenum f)
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects
// ----------------------------------------------------------------------------
    : w(w), h(h), format(f), refreshTime(-1), clearColor(1, 1, 1, 0)
{
    resize(w, h);
}


FrameInfo::FrameInfo(const FrameInfo &o)
// ----------------------------------------------------------------------------
//   Copy constructor - Don't copy the framebuffers
// ----------------------------------------------------------------------------
    : XL::Info(o), w(o.w), h(o.h), format(o.format), refreshTime(o.refreshTime),
      clearColor(o.clearColor)
{
    resize(w, h);
}


FrameInfo::~FrameInfo()
// ----------------------------------------------------------------------------
//   Delete the frame buffer objects and GL tiles
// ----------------------------------------------------------------------------
{
    fbo_map::iterator i;
    for (i = render_fbos.begin(); i != render_fbos.end(); i++)
    {
        fbo_map::key_type    k  = (*i).first;
        fbo_map::mapped_type rv = (*i).second;
        fbo_map::mapped_type tv = texture_fbos[k];
        delete rv;
        if (rv != tv)
            delete tv;
    }
    if (depth_tex)
        glDeleteTextures(1, &depth_tex);
}


void FrameInfo::resize(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the frame buffer used for rendering
// ----------------------------------------------------------------------------
{
    // Don't change anything if size stays the same
    if (render_fbo &&
        render_fbo->width() == int(w) &&
        render_fbo->height() == int(h))
        return;

    // Delete anything we might have
    if (texture_fbo)
        delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;

    // Select whether we draw directly in texture or blit to it
    // If we can blit and suceed in creating a multisample buffer,
    // we first draw in a multisample buffer
    // with 4 samples per pixel. This cannot be used directly as texture.
    QGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(this->format);
    format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        QGLFramebufferObjectFormat mformat(format);
        mformat.setSamples(4);
        render_fbo = new QGLFramebufferObject(w, h, mformat);
        QGLFramebufferObjectFormat actualFormat = render_fbo->format();
        int samples = actualFormat.samples();
        if (samples > 0)
        {
            // REVISIT: we pass format to have a depth buffer attachment.
            // This is required only when we want to later use depthTexture().
            // TODO: specify at object creation?
            texture_fbo = new QGLFramebufferObject(w, h, format);
        }
        else
        {
            // Multisample framebuffer objects are not supported.
            // Normally we could just do: texture_fbo = render_fbo, to use
            // the FBO as a texture.
            // But on Windows/VMWare, even when samples == 0 the
            // FBO cannot be used directly as a texture.
            // 2 options: (1) create a texture_fbo and blit as if MS was
            // enabled, or (2) re-create render FBO without asking for
            // multisampling. (2) is obviously better.
            delete render_fbo;
            render_fbo = new QGLFramebufferObject(w, h, format);
            texture_fbo = render_fbo;
        }
    }
    else
    {
        render_fbo = new QGLFramebufferObject(w, h, format);
        texture_fbo = render_fbo;
    }

    // depth_tex is optional, resize if we have one
    if (depth_tex)
        resizeDepthTexture(w, h);

    this->w = w; this->h = h;
    glShowErrors();

    // Clear the contents of the newly created frame buffer
    render_fbo->bind();
    clear();
    render_fbo->release();
}


void FrameInfo::resizeDepthTexture(uint w, uint h)
// ----------------------------------------------------------------------------
//   Change the size of the depth texture
// ----------------------------------------------------------------------------
{
    if (depth_tex && this->w == w && this->h == h)
        return;

    // Delete current depth texture
    if (depth_tex)
        glDeleteTextures(1, &depth_tex);

    // Create the depth texture
    glGenTextures(1, &depth_tex);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glShowErrors();
}


void FrameInfo::begin(bool clearContents)
// ----------------------------------------------------------------------------
//   Begin rendering in the given off-screen buffer
// ----------------------------------------------------------------------------
{
    // Clear the render FBO
    checkGLContext();
    int ok = render_fbo->bind();
    if (!ok) std::cerr << "FrameInfo::begin(): unexpected result\n";
    glShowErrors();

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
    int ok = render_fbo->release();
    if (!ok) std::cerr << "FrameInfo::end(): unexpected result\n";
    glShowErrors();

    // Blit the result in the texture if necessary
    blit();
    if (depth_tex)
        copyToDepthTexture();
    glShowErrors();
}


GLuint FrameInfo::bind()
// ----------------------------------------------------------------------------
//   Bind the GL texture associated to the off-screen buffer
// ----------------------------------------------------------------------------
{
    checkGLContext();
    GLuint texId = texture_fbo->texture();
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void FrameInfo::blit()
// ----------------------------------------------------------------------------
//   Blit from multisampled FBO to non-multisampled FBO
// ----------------------------------------------------------------------------
{
    if (render_fbo != texture_fbo)
    {
        GLenum buffers = GL_COLOR_BUFFER_BIT;
        if (depth_tex)
            buffers |= GL_DEPTH_BUFFER_BIT;
        QRect rect(0, 0, render_fbo->width(), render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(texture_fbo, rect,
                                              render_fbo, rect,
                                              buffers);
    }
}


GLuint FrameInfo::texture()
// ----------------------------------------------------------------------------
//   Return the GL texture associated to the off-screen buffer
// ----------------------------------------------------------------------------
{
    checkGLContext();
    return texture_fbo->texture();
}


GLuint FrameInfo::depthTexture()
// ----------------------------------------------------------------------------
//   Return the GL texture associated to the depth buffer attachment
// ----------------------------------------------------------------------------
{
    checkGLContext();
    if (!depth_tex)
    {
        resizeDepthTexture(w, h);
        blit();
        copyToDepthTexture();
    }
    return depth_tex;
}


void FrameInfo::checkGLContext()
// ----------------------------------------------------------------------------
//   Make sure FBOs have been allocated for the current GL context
// ----------------------------------------------------------------------------
{
    if (!render_fbos.count(QGLContext::currentContext()))
        resize(w, h);
}


QImage FrameInfo::toImage()
// ----------------------------------------------------------------------------
//   Return the frame buffer contents as a QImage
// ----------------------------------------------------------------------------
{
    checkGLContext();
    return render_fbo->toImage();
}


void FrameInfo::copyToDepthTexture()
// ----------------------------------------------------------------------------
//   Copy the texture_fbo depth buffer into our depth texture
// ----------------------------------------------------------------------------
{
    GLint fbname = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbname);
    glBindFramebuffer(GL_FRAMEBUFFER, texture_fbo->handle());
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbname);
    glShowErrors();
}


ModuleApi::fbo * FrameInfo::newFrameBufferObject(uint w, uint h)
// ----------------------------------------------------------------------------
//   Create a framebuffer object
// ----------------------------------------------------------------------------
{
    return (ModuleApi::fbo *)new FrameInfo(w, h);
}


ModuleApi::fbo * FrameInfo::newFrameBufferObjectWithFormat(uint w, uint h, uint format)
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
    info->checkGLContext();
    info->render_fbo->bind();
    glClearColor(0,0,0,0);
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
