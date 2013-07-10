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
//   Movie textures
// 
// ============================================================================

AnimatedTextureInfo::AnimatedTextureInfo()
// ----------------------------------------------------------------------------
//   Create an empty animated texture
// ----------------------------------------------------------------------------
    : id(0), width(0), height(0)
{}


AnimatedTextureInfo::~AnimatedTextureInfo()
// ----------------------------------------------------------------------------
//   Delete the reference to an animated texture
// ----------------------------------------------------------------------------
{
    if (id)
        GL.DeleteTextures(1, &id);
}


GLuint AnimatedTextureInfo::load(text file)
// ----------------------------------------------------------------------------
//   Load a movie from the given file
// ----------------------------------------------------------------------------
{
    if (id == 0)
    {
        // Generate the texture ID
        GL.GenTextures(1, &id);

        // Read the image file
        movie.setFileName(+file);
        if (!movie.isValid())
        {
            text qualified = "texture:" + file;
            movie.setFileName(+qualified);
        }
        movie.start();
    }
    return id;
}     



GLuint AnimatedTextureInfo::bind(text file)
// ----------------------------------------------------------------------------
//   Bind the current picture in the movie
// ----------------------------------------------------------------------------
{
    uint id = load(file);

    if (id)
    {
        QImage image = movie.currentImage();
        if (!image.isNull())
        {
            width = image.width();
            height = image.height();
            QImage texture = QGLWidget::convertToGLFormat(image);
            
            // Generate the GL texture
            GL.BindTexture(GL_TEXTURE_2D, id);
            GL.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                          width, height, 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, texture.bits());
        }
    }

    GL.BindTexture(GL_TEXTURE_2D, id);
    GL.Enable(GL_TEXTURE_2D);
    if (TaoApp->hasGLMultisample)
        GL.Enable(GL_MULTISAMPLE);

    return id;
}

TAO_END
