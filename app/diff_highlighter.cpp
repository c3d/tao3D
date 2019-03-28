// *****************************************************************************
// diff_highlighter.cpp                                            Tao3D project
// *****************************************************************************
//
// File description:
//
//    DiffHighlighter implementation
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

#include "diff_highlighter.h"
#include "tao_utf8.h"
#include <iostream>

TAO_BEGIN

DiffHighlighter::DiffHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
// ----------------------------------------------------------------------------
//    Define the patterns and formats for the "diff" syntax
// ----------------------------------------------------------------------------
{
    HighlightingRule rule;
    QColor color, bgcolor;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^diff.*|^index.*|^---.*|^\\+\\+\\+.*|^Identical");
    color = QColor(Qt::black);
    rule.format.setFontWeight(QFont::Bold);
    rule.format.setForeground(color);
    bgcolor = QColor("#E6E6E6");
    rule.format.setBackground(bgcolor);
    highlightingRules << rule;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^\\+.*");
    color = QColor(Qt::darkGreen);
    rule.format.setForeground(color);
    bgcolor.setHsv(color.hue(), color.saturation() * 0.1, 255);
    rule.format.setBackground(bgcolor);
    highlightingRules << rule;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^-.*");
    color = QColor(Qt::darkRed);
    rule.format.setForeground(color);
    bgcolor.setHsv(color.hue(), color.saturation() * 0.1, 255);
    rule.format.setBackground(bgcolor);
    highlightingRules << rule;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^@@.*");
    color = QColor(Qt::darkBlue);
    rule.format.setForeground(color);
    bgcolor.setHsv(color.hue(), color.saturation() * 0.1, 255);
    rule.format.setBackground(bgcolor);
    highlightingRules << rule;
}


void DiffHighlighter::highlightBlock(const QString &txt)
// ----------------------------------------------------------------------------
//    Parse a block of text and apply formatting
// ----------------------------------------------------------------------------
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        bool matched;
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(txt);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(txt, index + length);
            matched = true;
        }
        if (matched)
        {
            DiffBlockData *data = new DiffBlockData(rule.format);
            setCurrentBlockUserData(data);
            break;
        }
    }
}

TAO_END
