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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "renderer.h"
#include <QTextEdit>

namespace Tao {

class XLHighlighter;


class XLSourceEdit : public QTextEdit
// ----------------------------------------------------------------------------
//   XL source editor
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    XLSourceEdit(QWidget *parent = 0);

    void   setPlainTextKeepCursor(const QString &txt);
    void   setXLNames(const QStringList &names);
    void   setSelectedRanges(const XL::stream_ranges &ranges);

protected:
    virtual bool   event(QEvent *e);
    virtual void   keyPressEvent(QKeyEvent *e);

    bool           handleTabKey(QKeyEvent *e);
    bool           handleShiftTabKey(QKeyEvent *e);

private:
    XLHighlighter *    highlighter;
    XL::stream_ranges  selected;
};

}

#endif // XL_SOURCE_EDIT_H
