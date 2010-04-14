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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "attributes.h"
#include "shapes.h"
#include "activity.h"
#include "coords3d.h"
#include "tree.h"
#include <QFont>

TAO_BEGIN

struct TextSpan : Shape
// ----------------------------------------------------------------------------
//    A contiguous run of glyphs
// ----------------------------------------------------------------------------
{
    TextSpan(XL::Text *source, const QFont &font, uint start = 0, uint end = ~0)
        : Shape(), source(source), font(font), start(start), end(end) {}
    virtual void        Draw(Layout *where);
    virtual void        DrawSelection(Layout *where);
    virtual void        Identify(Layout *where);
    virtual void        Draw(GraphicPath &path);
    virtual Box3        Bounds();
    virtual Box3        Space();
    virtual TextSpan *  Break(BreakOrder &order);
    virtual scale       TrailingSpaceSize();

public:
    XL::Text *          source;
    QFont               font;
    uint                start, end;
};


struct TextSelect : Activity
// ----------------------------------------------------------------------------
//   A text selection (contiguous range of characters)
// ----------------------------------------------------------------------------
{
    TextSelect(Widget *w);

    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Key(text key);
    virtual Activity *  Click(uint button, bool down, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

    // Mark and point have roughly the same meaning as in GNU Emacs
    void                moveTo(uint pos)        { mark = point = pos; }
    void                select(uint mk, uint pt){ mark = mk; point = pt; }
    uint                start() { return mark < point ? mark : point; }
    uint                end()   { return mark < point ? point : mark; }
    bool                hasSelection()          { return mark != point; }
    void                updateSelection();

    uint                mark, point;
    int                 direction;
    Box3                selBox;
    text                replacement;
    bool                replace;
};


TAO_END

#endif // TEXT_DRAWING_H
