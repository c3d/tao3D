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
#include "tao_utf8.h"
#include "application.h"


TAO_BEGIN

// ============================================================================
// 
//    Image textures
// 
// ============================================================================

ImageTextureInfo::texture_map ImageTextureInfo::textures;

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
        glBindTexture(GL_TEXTURE_2D, result.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
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


ImageTextureInfo::Texture ImageTextureInfo::load(text file)
// ----------------------------------------------------------------------------
//   Load the given GL texture
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
                glDeleteTextures(1, &(*first).second.id);
            textures.erase(first);
        }

        // Read the image file and convert to proper GL image format
        QString loaded = +file;
        QImage image(+file);
        if (image.isNull())
        {
            text qualified = "texture:" + file;
            if (image.load(+qualified))
                loaded = +qualified;
        }
        if (!image.isNull())
        {
            IFTRACE(fileload)
                std::cerr << "Loaded texture: " << +loaded << "\n";
            texinfo.width = image.width();
            texinfo.height = image.height();
            QImage texture = QGLWidget::convertToGLFormat(image);

            // Generate the GL texture
            glGenTextures(1, &texinfo.id);
            glBindTexture(GL_TEXTURE_2D, texinfo.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         texinfo.width, texinfo.height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, texture.bits());
        }
        else
        {
            IFTRACE(fileload)
                std::cerr << "Failed to load texture: " << file << "\n";
            texinfo = defaultTexture();
        }

        // Remember the texture for next time
        textures[file] = texinfo;
    }
    else
    {
        texinfo = (*found).second;
    }

    return texinfo;
}


GLuint ImageTextureInfo::bind(text file)
// ----------------------------------------------------------------------------
//   Bind the given GL texture
// ----------------------------------------------------------------------------
{
    Texture texinfo = load(file);

    glBindTexture(GL_TEXTURE_2D, texinfo.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        glEnable(GL_MULTISAMPLE);
    width = texinfo.width;
    height = texinfo.height;

    return texinfo.id;
}



// ============================================================================
// 
//   Movie textures
// 
// ============================================================================

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
                glDeleteTextures(1, &(*first).second.id);
            textures.erase(first);
        }

        // Read the image file and convert to proper GL image format
        movie.setFileName(+file);
        if (!movie.isValid())
        {
            text qualified = "texture:" + file;
            movie.setFileName(+qualified);
        }
        movie.start();


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
        glGenTextures(1, &texinfo.id);
        glBindTexture(GL_TEXTURE_2D, texinfo.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     texinfo.width, texinfo.height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());
    }

    glBindTexture(GL_TEXTURE_2D, texinfo.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        glEnable(GL_MULTISAMPLE);
    width = texinfo.width;
    height = texinfo.height;

    return texinfo.id;
}

TAO_END
