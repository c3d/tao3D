#ifndef ANSI_TEXTEDIT_H
#define ANSI_TEXTEDIT_H
// *****************************************************************************
// ansi_textedit.h                                                 Tao3D project
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
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include <QTextEdit>
#include <QByteArray>

namespace Tao {

class AnsiTextEdit : public QTextEdit
// ----------------------------------------------------------------------------
//   QTextEdit with ANSI escape support
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    AnsiTextEdit(QWidget *parent = 0);

public slots:
    virtual void insertAnsiText(const QByteArray &text);

private:
    void flush();
    void killEndOfLine();

private:
    enum Mode
    {
        M_Normal,          // Currently reading 'normal' characters
        M_Escape,          // Just read an escape character (0x1B)
        M_Escape_LBracket, // Read a left bracket after an escape
    };

private:
    Mode    mode;
    QString esc;
    QString str;
};

}

#endif // ANSI_TEXTEDIT_H
