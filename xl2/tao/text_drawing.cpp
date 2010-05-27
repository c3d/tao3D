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
#include "gl_keepers.h"
#include "runtime.h"

#include <GL/glew.h>
#include <QtOpenGL>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>
#include <sstream>

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
    uint dbgMod = (Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier);
    bool debugForceDirect = (widget->lastModifiers() & dbgMod) == dbgMod;

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
    Text *      ttree  = source;
    text        str    = ttree->value;
    bool        canSel = ttree->Position() != XL::Tree::NOWHERE;
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

        // Advance to next character
        if (newLine)
        {
            scale height = glyphs.Ascent(font) + glyphs.Descent(font);
            scale spacing = height + glyphs.Leading(font);
            x = 0;
            y -= spacing * glyph.scalingFactor;
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
                if (canSel)
                    glLoadName(widget->newCharId() | Widget::CHAR_ID_BIT);
                glDrawArrays(GL_QUADS, i, 4);
            }
            glDisableClientState(GL_VERTEX_ARRAY);
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
    Widget     *widget = where->Display();
    GlyphCache &glyphs = widget->glyphs();
    Point3      pos    = where->offset;
    Text *      ttree  = source;
    text        str    = ttree->value;
    bool        canSel = ttree->Position() != XL::Tree::NOWHERE;
    QFont      &font   = where->font;
    coord       x      = pos.x;
    coord       y      = pos.y;
    coord       z      = pos.z;
    scale       lw     = where->lineWidth;
    if (where->lineColor.alpha <= 0)
        lw = 0;

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
        if (!glyphs.Find(font, unicode, glyph, true, true, lw))
            continue;

        if (canSel)
            glLoadName(widget->newCharId() | Widget::CHAR_ID_BIT);
        GLMatrixKeeper save;
        glTranslatef(x, y, z);
        scale gscale = glyph.scalingFactor;
        glScalef(gscale, gscale, gscale);

        setTexture(where);
        if (setFillColor(where))
            glCallList(glyph.interior);
        if (setLineColor(where))
            glCallList(glyph.outline);

        // Advance to next character
        if (newLine)
        {
            scale height = glyphs.Ascent(font) + glyphs.Descent(font);
            scale spacing = height + glyphs.Leading(font);
            x = 0;
            y -= spacing * glyph.scalingFactor;
        }
        else
        {
            x += glyph.advance;
        }
    }

    where->offset = Point3(x, y, z);
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
    Text *      ttree        = source;
    text        str          = ttree->value;
    bool        canSel       = ttree->Position() != XL::Tree::NOWHERE;
    QFont      &font         = where->font;
    coord       x            = pos.x;
    coord       y            = pos.y;
    coord       z            = pos.z;
    uint        first        = start;
    scale       textWidth    = 0;
    TextSelect *sel          = widget->textSelection();
    GLuint      charId       = ~0;
    bool        charSelected = false;
    scale       ascent       = glyphs.Ascent(font);
    scale       descent      = glyphs.Descent(font);
    scale       height       = ascent + descent;
    GlyphCache::GlyphEntry  glyph;

    // Loop over all characters in the text span
    uint i, next, max = str.length();
    for (i = start; i < max && i < end; i = next)
    {
        uint unicode = XL::Utf8Code(str, i);
        if (canSel)
        {
            charId = widget->newCharId();
            charSelected = widget->charSelected();
        }
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
                coord charY = y;

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
                    scale sd = glyph.scalingFactor * descent;
                    scale sh = glyph.scalingFactor * height;
                    sel->selBox |= Box3(charX,charY - sd,z, 1, sh, 0);
                } // if(charSelected)

            } // if (charSelected || upDown)

            // Check if we are in a formula, if so display formula box
            if (sel->formulaMode)
            {
                coord charX = x + glyph.bounds.lower.x;
                coord charY = y;
                scale sd = glyph.scalingFactor * descent;
                scale sh = glyph.scalingFactor * height;
                sel->formulaBox |= Box3(charX,charY - sd,z, 1, sh, 0);
                sel->formulaMode--;
                if (!sel->formulaMode)
                {
                    glBlendFunc(GL_DST_COLOR, GL_ZERO);
                    text mode = "formula_highlight";
                    widget->drawSelection(sel->formulaBox, mode, 0);
                    sel->formulaBox.Empty();
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
            }
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

    if (sel && max <= end)
    {
        charId++;
        if (charId >= sel->start() && charId <= sel->end())
        {
            if (sel->replace)
            {
                text rpl = sel->replacement;
                if (rpl.length())
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
            scale sd = glyph.scalingFactor * descent;
            scale sh = glyph.scalingFactor * height;
            sel->selBox |= Box3(x,y - sd,z, 1, sh, 0);
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

        scale sa = ascent * glyph.scalingFactor;
        scale sd = descent * glyph.scalingFactor;
        scale sl = leading * glyph.scalingFactor;

        // Enter the geometry coordinates
        coord charX1 = x + glyph.bounds.lower.x;
        coord charX2 = x + glyph.bounds.upper.x;
        coord charY1 = y - glyph.bounds.lower.y;
        coord charY2 = y - glyph.bounds.upper.y;

        // Advance to next character
        if (newLine)
        {
            result |= Point3(charX1, y + sa, z);
            result |= Point3(charX1, y - sd - sl, z);

            scale height = glyphs.Ascent(font) + glyphs.Descent(font);
            scale spacing = height + glyphs.Leading(font);
            x = 0;
            y -= spacing * glyph.scalingFactor;
        }
        else
        {
            result |= Point3(charX1, charY1, z);
            result |= Point3(charX2, charY2, z);

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

        scale sa = ascent * glyph.scalingFactor;
        scale sd = descent * glyph.scalingFactor;
        scale sl = leading * glyph.scalingFactor;

        // Enter the geometry coordinates
        coord charX1 = x + glyph.bounds.lower.x;
        coord charX2 = x + glyph.bounds.upper.x;
        coord charY1 = y - glyph.bounds.lower.y;
        coord charY2 = y - glyph.bounds.upper.y;

        result |= Point3(charX1, charY1, z);
        result |= Point3(charX2, charY2, z);
        result |= Point3(charX1, y + sa, z);

        // Advance to next character
        if (newLine)
        {
            result |= Point3(charX1, y - sd - sl, z);
            x = 0;
            y -= sa + sd + sl;
        }
        else
        {
            result |= Point3(charX1 + glyph.advance, y - sd - sl, z);
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
    text        str    = source->value;
    uint        pos    = str.length();
    Box3        box;

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

        // Enter the geometry coordinates
        coord charX1 = glyph.bounds.lower.x;
        coord charX2 = glyph.bounds.upper.x;
        coord charY1 = glyph.bounds.lower.y;
        coord charY2 = glyph.bounds.upper.y;
        box |= Point3(charX1, charY1, 0);
        box |= Point3(charX2, charY2, 0);
        box |= Point3(charX1 + glyph.advance, charY1, 0);
    }

    scale result = box.Width();
    if (result < 0)
        result = 0;
    return result;
}



// ============================================================================
//
//    A text formula is used to display numerical / evaluated values
//
// ============================================================================

uint TextFormula::formulas = 0;
uint TextFormula::shows = 0;

XL::Text *TextFormula::Format(XL::Prefix *self)
// ----------------------------------------------------------------------------
//   Return a formatted value for the given value
// ----------------------------------------------------------------------------
{
    Tree *value = self->right;
    TextFormulaEditInfo *info = value->GetInfo<TextFormulaEditInfo>();
    formulas++;
    shows = 0;

    // Check if this formula is currently being edited, if so return its text
    if (info && info->order == formulas)
        return info->source;

    // If the value is a name, we evaluate it in its normal symbol table
    Name *name = value->AsName();
    Symbols *symbols = value->Symbols();
    if (name)
        if (Tree *named = symbols->Named(name->value, true))
            value = named;

    // Make sure we evaluate that in the formulas symbol table
    if (symbols != widget->formulaSymbols())
    {
        XL::TreeClone clone;
        value = value->Do(clone);
        value->SetSymbols(widget->formulaSymbols());
        if (name)
        {
            Tree *definition = symbols->Defined(name->value);
            if (definition)
                if (Infix *infix = definition->AsInfix())
                    infix->right = value;
        }
        else
        {
            self->right = value;
        }
    }

    // Evaluate the tree and turn it into a tree
    Tree *computed = xl_evaluate(value);
    return new XL::Text(*computed);
}


void TextFormula::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Detect if we edit a formula, if so create its FormulEditInfo
// ----------------------------------------------------------------------------
{
    Widget              *widget = where->Display();
    TextSelect          *sel    = widget->textSelection();
    XL::Prefix *         prefix = self;
    XL::Tree *           value  = prefix->right;
    TextFormulaEditInfo *info   = value->GetInfo<TextFormulaEditInfo>();
    uint                 selId  = widget->currentCharId() + 1;

    // Count formulas to identify them uniquely
    shows++;
    formulas = 0;

    // Check if formula is selected and we are not editing it
    if (sel && sel->textMode)
    {
        if (!info && widget->charSelected(selId))
        {
            // No info: create one

            // If the value is a name, we evaluate it in its normal symbol table
            Name *name = value->AsName();
            Symbols *symbols = value->Symbols();
            if (name)
                if (Tree *named = symbols->Named(name->value, true))
                    value = named;
            
            text edited = text(" ") + text(*value) + " ";
            Text *editor = new Text(edited, "\"", "\"", value->Position());
            info = new TextFormulaEditInfo(editor, shows);
            prefix->right->SetInfo<TextFormulaEditInfo>(info);

            // Update mark and point
            XL::Text *source = info->source;
            uint length = source->value.length();
            sel->point = selId;
            sel->mark = selId + length;

            widget->refresh();
        }
    }

    // Indicate how many characters we want to display as "formula"
    if (info && sel)
    {
        if (shows == info->order)
        {
            XL::Text *source = info->source;
            sel->formulaMode = source->value.length() + 1;
        }
        else
        {
            XL::Text *source = this->source;
            sel->formulaMode = source->value.length() + 1;
        }
    }

    TextSpan::DrawSelection(where);

    // Check if the cursor moves out of the selection - If so, validate
    if (info &&info->order == shows)
    {
        XL::Text *source = info->source;
        uint length = source->value.length();

        if (!sel || (sel->mark == sel->point &&
                     (sel->point < selId || sel->point > selId + length)))
        {
            if (Validate(info->source, widget))
            {
                if (sel && sel->point > selId + length)
                {
                    sel->point -= length;
                    sel->mark -= length;
                }
            }
        }
    }
    else if (!info && sel && selId >= sel->start() && selId <= sel->end())
    {
        // First run, make sure we return here to create the editor
        widget->refresh();
    }
}


void TextFormula::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Give one ID to the whole formula so that we can click on it
// ----------------------------------------------------------------------------
{
    XL::Prefix *         prefix  = self->AsPrefix();
    XL::Tree *           value   = prefix->right;
    TextFormulaEditInfo *info    = value->GetInfo<TextFormulaEditInfo>();
    if (!info)
    {
        Widget *widget  = where->Display();
        uint    selId = widget->currentCharId() + 1;
        glLoadName(selId | Widget::CHAR_ID_BIT);
    }
    TextSpan::Identify(where);
}


bool TextFormula::Validate(XL::Text *source, Widget *widget)
// ----------------------------------------------------------------------------
//   Check if we can parse the input. If so, update self
// ----------------------------------------------------------------------------
{
    std::istringstream  input(source->value);
    XL::Syntax          syntax (XL::MAIN->syntax);
    XL::Positions      &positions = XL::MAIN->positions;
    XL::Errors         &errors    = XL::MAIN->errors;
    XL::Parser          parser(input, syntax,positions,errors);
    Tree *              newTree = parser.Parse();
    if (newTree)
    {
        newTree->SetSymbols(widget->formulaSymbols());

        XL::Prefix *prefix = self;
        XL::Name *name = prefix->right->AsName();
        if (name)
        {
            XL::Symbols *symbols = self->Symbols();
            Tree *definition = symbols->Defined(name->value);
            if (Infix *infix = definition->AsInfix())
                infix->right = newTree;
        }
        else
        {
            prefix->right = newTree;
        }
        widget->reloadProgram();
        widget->markChanged("Replaced formula");
        return true;
    }
    return false;
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
      textMode(false),
      pickingUpDown(false), movePointOnly(false),
      formulaMode(false),
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
    formulaMode = false;
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
