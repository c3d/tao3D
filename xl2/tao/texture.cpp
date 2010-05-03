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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "texture.h"
#include "tao_utf8.h"


TAO_BEGIN

ImageTextureInfo::ImageTextureInfo(Widget *w)
// ----------------------------------------------------------------------------
//   Prepare to record texture IDs for the various images
// ----------------------------------------------------------------------------
    : textures(), widget(w), width(0.0), height(0.0)
{
}


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


static inline GLuint computeDefaultTexture()
// ----------------------------------------------------------------------------
//   A texture that we use when the given source image is invalid
// ----------------------------------------------------------------------------
{
    GLuint txtId;
    QString file(":/images/defaultImage.svg");
    QImage defOrig(file);
    if (defOrig.isNull())
        return 0;

    QImage texture = QGLWidget::convertToGLFormat(defOrig);

    // Generate the GL texture
    glGenTextures(1, &txtId);
    glBindTexture(GL_TEXTURE_2D, txtId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texture.width(), texture.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, texture.bits());
    return txtId;
}


GLuint ImageTextureInfo::defaultTextureId()
// ----------------------------------------------------------------------------
//   Build a singleton texture ID used when source image is invalid
// ----------------------------------------------------------------------------
{
    static GLuint defTextureId = computeDefaultTexture();
    return defTextureId;
}


GLuint ImageTextureInfo::bind(text file)
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
        QImage original(+file);
        if (!original.isNull())
        {
            width = original.width();
            height = original.height();
            QImage texture = QGLWidget::convertToGLFormat(original);

            // Generate the GL texture
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         texture.width(), texture.height(), 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, texture.bits());

            // Remember the texture for next time
            textures[file] = textureId;
        }
        else
        {
            textureId = defaultTextureId();
        }
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
#ifdef GL_MULTISAMPLE   // Not supported on Windows
    glEnable(GL_MULTISAMPLE);
#endif

    return textureId;
}

TAO_END
