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
#include "utf8.h"
#include "tao_utf8.h"
#include <GL/glew.h>
#include <QtOpenGL>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>

TAO_BEGIN

// ============================================================================
//
//   A text span is a contiguous string of characters with similar formatting
//
// ============================================================================

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
//   Draw the selection for any selected character
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    Point3 pos = where->offset;
    text str = +value;
    QFontMetricsF fm(font);
    scale h = fm.height();
    coord x = pos.x;
    coord y = pos.y;
    coord z = pos.z;
    coord xx, yy, ww, hh;
    coord descent = fm.descent();
    coord leading = fm.leading();

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = 0; i < max; i = XL::Utf8Next(str, i))
    {
        QChar qc = QChar(XL::Utf8Code(str, i));
        float w = qc == '\n' ? 3 : fm.width(qc);

        xx = x + fm.leftBearing(qc);
        yy = y - descent - leading;
        ww = w;
        hh = h + leading;

        GLuint charId = widget->newId();
        bool charSelected = widget->selected();
        TextSelect *sel = NULL;
        if (charSelected)
        {
            sel = widget->textSelection(true);
            if (sel)
            {
                if (!sel->start || sel->start > charId)
                    sel->start = charId;
                if (sel->end < charId)
                    sel->end = charId;
            }
        }
        else
        {
            sel = widget->textSelection(false);
            if (sel)
            {
                if (charId >= sel->start && charId <= sel->end)
                {
                    charSelected = true;
                    widget->select(charId, 1);
                }
            }
        }

        if (charSelected && sel)
            sel->selBox |= Box3(xx,yy,z, 1, hh, 0);

        if (qc == '\n')
        {
            x = 0;
            y -= h;
        }
        else
        {
            x += w;
        }
    }

    where->offset = Point3(x, y, z);
}


void TextSpan::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Draw and identify the bounding boxes for the various characters
// ----------------------------------------------------------------------------
{
    Widget *widget = where->Display();
    Point3 pos = where->offset;
    text str = +value;
    QFontMetricsF fm(font);
    scale h = fm.height();
    coord x = pos.x;
    coord y = pos.y;
    coord z = pos.z;
    coord descent = fm.descent();
    coord leading = fm.leading();

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = 0; i < max; i = XL::Utf8Next(str, i))
    {
        QChar qc = QChar(XL::Utf8Code(str, i));
        float w = qc == '\n' ? 3 : fm.width(qc);

        coord xx = x + fm.leftBearing(qc);
        coord yy = y - descent - leading;
        coord ww = w;
        coord hh = h + leading;

        coord array[4][3] =
        {
            { xx,      yy,      z },
            { xx + ww, yy,      z },
            { xx + ww, yy + hh, z },
            { xx,      yy + hh, z }
        };

        glLoadName(widget->newId());
        glVertexPointer(3, GL_DOUBLE, 0, array);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_QUADS, 0, 4);
        glDisableClientState(GL_VERTEX_ARRAY);

        if (qc == '\n')
        {
            x = 0;
            y -= h;
        }
        else
        {
            x += w;
        }
    }

    where->offset = Point3(x, y, z);
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



// ============================================================================
//
//   A text selection identifies a range of text being edited
//
// ============================================================================

TextSelect::TextSelect(Widget *w)
// ----------------------------------------------------------------------------
//   Constructor initializes an empty text range
// ----------------------------------------------------------------------------
    : Activity("Text selection", w), anchor(0), start(0), end(0)
{
    Widget::selection_map::iterator i, last = w->selection.end();
    for (i = w->selection.begin(); i != last; i++)
    {
        uint id = (*i).first;
        if (!start)
            start = end = anchor = id;
        else if (start > id)
            start = id;
        else if (end < id)
            end = id;
    }
}


Activity *TextSelect::Display()
// ----------------------------------------------------------------------------
//   Display the text selection
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *TextSelect::Idle()
// ----------------------------------------------------------------------------
//   Idle activity
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *TextSelect::Click(uint button, bool down, int x, int y)
// ----------------------------------------------------------------------------
//   Selection of text
// ----------------------------------------------------------------------------
{
    if (button & Qt::LeftButton)
        return MouseMove(x, y, down);
    return next;
}


Activity *TextSelect::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Move text selection
// ----------------------------------------------------------------------------
{
    if (!active)
        return next;

    y = widget->height() - y;
    Box rectangle(x, y, 1, 1);

    // Create the select buffer and switch to select mode
    GLuint *buffer = new GLuint[4 * widget->capacity];
    glSelectBuffer(4 * widget->capacity, buffer);
    glRenderMode(GL_SELECT);

    // Adjust viewport for rendering
    widget->setup(widget->width(), widget->height(), &rectangle);

    // Initialize names
    glInitNames();
    glPushName(0);

    // Run the programs, which detects selected items
    widget->identifySelection();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names
    widget->selection = widget->savedSelection;
    int hits = glRenderMode(GL_RENDER);
    GLuint selected = 0;
    if (hits > 0)
    {
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            uint size = ptr[0];
            selected = ptr[3];
            if (selected)
            {
                if (selected < anchor)
                {
                    start = selected;
                    end = anchor;
                }
                else
                {
                    start = anchor;
                    end = selected;
                }
                break;
            }
            ptr += 3 + size;
        }

        widget->selection.clear();
        for (uint i = start; i <= end; i++)
            widget->selection[i] = 1;
    }
    delete[] buffer;

    // Need a refresh
    widget->refresh();

    // Let a possible selection do its own stuff
    return next;
}

TAO_END
