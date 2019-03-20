#ifndef XL_SOURCE_EDIT_H
#define XL_SOURCE_EDIT_H
// ****************************************************************************
//  xl_source_edit.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//     A text editor widget for XL source code.
//     Supports:
//       - Syntax highlighting
//       - Input as UTF-8 text or XL tree
//
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "renderer.h"
#include <QTextEdit>
#include <sstream>
#include <set>

namespace Tao {

class XLHighlighter;


struct XLSourceEdit : QTextEdit
// ----------------------------------------------------------------------------
//   XL source editor
// ----------------------------------------------------------------------------
{
private:
    Q_OBJECT

public:
    XLSourceEdit(QWidget *parent = 0);
    virtual ~XLSourceEdit();

    void   highlightNames(int index, std::set<text> &set);
    void   render(XL::Tree_p prog, std::set<XL::Tree_p>* selected = NULL);

protected:
    virtual bool   event(QEvent *e);
    virtual void   keyPressEvent(QKeyEvent *e);

    bool           handleTabKey(QKeyEvent *e);
    bool           handleShiftTabKey(QKeyEvent *e);
    bool           handleF1Key(QKeyEvent *e);
    void           setPlainTextKeepCursor(const QString &txt);
    void           setSelectedRanges(const XL::stream_ranges &ranges);

private:
    XLHighlighter *    highlighter;
    XL::Renderer *     renderer;
    std::ostringstream rendererOut;
};

}

#endif // XL_SOURCE_EDIT_H
