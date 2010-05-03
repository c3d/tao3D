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
    bool hasTexture = setTexture(where);
    GlyphCache &glyphs = widget->glyphs();
    bool tooBig = where->font.pointSize() > (int) glyphs.maxFontSize;
    bool debugForceDirect = widget->lastModifiers() & Qt::ShiftModifier;
    if (!hasLine && !hasTexture && !tooBig && !debugForceDirect)
        DrawCached(where, false);
    else
        DrawDirect(where);
}


void TextSpan::DrawCached(Layout *where, bool identify)
// ----------------------------------------------------------------------------
//   Draw text span using cached textures
// ----------------------------------------------------------------------------
{
    Widget     *widget = where->Display();
    GlyphCache &glyphs = widget->glyphs();
    Point3      pos    = where->offset;
    text        str    = source->value;
    QFont      &font   = where->font;
    coord       x      = pos.x;
    coord       y      = pos.y;
    coord       z      = pos.z;

    GlyphCache::GlyphEntry  glyph;
    std::vector<Point3>     quads;
    std::vector<Point>      texCoords;

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = start; i < max && i < end; i = XL::Utf8Next(str, i))
    {
        uint  unicode  = XL::Utf8Code(str, i);
        bool  newLine  = unicode == '\n';

        // Find the glyph in the glyph cache
        if (!glyphs.Find(font, unicode, glyph, false))
        {
            // Try to create the glyph
            if (!glyphs.Find(font, unicode, glyph, true))
                continue;
        }

        if (!newLine)
        {
            // Enter the geometry coordinates
            coord charX1 = x + glyph.bounds.lower.x;
            coord charX2 = x + glyph.bounds.upper.x;
            coord charY1 = y - glyph.bounds.lower.y;
            coord charY2 = y - glyph.bounds.upper.y;
            quads.push_back(Point3(charX1, charY1, z));
            quads.push_back(Point3(charX2, charY1, z));
            quads.push_back(Point3(charX2, charY2, z));
            quads.push_back(Point3(charX1, charY2, z));

            // Enter the texture coordinates
            Point &texL = glyph.texture.lower;
            Point &texU = glyph.texture.upper;
            texCoords.push_back(Point(texL.x, texL.y));
            texCoords.push_back(Point(texU.x, texL.y));
            texCoords.push_back(Point(texU.x, texU.y));
            texCoords.push_back(Point(texL.x, texU.y));
        }

        // Advance to next character
        if (newLine)
        {
            scale height = glyphs.Ascent(font) + glyphs.Descent(font) + 1;
            scale spacing = height + glyphs.Leading(font);
            x = 0;
            y -= spacing;
        }
        else
        {
            x += glyph.advance;
        }
    }

    // Check if there's anything to draw
    uint count = quads.size();
    if (count)
    {
        if (identify)
        {
            // Draw a list of rectangles with the textures
            glVertexPointer(3, GL_DOUBLE, 0, &quads[0].x);
            glEnableClientState(GL_VERTEX_ARRAY);
            for (uint i = 0; i < count; i += 4)
            {
                glLoadName(widget->newCharId() | Widget::CHAR_ID_BIT);
                glDrawArrays(GL_QUADS, i, 4);
            }
            glDisableClientState(GL_VERTEX_ARRAY);
            if (where->id != ~0U)
                glLoadName(where->id);
        }
        else if (setFillColor(where))
        {
            // Bind the glyph texture
            glBindTexture(GL_TEXTURE_2D, glyphs.Texture());
            GLenum blur = GL_LINEAR;
            if (!where->hasPixelBlur &&
                font.pointSizeF() < glyphs.minFontSizeForAntialiasing)
                blur = GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blur);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_MULTISAMPLE);

            // Draw a list of rectangles with the textures
            glVertexPointer(3, GL_DOUBLE, 0, &quads[0].x);
            glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0].x);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glDrawArrays(GL_QUADS, 0, count);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
    QString str = +source->value.substr(start, end - start);
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
    Widget     *widget       = where->Display();
    GlyphCache &glyphs       = widget->glyphs();
    Point3      pos          = where->offset;
    text        str          = source->value;
    QFont      &font         = where->font;
    coord       x            = pos.x;
    coord       y            = pos.y;
    coord       z            = pos.z;
    uint        first        = start;
    scale       textWidth    = 0;
    TextSelect *sel          = widget->textSelection();
    GLuint      charId       = 0;
    bool        charSelected = false;
    scale       height       = glyphs.Ascent(font) + glyphs.Descent(font) + 1;
    GlyphCache::GlyphEntry  glyph;

    // Loop over all characters in the text span
    uint i, next, max = str.length();
    for (i = start; i < max && i < end; i = next)
    {
        uint unicode = XL::Utf8Code(str, i);
        charId = widget->newCharId();
        charSelected = widget->charSelected();
        next = XL::Utf8Next(str, i);

        // Create a text selection if we need one
        if (charSelected && !sel && where->id)
            sel = new TextSelect(widget);

        // Fetch data about that glyph
        if (!glyphs.Find(font, unicode, glyph, false))
            continue;

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
                coord charX = x + glyph.bounds.lower.x;
                coord charY = y - glyph.bounds.upper.y;
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
                        source->value.replace(i, eos-i, rpl);
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
                    sel->selBox |= Box3(charX,charY,z, 1, height, 0);
                } // if(charSelected)
            } // if (charSelected || upDown)
        } // if(sel)

        // Advance to next character
        if (unicode == '\n')
        {
            scale spacing = height + glyphs.Leading(font);
            x = 0;
            y -= spacing;
            textWidth = 0;
            first = i;
        }
        else
        {
            x += glyph.advance;
            textWidth += glyph.advance;
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
            source->value.replace(i, eos-i, rpl);
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
    DrawCached(where, true);
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

    QString str = +source->value.substr(start, end - start);
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


Box3 TextSpan::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the smallest box that surrounds the text
// ----------------------------------------------------------------------------
{
    Widget     *widget = where->Display();
    GlyphCache &glyphs = widget->glyphs();
    text        str    = source->value;
    QFont      &font   = where->font;
    Box3        result;
    coord       x      = 0;
    coord       y      = 0;
    coord       z      = 0;

    GlyphCache::GlyphEntry  glyph;

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = start; i < max && i < end; i = XL::Utf8Next(str, i))
    {
        uint  unicode  = XL::Utf8Code(str, i);
        bool  newLine  = unicode == '\n';

        // Find the glyph in the glyph cache
        if (!glyphs.Find(font, unicode, glyph, true))
            continue;

        if (!newLine)
        {
            // Enter the geometry coordinates
            coord charX1 = x + glyph.bounds.lower.x;
            coord charX2 = x + glyph.bounds.upper.x;
            coord charY1 = y - glyph.bounds.lower.y;
            coord charY2 = y - glyph.bounds.upper.y;
            result |= Point3(charX1, charY1, z);
            result |= Point3(charX2, charY2, z);
        }

        // Advance to next character
        if (newLine)
        {
            scale height = glyphs.Ascent(font) + glyphs.Descent(font) + 1;
            scale spacing = height + glyphs.Leading(font);
            x = 0;
            y -= spacing;
        }
        else
        {
            x += glyph.advance;
        }
    }

    return result;
}


