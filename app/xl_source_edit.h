#ifndef XL_SOURCE_EDIT_H
#define XL_SOURCE_EDIT_H
// *****************************************************************************
// xl_source_edit.h                                                Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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
