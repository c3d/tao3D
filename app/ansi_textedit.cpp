// *****************************************************************************
// ansi_textedit.cpp                                               Tao3D project
// *****************************************************************************
//
// File description:
//
//     A QTextEdit widget that doesn't mind receiving (some) ANSI escape
//     sequences.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "ansi_textedit.h"
#include <QApplication>
#include <iostream>

namespace Tao {

AnsiTextEdit::AnsiTextEdit(QWidget *parent)
    : QTextEdit(parent), mode(M_Normal)
{
}


void AnsiTextEdit::insertAnsiText(const QByteArray &text)
// ----------------------------------------------------------------------------
//    Append text to the widget, interpret some ANSI escape sequences and CRs
// ----------------------------------------------------------------------------
{
#if 1
    str = QString(text);
    str.replace("\x1b[K", "");
    str.replace("\r", "\n");
    flush();
#else
    // REVISIT: This smart algorithm does not work well :(
    foreach (char c, QString(text).toStdString())
    {
        if (c == 0x1b)
        {
            mode = M_Escape;
        }
        else
        if (c == '\r')
        {
            flush();
            moveCursor(QTextCursor::StartOfLine);
        }
        else
        if (c == '\n')
        {
            str += c;
            flush();
        }
        else
        if (mode == M_Escape && c == '[')
        {
            mode = M_Escape_LBracket;
        }
        else
        if (mode == M_Escape_LBracket && c == 'K')
        {
            flush();
            killEndOfLine();
            mode = M_Normal;
        }
        else
            str += c;
    }
    flush();
#endif
}


void AnsiTextEdit::flush()
// ----------------------------------------------------------------------------
//    Insert current text buffer at current cursor position and clear it
// ----------------------------------------------------------------------------
{
    if (str.isEmpty())
        return;
    insertPlainText(str);
    str.clear();
    moveCursor(QTextCursor::End);
    update();
    QApplication::processEvents();
}


void AnsiTextEdit::killEndOfLine()
// ----------------------------------------------------------------------------
//    Delete all characters from cursor position to end of line
// ----------------------------------------------------------------------------
{
    moveCursor(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    textCursor().removeSelectedText();
    update();
    QApplication::processEvents();
}

}
