#ifndef XL_HIGHLIGHTER_H
#define XL_HIGHLIGHTER_H
// *****************************************************************************
// xl_highlighter.h                                                Tao3D project
// *****************************************************************************
//
// File description:
//
//    A syntax highlighter for XL.
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
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "renderer.h"
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>
#include <set>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace Tao {

class XLHighlighter : public QSyntaxHighlighter
// ----------------------------------------------------------------------------
//   XL syntax highlighter
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    XLHighlighter(QTextDocument *parent);

    void   highlightNames(int index, std::set<text> &set);
    void   setSelectedRanges(const XL::stream_ranges &selected);
    bool   hasSelectedObjects()    { return !selected.empty(); }
    void   clearSelectedRanges();

protected:
    void   highlightBlock(const QString &txt);

private:
    struct TextCharFormat : public QTextCharFormat
    {
        TextCharFormat()
        {
            QFont font("unifont");
            font.setPixelSize(16);
            setFont(font);
        }
    };

    struct HighlightingRule
    {
        HighlightingRule(): begin(), end(), format() {}
        HighlightingRule(const HighlightingRule &o)
            : begin(o.begin), end(o.end), format(o.format) {}
        HighlightingRule(QBrush brush, QString begin, QString end="")
            : begin(begin), end(end), format()
        {
            format.setForeground(brush);
        }

        QRegExp         begin;
        QRegExp         end;
        TextCharFormat  format;
    };

    bool   showSelectionInFragment(QTextFragment fragment);


    // Highlighting rules, the last one is for the names
    QVector<HighlightingRule>   highlightingRules;
    TextCharFormat              selectedFormat;
    XL::stream_ranges           selected;
};

}

#endif // XL_HIGHLIGHTER_H
