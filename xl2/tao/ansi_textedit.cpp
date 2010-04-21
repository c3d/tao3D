// ****************************************************************************
//  ansi_textedit.h                                                Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::End);
    setTextCursor(c);
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
