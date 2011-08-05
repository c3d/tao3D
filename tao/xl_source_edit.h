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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "renderer.h"
#include <QTextEdit>
#include <sstream>

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
    virtual ~XLSourceEdit();

    void   setXLNames(const QStringList &names);
    void   render(XL::Tree_p prog, std::set<XL::Tree_p>* selected = NULL);

protected:
    virtual bool   event(QEvent *e);
    virtual void   keyPressEvent(QKeyEvent *e);

    bool           handleTabKey(QKeyEvent *e);
    bool           handleShiftTabKey(QKeyEvent *e);
    void           setPlainTextKeepCursor(const QString &txt);
    void           setSelectedRanges(const XL::stream_ranges &ranges);
    QString        workaroundRendererBugs(QString txt);

private:
    XLHighlighter *    highlighter;
    XL::Renderer *     renderer;
    std::ostringstream rendererOut;
};

}

#endif // XL_SOURCE_EDIT_H
