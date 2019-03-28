#ifndef FRAME_H
#define FRAME_H
// *****************************************************************************
// frame.h                                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//     Frames are 2D rectangular drawing surfaces rendered in a GL framebuffer
//     Frames can be placed in other frames in a hierarchy.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010,2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2012, Soulisse Baptiste <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "tao.h"
#include "tree.h"
#include "gl_keepers.h"
#include "module_api.h"
#include "info_trash_can.h"
#include <map>
#include <QImage>

class QGLFramebufferObject;

TAO_BEGIN

struct Layout;

struct FrameInfo : XL::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//    Information about a given frame being rendered in a dynamic texture
// ----------------------------------------------------------------------------
//    A FrameInfo structure tends to be permanently attached to a tree
//    to record information that we don't want to re-create / destroy
//    at each and every evaluation of the tree
{
    typedef FrameInfo *data_t;
    typedef std::map<const QGLContext *, QGLFramebufferObject *> fbo_map;
    typedef std::map<const QGLContext *, GLuint> tex_map;

    FrameInfo(uint width = 512, uint height = 512, uint format = defaultFormat);
    FrameInfo(const FrameInfo &other);
    ~FrameInfo();
    virtual void Delete() { trash.push_back(this); }

    void        resize(uint width, uint height);
    void        setFormat(uint format);

    void        clear();
    void        begin(bool clearContents = true);
    void        end();
    GLuint      bind();
    GLuint      texture();
    GLuint      depthTexture();
    void        checkGLContext();
    QImage      toImage();

    uint                  w, h;
    uint                  format; // Internal texture format
    GLuint                depthTextureID;
    QGLContext           *context;
    QGLFramebufferObject *renderFBO;
    QGLFramebufferObject *textureFBO;
    Layout *              layout;
    Color                 clearColor;


    // Static methods exported by the module interface. See module_api.h.
    static ModuleApi::fbo *   newFrameBufferObject(uint w, uint h);
    static ModuleApi::fbo *   newFrameBufferObjectWithFormat(uint w, uint h,
                                                             uint format);
    static void               deleteFrameBufferObject(ModuleApi::fbo * obj);
    static void               resizeFrameBufferObject(ModuleApi::fbo * obj,
                                                      uint w, uint h);
    static void               bindFrameBufferObject(ModuleApi::fbo * obj);
    static void               releaseFrameBufferObject(ModuleApi::fbo * obj);
    static unsigned int       frameBufferObjectToTexture(ModuleApi::fbo * obj);
    static unsigned int       frameBufferAttachmentToTexture(ModuleApi::fbo *,
                                                             int attachment);
    static void *             imageFromFrameBufferObject(ModuleApi::fbo * obj);

protected:
    void        resizeDepthTexture(uint w, uint h);
    void        copyToDepthTexture();
    void        blit();
    void        backBlit();
    void        purge();

private:
// On MacOSX, GL_RGBA8 is not supported
#if defined(Q_OS_MACX)
    enum { defaultFormat = GL_RGBA };
#else
    enum { defaultFormat = GL_RGBA8 };
#endif
};


template<typename Index>
struct MultiFrameInfo : XL::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//   Records a number of frameinfos indexed on some value
// ----------------------------------------------------------------------------
{
    typedef std::map<Index, FrameInfo>        frame_map;

    MultiFrameInfo() {}
    ~MultiFrameInfo() {}
    virtual void Delete() { trash.push_back(this); }

    FrameInfo &frame(Index what)                { return map[what]; }
    frame_map   map;
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



// ============================================================================
//
//   Entering shapes in the symbols table
//
// ============================================================================

extern void EnterFrames();
extern void DeleteFrames();

TAO_END

#endif // FRAME_H
