// *****************************************************************************
// texture.cpp                                                     Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2012-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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
