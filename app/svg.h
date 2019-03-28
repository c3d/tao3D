#ifndef SVG_H
#define SVG_H
// *****************************************************************************
// svg.h                                                           Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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