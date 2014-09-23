#ifndef TEXT_DRAWING_H
#define TEXT_DRAWING_H
// ****************************************************************************
//  text_drawing.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Rendering of text
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "attributes.h"
#include "shapes.h"
#include "selection.h"
#include "coords3d.h"
#include "tree.h"
#include <QFont>
#include <QTextCursor>

TAO_BEGIN

struct GlyphCache;
struct GlyphCacheEntry;

struct TextSplit : Shape
// ----------------------------------------------------------------------------
//    A contiguous run of glyphs that where split at a word/sentence boundary
// ----------------------------------------------------------------------------
{
    TextSplit(Text *source, uint start = 0, uint end = ~0);
    virtual ~TextSplit();

    virtual void        Draw(Layout *where);
    virtual void        Identify(Layout *where);
    virtual Box3        Bounds(Layout *where);
    virtual Box3        Space(Layout *where);
    virtual bool        Paginate(PageLayout *);
    virtual scale       TrailingSpaceSize(Layout *where);
    virtual void        Draw(GraphicPath &path, Layout *where);
    virtual void        Clear();

    friend std::ostream &operator <<(std::ostream &, TextSplit &);

protected:
    virtual void        DrawCached(Layout *where);
    virtual void        DrawDirect(Layout *where);
    void                DrawSelection(Layout *where);
    int                 PerformEditOperation(Widget *w, uint i);
    void                PerformInsertOperation(Layout * l,
                                               Widget * widget,
                                               uint     position);

    // Quads management
    typedef std::vector<Point3> quads_t;
    typedef std::vector<Point>  texCoords_t;
    void AddGlyph(coord x, coord y, coord z,
                  GlyphCacheEntry &entry, GlyphCache &glyphs,
                  quads_t &quads, texCoords_t &texCoords);
    void DrawGlyphs(Layout *where, quads_t &quads, texCoords_t &texCoords);

public:
    Text_p              source;
    uint                start, end;
    static bool         cacheEnabled;
};


struct TextSplitRTL : TextSplit
// ----------------------------------------------------------------------------
//   Right-to-left text split, with separated characters (Hebrew)
// ----------------------------------------------------------------------------
{
    TextSplitRTL(Text *source, uint start = 0, uint end = ~0)
        : TextSplit(source, start, end) {}

    virtual bool        IsRTL() { return true; }

protected:
    virtual void        DrawCached(Layout *where);
    virtual void        DrawDirect(Layout *where);
};


struct TextSplitArabic : TextSplitRTL
// ----------------------------------------------------------------------------
//   Right-to-left text split with merged words (Arabic)
// ----------------------------------------------------------------------------
{
    TextSplitArabic(Text *source, uint start = 0, uint end = ~0)
        : TextSplitRTL(source, start, end) {}

protected:
    virtual void        DrawCached(Layout *where);
    virtual void        DrawDirect(Layout *where);

    virtual Box3        Bounds(Layout *where);
    virtual Box3        Space(Layout *where);
};


struct TextUnit : TextSplit
// ----------------------------------------------------------------------------
//    A contiguous run of glyphs which may contain spaces & separators
// ----------------------------------------------------------------------------
{
    TextUnit(Text *source, uint start = 0, uint end = ~0);
    virtual ~TextUnit();
    virtual bool        Paginate(PageLayout *);
    virtual void        Clear();
    virtual void        ClearCaches();

public:
    typedef std::vector<TextSplit *> TextSplits;
    TextSplits          splits;
    Drawings            caches;
};


struct TextFormulaEditInfo : XL::Info
// ----------------------------------------------------------------------------
//    Record the text format for a text formula while editing it
// ----------------------------------------------------------------------------
{
    TextFormulaEditInfo(XL::Text *s, uint id): source(s), order(id) {}
    Text_p              source;
    uint                order;
};


struct TextFormula : TextSplit
// ----------------------------------------------------------------------------
//   Like a text span, but for an evaluated value
// ----------------------------------------------------------------------------
{
    TextFormula(XL::Prefix *self, Widget *wid, uint start = 0, uint end = ~0)
        : TextSplit(NULL, start, end), self(self), widget(wid)
    {
        source = Format(self);
    }
    Text *              Format(Prefix *value);
    bool                Validate(Text *source, Widget *widget);

    virtual void        DrawSelection(Layout *where);
    virtual void        Identify(Layout *where);

public:
    Prefix_p            self;
    Widget *            widget;
    static uint         formulas, shows;
};


struct TextValue : TextUnit
// ----------------------------------------------------------------------------
//   Like a text span, but for the result of an evaluation
// ----------------------------------------------------------------------------
{
    TextValue(XL::Tree *value, Widget *wid, uint start = 0, uint end = ~0)
        : TextUnit(NULL, start, end), value(value), widget(wid)
    {
        source = Format(value);
    }
    Text *              Format(XL::Tree *value);
    bool                Validate(Text *source, Widget *widget);

    virtual void        DrawSelection(Layout *where);
    virtual void        Identify(Layout *where);

public:
    Tree_p              value;
    Widget *            widget;
    static uint         values, shows;
};


struct TextSelect : Identify
// ----------------------------------------------------------------------------
//   A text selection (contiguous range of characters)
// ----------------------------------------------------------------------------
{
    TextSelect(Widget *w);

    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Key(text key);
    virtual Activity *  Edit(text key);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

    // Mark and point have roughly the same meaning as in GNU Emacs
    void                moveTo(uint pos)        { mark = point = pos; }
    void                select(uint mk, uint pt){ mark = mk; point = pt; }
    uint                start() { return mark < point ? mark : point; }
    uint                end()   { return mark < point ? point : mark; }
    uint                length() { return mark > point ? mark-point : point-mark; }
    bool                hasSelection()          { return mark != point; }
    void                updateSelection();
    bool                needsPositions()        { return direction >= Up; }
    void                processLineBreak();
    void                processChar(uint id, coord x, bool selected, uint code);

    enum Direction      { None, Mark, Left, Right, Up, Down };
    uint                mark, point, previous, last, textBoxId;
    Direction           direction;
    coord               targetX;
    Box3                selBox;
    Box3                formulaBox;
    text                replacement;
    bool                replace;
    bool                textMode;
    bool                pickingLineEnds;
    bool                pickingUpDown;
    bool                movePointOnly;
    uint                formulaMode;

    QTextCursor         cursor;
    Tree_p              replacement_tree;
    bool                inSelection;
};





// ============================================================================
//
//   Entering text drawing in the symbols table
//
// ============================================================================

extern void EnterTextDrawing();
extern void DeleteTextDrawing();

TAO_END

#endif // TEXT_DRAWING_H