Box3 TextSpan::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Return the box that surrounds the text, including leading
// ----------------------------------------------------------------------------
{
    Widget     *widget  = where->Display();
    GlyphCache &glyphs  = widget->glyphs();
    text        str     = source->value;
    QFont      &font    = where->font;
    Box3        result;
    scale       ascent  = glyphs.Ascent(font);
    scale       descent = glyphs.Descent(font);
    scale       leading = glyphs.Leading(font);
    coord       x      = 0;
    coord       y      = 0;
    coord       z      = 0;

    GlyphCache::GlyphEntry  glyph;

    // Loop over all characters in the text span
    uint i, max = str.length();
    for (i = start; i < max && i < end; i = XL::Utf8Next(str, i))
    {
        uint  unicode  = XL::Utf8Code(str, i);
        bool  newLine  = unicode == '\n';

        // Find the glyph in the glyph cache
        if (!glyphs.Find(font, unicode, glyph, true))
            continue;

        if (!newLine)
        {
            // Enter the geometry coordinates
            coord charX1 = x + glyph.bounds.lower.x;
            coord charX2 = x + glyph.bounds.upper.x;
            coord charY1 = y - glyph.bounds.lower.y;
            coord charY2 = y - glyph.bounds.upper.y;
            result |= Point3(charX1, charY1, z);
            result |= Point3(charX2, charY2, z);
            result |= Point3(charX1, y + ascent, z);
            result |= Point3(charX1 + glyph.advance, y - descent - leading, z);
        }

        // Advance to next character
        if (newLine)
        {
            x = 0;
            y -= ascent + descent + leading + 1;
        }
        else
        {
            x += glyph.advance;
        }
    }

    return result;
}


TextSpan *TextSpan::Break(BreakOrder &order)
// ----------------------------------------------------------------------------
//   If the text span contains a word or line break, cut there
// ----------------------------------------------------------------------------
{
    text str = source->value;
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
    Widget     *widget = where->Display();
    GlyphCache &glyphs = widget->glyphs();
    QFont      &font   = where->font;
    scale       result = 0;
    text        str    = source->value;
    uint        pos    = str.length();
    if (pos > end)
        pos = end;
    while (pos > start)
    {
        pos = XL::Utf8Previous(str, pos);
        QChar c = QChar(XL::Utf8Code(str, pos));
        if (!c.isSpace())
            break;

        // Find the glyph in the glyph cache
        GlyphCache::GlyphEntry  glyph;
        uint  unicode  = XL::Utf8Code(str, pos);
        if (!glyphs.Find(font, unicode, glyph, true))
            continue;

        result += glyph.advance;
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
