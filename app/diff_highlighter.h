#ifndef DIFF_HIGHLIGHTER_H
#define DIFF_HIGHLIGHTER_H
// *****************************************************************************
// diff_highlighter.h                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//    A syntax highlighter for the diff syntax.
//    Used to colorize the diff output in DiffDialog.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include <QSyntaxHighlighter>
#include <QTextBlockUserData>
#include <QString>

namespace Tao {

class DiffHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    DiffHighlighter(QTextDocument *parent);

protected:
    void   highlightBlock(const QString &txt);

private:
    struct HighlightingRule
    {
        QRegExp         pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
};


struct DiffBlockData : public QTextBlockUserData
// ----------------------------------------------------------------------------
//   Information attached to some text blocks during highlighting
// ----------------------------------------------------------------------------
{
    DiffBlockData(const QTextCharFormat &format): format(format) {}

    QTextCharFormat     format;
};

}

#endif // DIFF_HIGHLIGHTER_H
