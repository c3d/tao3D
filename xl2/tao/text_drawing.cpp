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
#include "widget.h"
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
    QFontMetricsF fm(font);

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

    Widget *widget = where->Display();
    glLoadName(widget->newId());
    where->offset = Point3();
    GraphicPath::Draw(where, path, GLU_TESS_WINDING_ODD, -1);
    where->offset = position;
    glLoadName(0);
}


void TextSpan::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Move the offset without drawing the text
// ----------------------------------------------------------------------------
{
    Point3 position = where->offset;
    QString str = value;
    QFontMetricsF fm(font);

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

    Widget *widget = where->Display();
    widget->newId();
}


void TextSpan::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   For the moment, we simply draw
// ----------------------------------------------------------------------------
{
    TextSpan::Draw(where);
}


void TextSpan::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Render a portion of text and advance by the width of the text
// ----------------------------------------------------------------------------
{
    Point3 position = path.position;
    QFontMetricsF fm(font);

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
    QFontMetricsF fm(font);
    QRectF rect = fm.tightBoundingRect(value);
    return Box3(rect.x(), rect.height()+rect.y(), 0,
                rect.width(), rect.height(), 0);    
}


Box3 TextSpan::Space()
// ----------------------------------------------------------------------------
//   Return the box that surrounds the text, including leading
// ----------------------------------------------------------------------------
{
    QFontMetricsF fm(font);
    coord height = fm.height();
    coord descent = fm.descent();
    coord leading = fm.leading();
    coord width = fm.width(value);
    coord leftBearing = 0;
    if (value.length())
        leftBearing = fm.leftBearing(value[0]);
    return Box3(leftBearing, -descent-leading, 0, width, height+leading, 0);
}


TextSpan *TextSpan::Break(BreakOrder &order)
// ----------------------------------------------------------------------------
//   If the text span contains a word or line break, cut there
// ----------------------------------------------------------------------------
{
    if (order <= LineBreak)
    {
        uint i, max = value.length();
        for (i = 0; i < max; i++)
        {
            QChar c = value[i];
            BreakOrder charOrder = CharBreak;
            if (c.isSpace())
            {
                charOrder = WordBreak;
                if (c == '\n')
                    charOrder = LineBreak;
            }
            if (order <= charOrder)
            {
                // Create two text spans, the first one containing the split
                QString remainder = value.mid(i+1);
                TextSpan *result = remainder.length()
                    ? new TextSpan(remainder, font)
                    : NULL;
                value = value.left(i+1);
                order = charOrder;
                return result;
            }
        }
    }
    order = NoBreak;
    return NULL;
}


scale TextSpan::TrailingSpaceSize()
// ----------------------------------------------------------------------------
//   Return the size of all the spaces at the end of the value
// ----------------------------------------------------------------------------
{
    QFontMetricsF fm(font);
    scale result = 0;
    for (int i = value.length(); i > 0; i--)
    {
        QChar c = value[i-1];
        if (!c.isSpace())
            break;
        result += fm.width(c);
    }
    return result;
}

TAO_END
