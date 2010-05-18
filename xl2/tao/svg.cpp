// ****************************************************************************
//  svg.cpp                                                         Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "svg.h"
#include "tao_utf8.h"


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
    glDisable(GL_TEXTURE_2D);
    for (i = renderers.begin(); i != renderers.end(); i++)
        delete (*i).second;

}


QSvgRenderer * SvgRendererInfo::defaultImageRenderer()
// ----------------------------------------------------------------------------
//   Return an image to use when the source file is invalid
// ----------------------------------------------------------------------------
{
    static  QSvgRenderer * defSvg =
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
        r = new QSvgRenderer(svgFile, widget);
        if (r->isValid())
        {
            r->connect(r, SIGNAL(repaintNeeded()), widget, SLOT(updateGL()));
            renderers[file] = r;
        }
        else
        {
            delete r;
            r = defaultImageRenderer();
        }
    }

    if (r)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_MULTISAMPLE);
        FramePainter painter(this);
        r->render(&painter);
    }

    return FrameInfo::bind();
}

TAO_END
