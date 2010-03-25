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
//   Render a portion of text
// ----------------------------------------------------------------------------
{
    QString qs = QString::fromUtf8(utf8.data(), utf8.length());
    QFont font = where->textFont ? where->textFont->font : qApp->font();
    QPainterPath path;
    path.addText(0,0, font, qs);
    GraphicPath::Draw(where, path, GLU_TESS_WINDING_ODD, -1);
}


Box3 TextSpan::Bounds()
// ----------------------------------------------------------------------------
//   Return the smallest box that surrounds the text
// ----------------------------------------------------------------------------
{
    QString qs = QString::fromUtf8(utf8.data(), utf8.length());
    QFont font = TextFont::last ? TextFont::last->font : qApp->font();
    QFontMetrics fm(font);
    QRect rect = fm.boundingRect(qs);
    return Box3(rect.x(), rect.y(), 0, rect.width(), rect.height(), 0);    
}


Box3 TextSpan::Space()
// ----------------------------------------------------------------------------
//   Return the box that surrounds the text, including leading
// ----------------------------------------------------------------------------
{
    QString qs = QString::fromUtf8(utf8.data(), utf8.length());
    QFont font = TextFont::last ? TextFont::last->font : qApp->font();
    QFontMetrics fm(font);
    int flags = Qt::AlignCenter;
    QRect openSpace(-10000, -10000, 20000, 20000);
    QRect rect = fm.boundingRect(openSpace, flags, qs);
    return Box3(rect.x(), rect.y(), 0, rect.width(), rect.height(), 0);    
}


void TextFont::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the font in the layout
// ----------------------------------------------------------------------------
{
    where->textFont = this;
    last = this;
}

TextFont *TextFont::last = NULL;

TAO_END
