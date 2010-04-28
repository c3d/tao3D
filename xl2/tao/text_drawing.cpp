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
#include "tao_utf8.h"
#include "drag.h"
#include "glyph_cache.h"
#include "frame.h"

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
    Widget *widget = where->Display();
    bool hasLine = setLineColor(where);
    if (!hasLine && widget->lastModifiers() & Qt::ShiftModifier)
        DrawCached(where);
    else
        DrawDirect(where);
}


void TextSpan::DrawCached(Layout *where)
// ----------------------------------------------------------------------------
//   Draw text span using cached textures
// ----------------------------------------------------------------------------
{
    // If we have a texture, we need to draw "slowly" (polygons)
    bool hasTexture = setTexture(where);
    bool hasFillColor = setFillColor(where);
    if (hasTexture)
        DrawDirect(where);

    Widget *widget = where->Display();
    GlyphCache &glyphs = widget->glyphs();
    Point3 pos = where->offset;
    text str = source.Value();
    QFont &font = where->font;
    QFontMetricsF fm(font);
    scale h = fm.height();
    coord x = pos.x;
    coord y = pos.y;
    coord z = pos.z;
    scale ascent = fm.ascent();
    scale descent = fm.descent();
    scale spacing = h + fm.leading();
    uint  lastCode = 0;
    scale lastW = 0;

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = start; i < max && i < end; i = XL::Utf8Next(str, i))
    {
        uint  unicode  = XL::Utf8Code(str, i);
        QChar qc       = QChar(unicode);
        bool  newLine  = qc == '\n';
        float w        = newLine ? 3 : fm.width(qc);
        bool  mustDraw = !newLine && hasFillColor;

        // Check texture in the texture cache
        if (mustDraw)
        {
            if (font.pointSize() < GlyphCache::maxFontSize)
            {
                // Lookup texture in glyph cache
                uint texture = glyphs.texture(font, unicode);
                if (!texture)
                {
                    // Draw the glyph in a QImage
                    uint ww = w + 0.9;
                    uint hh = h + 0.9;
                    QImage image(QSize(ww, hh), QImage::Format_ARGB32);
                    image.fill(0);
                    
                    QPainter painter(&image);
                    painter.setFont(font);
                    painter.setBrush(Qt::transparent);
                    painter.setPen(Qt::white);
                    painter.drawText(0, ascent+1, QString(qc));
                    painter.end();

                    // Convert to a texture
                    QImage texImg = QGLWidget::convertToGLFormat(image);
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                                 texImg.width(), texImg.height(), 0, GL_RGBA,
                                 GL_UNSIGNED_BYTE, texImg.bits());

                    // Save the texture in the glyph cache
                    glyphs.enter(font, unicode, texture);
                }

                // Draw the rectangle with a texture in it
                coord charX = x + fm.leftBearing(qc);
                coord lineY = y - descent;

                coord array[4][3] =
                {
                    { charX,      lineY,      z },
                    { charX + w,  lineY,      z },
                    { charX + w,  lineY + h,  z },
                    { charX,      lineY + h,  z }
                };
                static GLint txCoords[4][2] =
                {
                    { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }
                };
                
                // Bind the glyph texture
                glBindTexture(GL_TEXTURE_2D, texture);
                GLenum blur = where->hasPixelBlur ? GL_LINEAR : GL_NEAREST;
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blur);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur);
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_MULTISAMPLE);

                // Draw a rectangle with the texture
                glVertexPointer(3, GL_DOUBLE, 0, array);
                glTexCoordPointer(2, GL_INT, 0, txCoords);
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glDrawArrays(GL_QUADS, 0, 4);
                glDisableClientState(GL_VERTEX_ARRAY);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
            else
            {
                // Too big, draw directly
                QPainterPath path;
                path.addText(x, -y, font, QString(qc));
                GraphicPath::Draw(where, path, GLU_TESS_WINDING_ODD, -1);
            }
        } // mustDraw

        if (qc == '\n')
        {
            x = 0;
            lastW = 0;
            lastCode = 0;
            y -= spacing;
        }
        else
        {
            if (lastCode)
            {
                QString twoChars = QString(QChar(lastCode)) + QChar(unicode);
                scale twoWidth = fm.width(twoChars);
                x += twoWidth - lastW;
            }
            else
            {
                x += w;
            }
            lastCode = unicode;
            lastW = w;
        }
    }

    where->offset = Point3(x, y, z);
}


