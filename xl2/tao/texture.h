#ifndef TEXTURE_H
#define TEXTURE_H
// ****************************************************************************
//  texture.h                                                       Tao project
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

#include "tao.h"
#include "tree.h"
#include "widget.h"
#include <GL/glew.h>
#include <QtOpenGL>

TAO_BEGIN

struct ImageTextureInfo : XL::Info
// ----------------------------------------------------------------------------
//    Hold information about an image texture
// ----------------------------------------------------------------------------
{
    typedef ImageTextureInfo *          data_t;
    struct  Texture { GLuint id, width, height; };
    typedef std::map<text, Texture>     texture_map;
    enum { MAX_TEXTURES = 200 };

    ImageTextureInfo();
    ~ImageTextureInfo();

    Texture load(text img);
    GLuint bind(text img);
    operator data_t() { return this; }
    uint        width, height;

    static texture_map textures;
    static Texture &defaultTexture();
};

TAO_END

#endif // texture.h

