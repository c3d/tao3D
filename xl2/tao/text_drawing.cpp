// ****************************************************************************
//  text_drawing.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//    Rendering of text
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

#include "text_drawing.h"
#include "path3d.h"
#include "layout.h"
#include <GL/glew.h>
#include <QtOpenGL>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>

TAO_BEGIN

void TextSpan::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Render a portion of text and advance by the width of the text
// ----------------------------------------------------------------------------
{
    Point3 position = where->offset;
    QPainterPath path;
    path.addText(position.x, position.y, font, value);
    GraphicPath::Draw(where, path, GLU_TESS_WINDING_ODD, -1);
    QFontMetrics fm(font);
    where->offset.x += fm.width(value);
}


void TextSpan::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Render a portion of text and advance by the width of the text
// ----------------------------------------------------------------------------
{
    Point3 position = path.position;
    QPainterPath qt;
    qt.addText(position.x, position.y, font, value);
    path.addQtPath(qt, -1);
    QFontMetrics fm(font);
    path.moveTo(position + Vector3(fm.width(value), 0, 0));
}


Box3 TextSpan::Bounds()
// ----------------------------------------------------------------------------
//   Return the smallest box that surrounds the text
// ----------------------------------------------------------------------------
{
    QFontMetrics fm(font);
    QRect rect = fm.tightBoundingRect(value);
    return Box3(rect.x(), rect.y(), 0, rect.width(), rect.height(), 0);    
}


Box3 TextSpan::Space()
// ----------------------------------------------------------------------------
//   Return the box that surrounds the text, including leading
// ----------------------------------------------------------------------------
{
    QFontMetrics fm(font);
    int flags = Qt::AlignCenter;
    QRect openSpace(-10000, -10000, 20000, 20000);
    QRect rect = fm.boundingRect(openSpace, flags, value);
    return Box3(rect.x(), rect.y(), 0, rect.width(), rect.height(), 0);    
}

TAO_END
