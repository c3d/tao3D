// ****************************************************************************
//  glyph_cache.cpp                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     Cache transforming glyphs into textures
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
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "glyph_cache.h"
#include <QtOpenGL>

TAO_BEGIN

int GlyphCache::maxFontSize = 64;


GlyphCache::~GlyphCache()
// ----------------------------------------------------------------------------
//   Release all the textures
// ----------------------------------------------------------------------------
{
    for (std::map<Key, uint>::iterator i = cache.begin(); i != cache.end(); i++)
        glDeleteTextures(1, &(*i).second);    
}

TAO_END
