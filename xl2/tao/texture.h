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
    typedef std::map<text, GLuint>      texture_map;
    enum { MAX_TEXTURES = 20 };

    ImageTextureInfo(Widget *w);
    ~ImageTextureInfo();
    GLuint bind(text img);
    operator data_t() { return this; }

    texture_map textures;
    Widget *    widget;
    GLuint      defaultTextureId;
};

TAO_END

#endif // texture.h

