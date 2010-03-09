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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "texture.h"
#include <QtOpenGL>


TAO_BEGIN

ImageTextureInfo::ImageTextureInfo()
// ----------------------------------------------------------------------------
//   Prepare to record texture IDs for the various images
// ----------------------------------------------------------------------------
    : textures()
{}


ImageTextureInfo::~ImageTextureInfo()
// ----------------------------------------------------------------------------
//   Release the GL texture
// ----------------------------------------------------------------------------
{
    texture_map::iterator i;
    glDisable(GL_TEXTURE_2D);
    for (i = textures.begin(); i != textures.end(); i++)
        glDeleteTextures(1, &(*i).second);
}


void ImageTextureInfo::bind(text file)
// ----------------------------------------------------------------------------
//   Bind the given GL texture
// ----------------------------------------------------------------------------
{
    GLuint textureId = textures[file];
    if (textureId == 0)
    {
        // Prune the map if it gets too big
        while (textures.size() > MAX_TEXTURES)
            textures.erase(textures.begin());

        // Read the image file and convert to proper GL image format
        QImage original(QString::fromStdString(file));
        QImage texture = QGLWidget::convertToGLFormat(original);

        // Generate the GL texture
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
                     texture.width(), texture.height(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, texture.bits());

        // Remember the texture for next time
        textures[file] = textureId;
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
#ifdef GL_MULTISAMPLE   // Not supported on Windows
    glEnable(GL_MULTISAMPLE);
#endif
    glEnable(GL_CULL_FACE);
}

TAO_END
