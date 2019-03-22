// *****************************************************************************
// svg.cpp                                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//    Render SVG documents in a texture
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
// (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "svg.h"
#include "tao_utf8.h"
#include "application.h"


TAO_BEGIN

SvgRendererInfo::SvgRendererInfo(Widget *w, uint width, uint height)
// ----------------------------------------------------------------------------
//   Create a renderer with the right size
// ----------------------------------------------------------------------------
    : FrameInfo(width, height), widget(w)
{
}


SvgRendererInfo::~SvgRendererInfo()
// ----------------------------------------------------------------------------
//   When deleting the info, delete all renderers we have
// ----------------------------------------------------------------------------
{
    renderer_map::iterator i;
    GL.Disable(GL_TEXTURE_2D);
    for (i = renderers.begin(); i != renderers.end(); i++)
        delete (*i).second;
}


QSvgRenderer * SvgRendererInfo::defaultImageRenderer()
// ----------------------------------------------------------------------------
//   Return an image to use when the source file is invalid
// ----------------------------------------------------------------------------
{
    static QSvgRenderer * defSvg =
        new QSvgRenderer(QString(":/images/default_image.svg"));
    return defSvg;
}


GLuint SvgRendererInfo::bind (text file)
// ----------------------------------------------------------------------------
//    Activate a given SVG renderer
// ----------------------------------------------------------------------------
{
    QSvgRenderer *r = renderers[file];
    if (!r)
    {
        while (renderers.size() > MAX_TEXTURES)
        {
            renderer_map::iterator first = renderers.begin();
            delete (*first).second;
            renderers.erase(first);
        }

        QString svgFile(+file);
        QFileInfo fi(svgFile);
        if (!fi.isReadable())
            svgFile = "svg:" + svgFile;
        r = new QSvgRenderer(svgFile, widget);
        if (r->isValid())
        {
            r->connect(r, SIGNAL(repaintNeeded()), widget, SLOT(updateGL()));
        }
        else
        {
            delete r;
            r = defaultImageRenderer();
        }
        renderers[file] = r;
    }

    if (r)
    {
        GL.Disable(GL_TEXTURE_2D);
        if (TaoApp->hasGLMultisample)
            GL.Disable(GL_MULTISAMPLE);
        FramePainter painter(this);
        r->render(&painter);
    }

    return FrameInfo::bind();
}

TAO_END
