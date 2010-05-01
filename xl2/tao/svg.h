#ifndef SVG_H
#define SVG_H
// ****************************************************************************
//  svg.h                                                           Tao project
// ****************************************************************************
//
//   File Description:
//
//     Rendering of an SVG file in an off-screen buffer
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

#include "frame.h"

TAO_BEGIN

struct SvgRendererInfo : FrameInfo
// ----------------------------------------------------------------------------
//    Hold information about the SVG renderer for a tree
// ----------------------------------------------------------------------------
{
    typedef SvgRendererInfo *                   data_t;
    typedef std::map<text, QSvgRenderer *>      renderer_map;
    enum { MAX_TEXTURES = 20 };

    SvgRendererInfo(Widget *w, uint width=512, uint height=512);
    ~SvgRendererInfo();
    operator data_t() { return this; }
    GLuint bind(text img);

    Widget *            widget;
    renderer_map        renderers;
    static QSvgRenderer * defaultImageRenderer();
};

TAO_END

#endif // SVG_H
