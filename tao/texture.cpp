// ****************************************************************************
//  texture.cpp                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     Loading a texture from an image
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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "texture.h"
#include "texture_cache.h"
#include "tao_utf8.h"
#include "application.h"


TAO_BEGIN

// ============================================================================
// 
//    Image textures
// 
// ============================================================================

ImageTextureInfo::ImageTextureInfo()
// ----------------------------------------------------------------------------
//   Prepare to record texture IDs for the various images
// ----------------------------------------------------------------------------
    : width(0), height(0)
{}


ImageTextureInfo::~ImageTextureInfo()
// ----------------------------------------------------------------------------
//   Release the GL texture
// ----------------------------------------------------------------------------
{}

static inline ImageTextureInfo::Texture computeDefaultTexture()
// ----------------------------------------------------------------------------
//   A texture that we use when the given source image is invalid
// ----------------------------------------------------------------------------
{
    ImageTextureInfo::Texture result = { 0,0,0 };
    glGenTextures(1, &result.id);

    QString file(":/images/default_image.svg");
    QImage image(file);
    if (!image.isNull())
    {
        QImage texture = QGLWidget::convertToGLFormat(image);
        result.width = texture.width();
        result.height = texture.height();

        // Generate the GL texture
        GL.BindTexture(GL_TEXTURE_2D, result.id);
        GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     result.width, result.height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());
    }
    return result;
}


ImageTextureInfo::Texture &ImageTextureInfo::defaultTexture()
// ----------------------------------------------------------------------------
//   Build a singleton texture ID used when source image is invalid
// ----------------------------------------------------------------------------
{
    static Texture texture = computeDefaultTexture();
    return texture;
}


ImageTextureInfo::Texture ImageTextureInfo::load(text file, text docPath)
// ----------------------------------------------------------------------------
//   Load the given GL texture
// ----------------------------------------------------------------------------
{
    Texture texture;
    CachedTexture * cached = TextureCache::instance()->load(+file, +docPath);
    texture.id = cached->id;
    width = texture.width = cached->width;
    height = texture.height = cached->height;
    return texture;
}



// ============================================================================
// 
//   Movie textures
// 
// ============================================================================

AnimatedTextureInfo::texture_map AnimatedTextureInfo::textures;

AnimatedTextureInfo::AnimatedTextureInfo()
// ----------------------------------------------------------------------------
//   Create an empty movie
// ----------------------------------------------------------------------------
    : ImageTextureInfo()
{}


AnimatedTextureInfo::~AnimatedTextureInfo()
// ----------------------------------------------------------------------------
//   Delete the reference to a movie
// ----------------------------------------------------------------------------
{}


AnimatedTextureInfo::Texture AnimatedTextureInfo::load(text file)
// ----------------------------------------------------------------------------
//   Load a movie from the given file
// ----------------------------------------------------------------------------
{
    texture_map::iterator found = textures.find(file);
    Texture texinfo = { 0, 0, 0 };
    if (found == textures.end())
    {
        // Prune the map if it gets too big
        while (textures.size() > MAX_TEXTURES)
        {
            texture_map::iterator first = textures.begin();
            if ((*first).second.id != defaultTexture().id)
                GL.DeleteTextures(1, &(*first).second.id);
            textures.erase(first);
        }

        // Read the image file
        movie.setFileName(+file);
        if (!movie.isValid())
        {
            text qualified = "texture:" + file;
            movie.setFileName(+qualified);
        }
        movie.start();

        // Allocate texture ID
        GL.GenTextures(1, &texinfo.id);

        // Remember the texture for next time
        textures[file] = texinfo;
    }
    else
    {
        texinfo = (*found).second;
    }

    return texinfo;
}


GLuint AnimatedTextureInfo::bind(text file)
// ----------------------------------------------------------------------------
//   Bind the current picture in the movie
// ----------------------------------------------------------------------------
{
    Texture texinfo = load(file);

    QImage image = movie.currentImage();
    if (!image.isNull())
    {
        texinfo.width = image.width();
        texinfo.height = image.height();
        QImage texture = QGLWidget::convertToGLFormat(image);

        // Generate the GL texture
        Q_ASSERT(texinfo.id);
        GL.BindTexture(GL_TEXTURE_2D, texinfo.id);
        GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     texinfo.width, texinfo.height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());
    }

    GL.BindTexture(GL_TEXTURE_2D, texinfo.id);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL.TexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL.Enable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        GL.Enable(GL_MULTISAMPLE);
    width = texinfo.width;
    height = texinfo.height;

    return texinfo.id;
}

TAO_END
