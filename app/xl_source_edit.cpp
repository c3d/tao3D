// *****************************************************************************
// xl_source_edit.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//     XLSourceEdit implementation.
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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


#include "xl_source_edit.h"
#include "xl_highlighter.h"
#include "tao_utf8.h"
#include "assistant.h"
#include "context.h"
#include <QScrollBar>
#include <QEvent>
#include <QKeyEvent>
#include <QFileInfo>
#include <QTextDocumentFragment>

namespace Tao {

XLSourceEdit::XLSourceEdit(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create a source editor
// ----------------------------------------------------------------------------
    : QTextEdit(parent)
{
    QFont font("unifont");
    font.setPixelSize(16);
    setFont(font);
    highlighter = new XLHighlighter(document());
    setLineWrapMode(QTextEdit::NoWrap);

    renderer = new XL::Renderer(rendererOut);
    QFileInfo stylesheet("xl:git.stylesheet");
    QFileInfo syntax("xl:xl.syntax");
    QString sspath(stylesheet.absoluteFilePath());
    QString sypath(syntax.absoluteFilePath());
    renderer->SelectStyleSheet(+sspath, +sypath);
}


XLSourceEdit::~XLSourceEdit()
// ----------------------------------------------------------------------------
//   Destroy source editor
// ----------------------------------------------------------------------------
{
    delete highlighter;
    delete renderer;
}


void XLSourceEdit::setPlainTextKeepCursor(const QString &txt)
// ----------------------------------------------------------------------------
//   Update the text, try not to change the scrollbars and cursor positions
// ----------------------------------------------------------------------------
{
    QScrollBar * hsb = horizontalScrollBar();
    QScrollBar * vsb = verticalScrollBar();
    int x = hsb->value();
    int y = vsb->value();
    int pos = textCursor().position();

    setPlainText(txt);

    hsb->setValue(x);
    vsb->setValue(y);
    QTextCursor cursor(document());
    int last = document()->characterCount();
    if (pos < last)
    {
        cursor.setPosition(pos);
        setTextCursor(cursor);
    }
    update();
}


void XLSourceEdit::highlightNames(int index, std::set<text> &set)
// ----------------------------------------------------------------------------
//   Define the list of XL names for syntax highlighting
// ----------------------------------------------------------------------------
{
    highlighter->highlightNames(index, set);
}


void XLSourceEdit::setSelectedRanges(const XL::stream_ranges &ranges)
// ----------------------------------------------------------------------------
//   Tell the highlighter the ranges of text that correspond to selected items
// ----------------------------------------------------------------------------
{
    highlighter->setSelectedRanges(ranges);
}


void XLSourceEdit::render(XL::Tree_p prog, std::set<XL::Tree_p> *selected)
// ----------------------------------------------------------------------------
//   Show prog source
// ----------------------------------------------------------------------------
{
    if (!prog)
        return clear();

    text txt = "";
    rendererOut.str(txt);

    // Tell renderer how to highlight selected items
    std::set<XL::Tree_p >::iterator i;
    renderer->highlights.clear();
    if (selected)
        for (i = selected->begin(); i != selected->end(); i++)
            renderer->highlights[*i] = "selected";
    renderer->RenderFile(prog);

    XL::stream_ranges sel;
    if (renderer->highlighted.count("selected"))
        sel = renderer->highlighted["selected"];
    setSelectedRanges(sel);

    txt = rendererOut.str();
    setPlainTextKeepCursor(+txt);
}


bool XLSourceEdit::event(QEvent *e)
// ----------------------------------------------------------------------------
//   Process events sent to the text edit
// ----------------------------------------------------------------------------
{
    // Special handling of Tab/Shit+Tab keys has to be done here, because when
    // keyPressEvent is called it is too late (Qt already has taken special
    // actions such as changing focus)
    if (e->type() == QEvent::KeyPress)
    {
        bool done = false;
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Qt::Key_Tab)
        {
            if (ke->modifiers() & Qt::ShiftModifier)
                done = handleShiftTabKey(ke);
            else
                done = handleTabKey(ke);
        }
        else if (ke->key() == Qt::Key_Backtab)
        {
            done = handleShiftTabKey(ke);
        }
        else if (ke->key() == Qt::Key_F1)
        {
            done = handleF1Key(ke);
        }
        if (done)
            return done;
    }
    return QTextEdit::event(e);
}


bool XLSourceEdit::handleTabKey(QKeyEvent *)
// ----------------------------------------------------------------------------
//   Indent everything in the selection by 4 spaces
// ----------------------------------------------------------------------------
{
    QTextCursor cur = textCursor();
    int a = cur.anchor();
    int p = cur.position();
    int newA = a + 4;

    cur.beginEditBlock();

    // Save a new anchor at the beginning of the line of the selected text
    cur.setPosition(a);
    cur.movePosition(QTextCursor::StartOfBlock,QTextCursor::MoveAnchor);
    a = cur.position();

    // Set a new selection with the new beginning
    cur.setPosition(a);
    cur.setPosition(p, QTextCursor::KeepAnchor);

    // Get the selected text and split into lines, insert spaces
    QString str = cur.selection().toPlainText();
    QStringList list = str.split("\n");
    int last = list.count();
    if (last > 1 && cur.atBlockStart())
        --last;
    for (int i = 0; i < last; i++)
    {
        list[i].insert(0, "    ");
        p += 4;
    }

    // Put the text with indents back
    str = list.join("\n");
    cur.removeSelectedText();
    cur.insertText(str);

    // Reselect the text for more indents
    cur.setPosition(newA);
    cur.setPosition(p, QTextCursor::KeepAnchor);

    setTextCursor(cur);

    cur.endEditBlock();

    return true;
}


bool XLSourceEdit::handleShiftTabKey(QKeyEvent *)
// ----------------------------------------------------------------------------
//   Unindent by 4 spaces
// ----------------------------------------------------------------------------
{
    QTextCursor cur = textCursor();
    int a = cur.anchor();
    int p = cur.position();
    int newA = a - 4;

    cur.beginEditBlock();

    // Save a new anchor at the beginning of the line of the selected text
    cur.setPosition(a);
    cur.movePosition(QTextCursor::StartOfBlock,QTextCursor::MoveAnchor);
    a = cur.position();

    // Set a new selection with the new beginning
    cur.setPosition(a);
    cur.setPosition(p, QTextCursor::KeepAnchor);

    // Get the selected text and split into lines, insert spaces
    QString str = cur.selection().toPlainText();
    QStringList list = str.split("\n");
    int last = list.count();
    if (last > 1 && cur.atBlockStart())
        --last;

    // Check if we can actually unindent
    bool ok = true;
    for (int i = 0; ok && i < last; i++)
        if (list[i].left(4) != "    ")
            ok = false;

    if (ok)
    {
        for (int i = 0; i < last; i++)
        {
            list[i] = list[i].mid(4);
            p -= 4;
        }

        // Put the text with indents back
        str = list.join("\n");
        cur.removeSelectedText();
        cur.insertText(str);

        // Reselect the text for more indents
        cur.setPosition(newA);
        cur.setPosition(p, QTextCursor::KeepAnchor);

        setTextCursor(cur);
    }

    cur.endEditBlock();

    return ok;
}


bool XLSourceEdit::handleF1Key(QKeyEvent *)
// ----------------------------------------------------------------------------
//   Open help about the word under the cursor
// ----------------------------------------------------------------------------
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    QString keyword = tc.selectedText();
    if (!keyword.isEmpty())
        Assistant::instance()->showKeywordHelp(keyword);
    return true;
}


void XLSourceEdit::keyPressEvent(QKeyEvent *e)
// ----------------------------------------------------------------------------
//   Editor receives key press
// ----------------------------------------------------------------------------
{
    if (e->key() == Qt::Key_Escape)
        return;
    if (highlighter->hasSelectedObjects())
        highlighter->clearSelectedRanges();
    QTextEdit::keyPressEvent(e);
}

}
