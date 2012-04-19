// ****************************************************************************
//  audio_video.cpp                                                Tao project
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

#include "audio_video.h"
#include "errors.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QEvent>

using namespace XL;

const Tao::ModuleApi * VideoSurface::tao = NULL;
bool                   VideoSurface::tested = false;
bool                   VideoSurface::licensed = false;

VideoSurface::VideoSurface()
// ----------------------------------------------------------------------------
//   Create the video player
// ----------------------------------------------------------------------------
    : video(new Phonon::VideoWidget),
      audio(new Phonon::AudioOutput(Phonon::VideoCategory)),
      media(new Phonon::MediaObject),
      useFBO(QGLFramebufferObject::hasOpenGLFramebufferObjects()),
      fbo(NULL), textureId(0), w(0), h(0)
{
    IFTRACE(video)
        debug() << "Creating VideoSurface\n";

    if (!useFBO)
    {
        IFTRACE(video)
            debug() << "FBO not supported: will use QImage\n";
        glGenTextures(1, &textureId);
    }
    Phonon::createPath(media, audio);
    Phonon::createPath(media, video);
    video->setAttribute(Qt::WA_DontShowOnScreen);
    video->setVisible(true);
    video->setAutoFillBackground(false);
}


VideoSurface::~VideoSurface()
// ----------------------------------------------------------------------------
//    Stop the player and delete the frame buffer object
// ----------------------------------------------------------------------------
{
    IFTRACE(video)
        debug() << "Stopping playback and deleting VideoSurface\n";

    media->stop();
    delete video;
    delete audio;
    delete media;
    if (textureId)
        glDeleteTextures(1, &textureId);
    if (fbo)
        delete fbo;
}


GLuint VideoSurface::texture()
// ----------------------------------------------------------------------------
//    Return the identifier of the video texture
// ----------------------------------------------------------------------------
{
    if (fbo)
        return fbo->texture();
    return textureId;
}


GLuint VideoSurface::bind(XL::Text *urlTree)
// ----------------------------------------------------------------------------
//    Bind the surface to the texture
// ----------------------------------------------------------------------------
{
    tao->makeGLContextCurrent();

    if (urlTree->value != url)
    {
        IFTRACE2(fileload, video)
        {
              debug() << "Loading media: " << urlTree->value << "\n";
              debug() << "     previous: " << url << "\n";
        }

        url = urlTree->value;
        media->stop();
        if (url == "")
            return texture();

        QString u = QString::fromUtf8(url.data(), url.length());
        media->setCurrentSource(Phonon::MediaSource(QUrl(u)));
        media->play();
        tao->makeGLContextCurrent();
        if (media->state() == Phonon::ErrorState)
        {
            QString e = media->errorString();
            lastError = text(e.toUtf8().constData());
        }
    }

    if (url == "")
        return texture();

    QSize hint = video->sizeHint();
    if (hint.isValid())
    {
        if (hint.width() != w || hint.height() != h)
        {
            // Dimensions changed: resize widget, (re-)allocate FBO

            w = hint.width();
            h = hint.height();
            IFTRACE(video)
                debug() << "Video resolution: " << w << "x" << h << "\n";

            video->resize(w, h);
            if (useFBO)
            {
                IFTRACE(video)
                    debug() << "Allocating FBO\n";
                if (fbo)
                    delete fbo;
                fbo = new QGLFramebufferObject(w, h);
            }
        }

        if (useFBO)
        {
            video->render(fbo);
        }
        else
        {
            QImage image(w, h, QImage::Format_ARGB32);
            video->render(&image);
            image = QGLWidget::convertToGLFormat(image);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, 3,
                         image.width(), image.height(), 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, image.bits());
        }
        return texture();
    }

    // Default is to return no texture
    return 0;
}


std::ostream & VideoSurface::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[AudioVideo] " << (void*)this << " ";
    return std::cerr;
}



XL::Integer_p VideoSurface::movie_texture(XL::Context_p context,
                                     XL::Tree_p self, text name)
// ----------------------------------------------------------------------------
//   Make a video player texture
// ----------------------------------------------------------------------------
{
    if (!tested)
    {
        licensed = tao->checkImpressOrLicense("AudioVideo 1.0");
        tested = true;
    }
    if (!licensed && !tao->blink(1.0, 0.2, 300))
        return 0;


    // Get or build the current frame if we don't have one
    VideoSurface *surface = self->GetInfo<VideoSurface>();
    if (!surface)
    {
        surface = new VideoSurface();
        self->SetInfo<VideoSurface> (surface);
    }

    if (name != surface->unresolvedName)
    {
        // name has not been converted to URL format, or has changed
        surface->unresolvedName = name;
        if (name != "")
        {
            QRegExp re("[a-z]+://");
            QString qn = QString::fromUtf8(name.data(), name.length());
            if (re.indexIn(qn) == -1)
            {
                name = context->ResolvePrefixedPath(name);
                text folder = tao->currentDocumentFolder();
                QString qf = QString::fromUtf8(folder.data(), folder.length());
                QString qn = QString::fromUtf8(name.data(), name.length());
                QFileInfo inf(QDir(qf), qn);
                if (inf.isReadable())
                {
                    name =
#if defined(Q_OS_WIN)
                            "file:///"
#else
                            "file://"
#endif
                            + text(inf.absoluteFilePath().toUtf8().constData());
                }
            }
        }
    }
    else
    {
        // Raw name has not changed, no need to resolve again
        name = surface->url;
    }

    // Resize to requested size, and bind texture
    GLuint id = surface->bind(new Text(name));
    if (surface->lastError != "")
    {
        XL::Ooops("Cannot play: $1", self);
        text err = "Media player error: ";
        err.append(surface->lastError);
        XL::Ooops(err, self);
        text err2 = "Path or URL: ";
        err2.append(surface->url);
        XL::Ooops(err2, self);
        surface->lastError = "";
        return new Integer(0, self->Position());
    }
    if (id != 0)
        tao->BindTexture2D(id, surface->width(), surface->height());

    tao->refreshOn(QEvent::Timer, -1.0);
    return new Integer(id, self->Position());
}


XL_DEFINE_TRACES

int module_init(const Tao::ModuleApi *api, const Tao::ModuleInfo *mod)
// ----------------------------------------------------------------------------
//   Initialize the Tao module
// ----------------------------------------------------------------------------
{
    Q_UNUSED(mod);
    glewInit();
    XL_INIT_TRACES();
    VideoSurface::tao = api;
    return 0;
}


int module_exit()
// ----------------------------------------------------------------------------
//   Uninitialize the Tao module
// ----------------------------------------------------------------------------
{
    return 0;
}
