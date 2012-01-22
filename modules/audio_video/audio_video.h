#ifndef AUDIO_VIDEO_H
#define AUDIO_VIDEO_H
// ****************************************************************************
//  audio_video.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    Play video as a GL texture.
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
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010-2011 Taodyne SAS
// ****************************************************************************

#include "tao_gl.h"
#include <QObject>
#include <QGLFramebufferObject>
#include <phonon>
#include "tree.h"
#include "context.h"
#include "tao/module_api.h"
#include "tao/tao_info.h"

struct VideoSurface : QObject, Tao::Info
// ----------------------------------------------------------------------------
//    Display a Phonon::VideoWidget
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    typedef VideoSurface * data_t;
    VideoSurface();
    ~VideoSurface();
    virtual void           Delete() { tao->deferredDelete(this); }
    operator               data_t() { return this; }
    virtual GLuint         bind(XL::Text *url);
    int                    width()  { return video->width(); }
    int                    height() { return video->height(); }

public:
    // XL interface
    static XL::Integer_p   movie_texture(XL::Context_p context,
                                         XL::Tree_p self, text name);

protected:
    GLuint                 texture();
    std::ostream &         debug();
public:
    text                   url, unresolvedName, lastError;
    Phonon::VideoWidget *  video;
    Phonon::AudioOutput *  audio;
    Phonon::MediaObject *  media;
    bool                   useFBO;
    QGLFramebufferObject * fbo;
    GLuint                 textureId;
    int                    w, h;
    static const
    Tao::ModuleApi *       tao;
    static bool            tested, licensed;
};

#endif // AUDIO_VIDEO_H
