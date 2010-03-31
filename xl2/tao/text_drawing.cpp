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
    QString str = value;
    QFontMetrics fm(font);

    int index = str.indexOf(QChar('\n'));
    while (index >= 0)
    {
        QString fragment = str.left(index);
        path.addText(position.x, -position.y, font, fragment);
        position.x = 0;
        position.y -= fm.height();
        str = str.mid(index+1);
        index = str.indexOf(QChar('\n'));
    }

    path.addText(position.x, -position.y, font, str);
    position.x += fm.width(str);

    where->offset = Point3();
    GraphicPath::Draw(where, path, GLU_TESS_WINDING_ODD, -1);
    where->offset = position;
}


void TextSpan::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Move the offset without drawing the text
// ----------------------------------------------------------------------------
{
    Point3 position = where->offset;
    QString str = value;
    QFontMetrics fm(font);

    Shape::DrawSelection(where);

    int index = str.indexOf(QChar('\n'));
    while (index >= 0)
    {
        QString fragment = str.left(index);
        position.x = 0;
        position.y -= fm.height();
        str = str.mid(index+1);
        index = str.indexOf(QChar('\n'));
    }

    position.x += fm.width(str);
    where->offset = position;
}


void TextSpan::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Render a portion of text and advance by the width of the text
// ----------------------------------------------------------------------------
{
    Point3 position = path.position;
    QFontMetrics fm(font);

    QPainterPath qt;

    QString str = value;
    int index = str.indexOf(QChar('\n'));
    while (index >= 0)
    {
        QString fragment = str.left(index);
        qt.addText(position.x, -position.y, font, fragment);
        position.x = 0;
        position.y -= fm.height();
        str = str.mid(index+1);
        index = str.indexOf(QChar('\n'));
    }

    qt.addText(position.x, -position.y, font, str);
    position.x += fm.width(str);

    path.addQtPath(qt, -1);
    path.moveTo(position);
}


Box3 TextSpan::Bounds()
// ----------------------------------------------------------------------------
//   Return the smallest box that surrounds the text
// ----------------------------------------------------------------------------
{
    QFontMetrics fm(font);
    QRect rect = fm.tightBoundingRect(value);
    return Box3(rect.x(), rect.height()+rect.y(), 0, rect.width(), rect.height(), 0);    
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
    return Box3(rect.x(), rect.height()+rect.y(), 0, rect.width(), rect.height(), 0);    
}

TAO_END
