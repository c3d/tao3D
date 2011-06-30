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

FrameInfo::FrameInfo(uint w, uint h)
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects
// ----------------------------------------------------------------------------
    : w(w), h(h), refreshTime(-1)
{
    resize(w, h);
}


FrameInfo::FrameInfo(const FrameInfo &o)
// ----------------------------------------------------------------------------
//   Copy constructor - Don't copy the framebuffers
// ----------------------------------------------------------------------------
    : XL::Info(o), w(o.w), h(o.h), refreshTime(o.refreshTime)
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
    format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
#if !defined(Q_OS_LINUX)
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        // Setting this on Linux crashes in the first framebuffer object ctor
        QGLFramebufferObjectFormat mformat(format);
        mformat.setSamples(4);
        render_fbo = new QGLFramebufferObject(w, h, mformat);
        QGLFramebufferObjectFormat actualFormat = render_fbo->format();
        int samples = actualFormat.samples();
        if (samples > 0)
        {
            texture_fbo = new QGLFramebufferObject(w, h);
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
#endif // CONFIG_LINUX
    {
        render_fbo = new QGLFramebufferObject(w, h, format);
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
    checkGLContext();
    int ok = render_fbo->bind();
    if (!ok) std::cerr << "FrameInfo::begin(): unexpected result\n";
    glShowErrors();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);
    glClearColor(1,1,1,0);
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


ModuleApi::fbo * FrameInfo::newFrameBufferObject(uint w, uint h)
// ----------------------------------------------------------------------------
//   Create a framebuffer object
// ----------------------------------------------------------------------------
{
    return (ModuleApi::fbo *)new FrameInfo(w, h);
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