void TextSpan::DrawDirect(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the given text directly using Qt paths
// ----------------------------------------------------------------------------
{
    Point3 position = where->offset;
    QPainterPath path;
    QString str = +source.Value().substr(start, end - start);
    QFont &font = where->font;
    QFontMetricsF fm(font);
    Widget *widget = where->Display();
    widget->newCharId(str.length());
    scale leading = fm.leading();
    scale height = fm.height();
    scale spacing = leading + height;

    int index = str.indexOf(QChar('\n'));
    while (index >= 0)
    {
        QString fragment = str.left(index);
        path.addText(position.x, -position.y, font, fragment);
        position.x = 0;
        position.y -= spacing;
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
//   Draw the selection for any selected character
// ----------------------------------------------------------------------------
//   Note that when drawing the rectangles, we don't include leading
//   (space between lines) in the rectangle being drawn.
//   For selection purpose we do include it to make it easier to click
{
    Widget *widget = where->Display();
    Point3 pos = where->offset;
    text str = source.Value();
    QFont &font = where->font;
    QFontMetricsF fm(font);
    coord descent = fm.descent();
    coord leading = fm.leading();
    scale h = fm.height() + leading;
    coord x = pos.x;
    coord y = pos.y;
    coord z = pos.z;
    uint  first = start;

    // Loop over all characters in the text span
    TextSelect *sel = widget->textSelection();
    GLuint charId = 0;
    bool charSelected = false;
    uint i, next, max = str.length();
    for (i = start; i < max && i < end; i = next)
    {
        charId = widget->newCharId();
        charSelected = widget->charSelected();
        next = XL::Utf8Next(str, i);

        // Create a text selection if we need one
        if (charSelected && !sel && where->id)
            sel = new TextSelect(widget);

        if (sel)
        {
            // Mark characters in selection range
            if (!charSelected &&
                charId >= sel->start() && charId <= sel->end())
            {
                charSelected = true;
                widget->selectChar(charId, 1);
            }

            // Check up and down keys
            if (charSelected || sel->needsPositions())
            {
                scale sw = fm.width(+str.substr(first, i-first));
                coord charX = x + sw;
                coord lineY = y - descent - leading;
                sel->newChar(charX, charSelected);

                if (charSelected)
                {
                    // Edit text in place if we have an editing request
                    if (sel->replace)
                    {
                        text rpl = sel->replacement;
                        uint eos = i;
                        if (sel->point != sel->mark)
                        {
                            eos = next;
                            if (sel->point > sel->mark)
                                sel->point--;
                            else
                                sel->mark--;
                        }
                        source.Value().replace(i, eos-i, rpl);
                        sel->replacement = "";
                        uint length = XL::Utf8Length(rpl);
                        sel->point += length;
                        sel->mark += length;
                        if (!length)
                        {
                            next = i;
                            max--;
                        }
                        if (sel->point == sel->mark)
                            sel->replace = false;
                    }
                    sel->selBox |= Box3(charX,lineY,z, 1, h, 0);
                } // if(charSelected)
            } // if (charSelected || upDown)
        } // if(sel)

        if (str[i] == '\n')
        {
            x = 0;
            y -= h;
            first = i;
        }
    }

    if (sel && sel->replace && max <= end)
    {
        charId++;
        text rpl = sel->replacement;
        if (charId >= sel->start() && charId <= sel->end() && rpl.length())
        {
            uint eos = i;
            if (sel->point != sel->mark)
            {
                eos = next;
                if (sel->point > sel->mark)
                    sel->point--;
                else
                    sel->mark--;
            }
            source.Value().replace(i, eos-i, rpl);
            sel->replacement = "";
            uint length = XL::Utf8Length(rpl);
            sel->point += length;
            sel->mark += length;
            if (sel->point == sel->mark)
                sel->replace = false;
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
    text str = source.Value();
    QFont &font = where->font;
    QFontMetricsF fm(font);
    scale h = fm.height();
    coord x = pos.x;
    coord y = pos.y;
    coord z = pos.z;
    coord descent = fm.descent();
    coord leading = fm.leading();

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = start; i < max && i < end; i = XL::Utf8Next(str, i))
    {
        QChar qc = QChar(XL::Utf8Code(str, i));
        float w = qc == '\n' ? 3 : fm.width(qc);

        coord charX = x + fm.leftBearing(qc);
        coord lineY = y - descent - leading;
        coord ww = w;
        coord hh = h + leading;

        coord array[4][3] =
        {
            { charX,      lineY,      z },
            { charX + ww, lineY,      z },
            { charX + ww, lineY + hh, z },
            { charX,      lineY + hh, z }
        };

        glLoadName(widget->newCharId() | Widget::CHAR_ID_BIT);
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


void TextSpan::Draw(GraphicPath &path, Layout *where)
// ----------------------------------------------------------------------------
//   Render a portion of text and advance by the width of the text
// ----------------------------------------------------------------------------
{
    Point3 position = path.position;
    QFont &font = where->font;
    QFontMetricsF fm(font);
    QPainterPath qt;

    QString str = +source.Value().substr(start, end - start);
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


Box3 TextSpan::Bounds(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the smallest box that surrounds the text
// ----------------------------------------------------------------------------
{
    QFontMetricsF fm(layout->font);
    QString       str = +source.Value().substr(start, end - start);
    QRectF        rect = fm.tightBoundingRect(str);
    return Box3(rect.x(), rect.height()+rect.y(), 0,
                rect.width(), rect.height(), 0);
}


Box3 TextSpan::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Return the box that surrounds the text, including leading
// ----------------------------------------------------------------------------
{
    QFont &       font = where->font;
    QFontMetricsF fm(font);
    QString       str = +source.Value().substr(start, end - start);
    coord         height      = fm.height();
    coord         descent     = fm.descent();
    coord         leading     = fm.leading();
    coord         width       = fm.width(str);
    coord         leftBearing = 0;
    if (str.length())
        leftBearing = fm.leftBearing(str[0]);
    return Box3(leftBearing, -descent-leading, 0, width, height+leading, 0);
}


TextSpan *TextSpan::Break(BreakOrder &order)
// ----------------------------------------------------------------------------
//   If the text span contains a word or line break, cut there
// ----------------------------------------------------------------------------
{
    text str = source.Value();
    uint i, max = str.length();
    for (i = start; i < max && i < end; i = XL::Utf8Next(str, i))
    {
        QChar c = QChar(XL::Utf8Code(str, i));
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
            uint next = XL::Utf8Next(str, i);
            TextSpan *result = (next < max && next < end)
                ? new TextSpan(source, next, end)
                : NULL;
            order = charOrder;
            end = next;
            return result;
        }
    }
    order = NoBreak;
    return NULL;
}


scale TextSpan::TrailingSpaceSize(Layout *where)
// ----------------------------------------------------------------------------
//   Return the size of all the spaces at the end of the value
// ----------------------------------------------------------------------------
{
    QFont &font = where->font;
    QFontMetricsF fm(font);
    scale result = 0;
    text str = source.Value();
    uint pos = str.length();
    if (pos > end)
        pos = end;
    while (pos > start)
    {
        pos = XL::Utf8Previous(str, pos);
        QChar c = QChar(XL::Utf8Code(str, pos));
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
    : Activity("Text selection", w),
      mark(0), point(0), direction(None), targetX(0),
      replacement(""), replace(false),
      textMode(false), pickingUpDown(false), movePointOnly(false),
      findingLayout(true)
{
    Widget::selection_map::iterator i, last = w->selection.end();
    for (i = w->selection.begin(); i != last; i++)
    {
        uint id = (*i).first;
        if (id & Widget::CHAR_ID_BIT)
        {
            id &= Widget::CHAR_ID_MASK;
            if (!mark)
                mark = point = id;
            else
                point = id;
        }
    }
}


Activity *TextSelect::Display()
// ----------------------------------------------------------------------------
//   Display the text selection
// ----------------------------------------------------------------------------
{
    findingLayout = false;
    return next;
}


Activity *TextSelect::Idle()
// ----------------------------------------------------------------------------
//   Idle activity
// ----------------------------------------------------------------------------
{
    return next;
}


Activity *TextSelect::Key(text key)
// ----------------------------------------------------------------------------
//    Perform activities on the text selection
// ----------------------------------------------------------------------------
{
    if (!textMode)
        return next;

    if (key == "Space")                 key = " ";
    else if (key == "Return")           key = "\n";
    else if (key == "Enter")            key = "\n";

    if (key == "Left")
    {
        moveTo(start() - !hasSelection());
        direction = Left;
    }
    else if (key == "Right")
    {
        moveTo(end() + !hasSelection());
        direction = Right;
    }
    else if (key == "Shift-Left")
    {
        point--;
        direction = Left;
    }
    else if (key == "Shift-Right")
    {
        point++;
        direction = Right;
    }
    else if (key == "Up")
    {
        mark = point = start();
        direction = Up;
        movePointOnly = false;
    }
    else if (key == "Down")
    {
        mark = point = end();
        direction = Down;
        movePointOnly = false;
    }
    else if (key == "Shift-Up")
    {
        direction = Up;
        movePointOnly = true;
    }
    else if (key == "Shift-Down")
    {
        direction = Down;
        movePointOnly = true;
    }
    else if (key == "Delete" || key == "Backspace")
    {
        replacement = "";
        replace = true;
        if (!hasSelection())
            point = (key == "Delete") ? point+1 : point-1;
        widget->markChanged("Deleted text");
        direction = Mark;
    }
    else if (XL::Utf8Length(key) == 1)
    {
        replacement = key;
        replace = true;
        if (hasSelection())
            widget->markChanged("Replaced text");
        else
            widget->markChanged("Inserted text");
        direction = Mark;
    }

    if (replace)
    {
        widget->updateSelection();
        mark = point;
        replace = false;
    }

    updateSelection();
    return next;
}


Activity *TextSelect::Click(uint button, uint count, int x, int y)
// ----------------------------------------------------------------------------
//   Selection of text
// ----------------------------------------------------------------------------
{
    if (button & Qt::LeftButton)
    {
        if (count)
        {
            mark = point = 0;
            if (count == 2)
                textMode = true;
            direction = Mark;
            return MouseMove(x, y, true);
        }
    }
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
    int hits = glRenderMode(GL_RENDER);
    GLuint selected = 0;
    GLuint manipulator = 0;
    if (hits > 0)
    {
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            uint size = ptr[0];
            if (size > 1)
            {
                manipulator = ptr[4];
                selected = ptr[3];
            }
            else if (!selected)
                selected = ptr[3];
            ptr += 3 + size;
        }

        findingLayout = true;
        if (manipulator)
        {
            textMode = false;
        }
        else if ((selected & Widget::CHAR_ID_BIT))
        {
            selected &= Widget::CHAR_ID_MASK;
            if (textMode)
            {
                if (mark)
                    point = selected;
                else
                    mark = point = selected;
                updateSelection();
            }
            else if (!mark)
            {
                mark = point = selected;
            }
        }
    }
    delete[] buffer;

    // If selecting a range of text, prevent the drag from moving us around
    if (textMode)
    {
        if (Drag *drag = widget->drag())
        {
            drag->x1 = drag->x2 = x;
            drag->y1 = drag->y2 = widget->height() - y;
        }
    }

    // Need a refresh
    widget->refresh();

    // Let a possible selection do its own stuff
    return next;
}


void TextSelect::updateSelection()
// ----------------------------------------------------------------------------
//   Update the selection of the widget based on mark and point
// ----------------------------------------------------------------------------
{
    widget->selection.clear();
    uint s = start(), e = end();
    for (uint i = s; i < e; i++)
        widget->selection[i | Widget::CHAR_ID_BIT] = 1;
    findingLayout = true;
    widget->refresh();
}


void TextSelect::newLine()
// ----------------------------------------------------------------------------
//   Mark the beginning of a new drawing line for Up/Down keys
// ----------------------------------------------------------------------------
{
    bool up = direction == Up;
    bool down = direction == Down;
    if (!up && !down)
        return;

    uint charId = widget->currentCharId();
    if (down)
    {
        // Current best position
        if (charId >= point)
        {
            if (pickingUpDown)
            {
                // What we had was the best position
                point = charId;
                if (!movePointOnly)
                    mark = point;
                direction = None;
                pickingUpDown = false;
                updateSelection();
            }
            else
            {
                // Best default position is start of line
                pickingUpDown = true;
            }
        }
        else
        {
            pickingUpDown = false;
        }
    }
    else // Up
    {
        if (charId < point)
        {
            if (pickingUpDown)
                previous = charId; // We are at right of previous line
            else
                pickingUpDown = true;
        }
    }
}


void TextSelect::newChar(coord x, bool selected)
// ----------------------------------------------------------------------------
//   Record a new character and deal with Up/Down keys
// ----------------------------------------------------------------------------
{
    bool up = direction == Up;
    bool down = direction == Down;
    if (!up && !down)
    {
        if (selected && direction != None)
            targetX = x;
        return;
    }

    uint charId = widget->currentCharId();
    if (down)
    {
        if (pickingUpDown && x >= targetX)
        {
            // We found the best position candidate: stop here
            point = charId;
            if (!movePointOnly)
                mark = point;
            direction = None;
            pickingUpDown = false;
            updateSelection();
        }
    }
    else // Up
    {
        if (pickingUpDown && charId < point && x >= targetX)
        {
            // We found the best position candidate
            previous = charId;
            pickingUpDown = false;
        }
        else if (charId >= point)
        {
            // The last position we had was the right one
            point = previous;
            if (!movePointOnly)
                mark = point;
            pickingUpDown = false;
            direction = None;
            updateSelection();
        }
    }
}

TAO_END
