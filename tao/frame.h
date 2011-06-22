#ifndef FRAME_H
#define FRAME_H
// ****************************************************************************
//  frame.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tree.h"
#include "gl_keepers.h"
#include "module_api.h"
#include <map>
#include <QImage>

TAO_BEGIN

struct FrameInfo : XL::Info
// ----------------------------------------------------------------------------
//    Information about a given frame being rendered in a dynamic texture
// ----------------------------------------------------------------------------
//    A FrameInfo structure tends to be permanently attached to a tree
//    to record information that we don't want to re-create / destroy
//    at each and every evaluation of the tree
{
    typedef FrameInfo *data_t;
    typedef std::map<const QGLContext *, QGLFramebufferObject *> fbo_map;

    FrameInfo(uint width = 512, uint height = 512);
    FrameInfo(const FrameInfo &other);
    ~FrameInfo();

    void        resize(uint width, uint height);
    void        begin();
    void        end();
    GLuint      bind();
    void        checkGLContext();
    QImage      toImage();

    uint    w, h;
    double  refreshTime;
    fbo_map render_fbos;
    fbo_map texture_fbos;

#define render_fbo  render_fbos[QGLContext::currentContext()]
#define texture_fbo texture_fbos[QGLContext::currentContext()]

    // Static methods exported by the module interface. See module_api.h.

    static ModuleApi::fbo *   newFrameBufferObject(uint w, uint h);
    static void               deleteFrameBufferObject(ModuleApi::fbo * obj);
    static void               resizeFrameBufferObject(ModuleApi::fbo * obj,
                                                      uint w, uint h);
    static void               bindFrameBufferObject(ModuleApi::fbo * obj);
    static void               releaseFrameBufferObject(ModuleApi::fbo * obj);
    static unsigned int       frameBufferObjectToTexture(ModuleApi::fbo * obj);
};


template<typename Index>
struct MultiFrameInfo : XL::Info
// ----------------------------------------------------------------------------
//   Records a number of frameinfos indexed on some value
// ----------------------------------------------------------------------------
{
    typedef std::map<Index, FrameInfo>        frame_map;

    MultiFrameInfo() {}
    ~MultiFrameInfo() {}

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

TAO_END

#endif // FRAME_H
