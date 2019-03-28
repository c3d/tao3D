// *****************************************************************************
// xl_highlighter.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//    XLHighlighter implementation
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

#include "xl_highlighter.h"
#include "tao_utf8.h"
#include <iostream>

TAO_BEGIN

XLHighlighter::XLHighlighter(QTextDocument *parent)
// ----------------------------------------------------------------------------
//    Define the patterns and formats for the XL syntax
// ----------------------------------------------------------------------------
    : QSyntaxHighlighter(parent)
{
    QColor selectedColor("#FFFF30");
    QColor longTextColor("#005500");

    // Build the highlighting rules
    highlightingRules

        // Infix, postfix, prefix and names
        << HighlightingRule(Qt::darkCyan,       "", "")
        << HighlightingRule(Qt::darkYellow,     "", "")
        << HighlightingRule(Qt::darkMagenta,    "", "")
        << HighlightingRule(Qt::darkGray,       "", "")

        // Digits
        << HighlightingRule(Qt::darkBlue,       "\\d*\\.?\\d+")

        // Text
        << HighlightingRule(Qt::darkGreen,      "\"[^\"]*\"")
        << HighlightingRule(Qt::darkGreen,      "'[^']*'")

        // Multiline text
        << HighlightingRule(Qt::darkGreen,      "<<", ">>")

        // Single line comment
        << HighlightingRule(Qt::darkRed,        "//[^\n]*")

        // Multi-line comment
        << HighlightingRule(Qt::darkRed,        "/\\*", "\\*/");

    selectedFormat.setBackground(selectedColor);
}


void XLHighlighter::highlightNames(int index, std::set<text> &names)
// ----------------------------------------------------------------------------
//    Set the words to highlight as XL names or symbols
// ----------------------------------------------------------------------------
{
    HighlightingRule &nameRule = highlightingRules[index];
    if (names.empty())
    {
        nameRule.begin = QRegExp("");
        return;
    }
    QStringList w;
    for (std::set<text>::iterator n = names.begin(); n != names.end(); n++)
        w << QRegExp::escape(+*n);
    QString exp = w.join("|");
    if (index != 1)
        exp = QString("\\b(%1)\\b").arg(exp);
    nameRule.begin = QRegExp(exp);
}


void XLHighlighter::highlightBlock(const QString &txt)
// ----------------------------------------------------------------------------
//    Parse a block of text and apply formatting
// ----------------------------------------------------------------------------
{
    // State is index in highlightingRules
    int currentState = previousBlockState();
    int pos = 0;

    // Check if we have a definition
    int defIndex = txt.indexOf("->");

    // Check which rules apply and apply them
    while (pos >= 0 && pos < txt.length())
    {
        // Check if we are still in a multi-line comment or text
        if (currentState >= 0)
        {
            HighlightingRule &rule = highlightingRules[currentState];
            int endIndex = rule.end.indexIn(txt, pos);
            if (endIndex == -1)
            {
                // Remain in the same block state
                setFormat(pos, txt.length()-pos, rule.format);
                pos = txt.length();
                if (pos <= defIndex)
                    defIndex = -1;
                break;
            }
            else
            {
                // Highlight until end
                endIndex += rule.end.matchedLength();
                setFormat(pos, endIndex-pos, rule.format);
                if (pos <= defIndex && endIndex > defIndex)
                    defIndex = -1;

                // Return to default state at end of match
                currentState = -1;
                pos = endIndex;
            }
        }

        // Find the earliest match amongst possible rules
        int bestIndex = txt.length();
        for (int r = 0; r < highlightingRules.size(); r++)
        {
            HighlightingRule &rule = highlightingRules[r];
            if (!rule.begin.isEmpty())
            {
                int index = rule.begin.indexIn(txt, pos);
                if (index >= 0 && index < bestIndex)
                {
                    currentState = r;
                    bestIndex = index;
                }
            }
        }

        // If nothing else matches, we are done with this line
        if (currentState < 0)
            break;

        // Apply the best rule
        HighlightingRule &rule = highlightingRules[currentState];
        if (rule.end.isEmpty())
        {
            // No end: simply highlight what we match
            int startIndex = rule.begin.indexIn(txt, pos);
            int endIndex = startIndex + rule.begin.matchedLength();
            setFormat(startIndex, endIndex-startIndex, rule.format);
            if (startIndex <= defIndex && endIndex > defIndex)
                defIndex = -1;
            pos = endIndex;
            currentState = -1;
        }
        else
        {
            // There is an end: we deal with it at beginning of loop
            pos = rule.begin.indexIn(txt, pos);
        }
    }

    // Remember current block state for next blocks
    setCurrentBlockState(currentState);

    // Highlight definitions
    if (defIndex >= 0)
    {
        QColor highlight("#F1FFE5");
        int begin = 0, end = defIndex-1;
        while (begin < defIndex && txt[begin] == ' ')
            begin++;
        while (end > begin && txt[end] == ' ')
            end--;
        for (int c = begin; c <= end; c++)
        {
            QTextCharFormat background = format(c);
            background.setBackground(highlight);
            setFormat(c, 1, background);
        }
    }

    // Objects that are selected in the graphical view are shown in a special
    // way in the source code, too
     QTextBlock::iterator it;
     for (it = currentBlock().begin(); !(it.atEnd()); ++it)
     {
         QTextFragment currentFragment = it.fragment();
         if (currentFragment.isValid())
             showSelectionInFragment(currentFragment);
     }
}


static XL::stream_range
intersect(const XL::stream_range a, const XL::stream_range b)
// ----------------------------------------------------------------------------
//    Helper function to intersect two integer ranges
// ----------------------------------------------------------------------------
{
    XL::stream_range r(-1, -1);
    if (b.first >= a.first && b.second <= a.second)
    {
        r = b;
    }
    else if (b.first <= a.first && b.second >= a.first && b.second <= a.second)
    {
        r.first = a.first;
        r.second = b.second;
    }
    else if (b.first >= a.first && b.first <= a.second && b.second >= a.second)
    {
        r.first = b.first;
        r.second = a.second;
    }
    else if (b.first <= a.first && b.second >= a.second)
    {
        r = a;
    }
    return r;
}


bool XLHighlighter::showSelectionInFragment(QTextFragment fragment)
// ----------------------------------------------------------------------------
//    Highlight (part of) fragment containing code related to a selected object
// ----------------------------------------------------------------------------
{
    bool matched = false;
    int spos= fragment.position();
    int epos = spos + fragment.length();
    int blockstart = currentBlock().begin().fragment().position();

    XL::stream_range frag(spos, epos);
    XL::stream_ranges::iterator r;
    for (r = selected.begin(); r != selected.end(); r++)
    {
        XL::stream_range i = intersect((*r), frag);
        if (i.first != -1)
        {
            int hstart = (int)(i.first) - blockstart;
            int hcount = (int)(i.second - i.first);
            setFormat(hstart, hcount, selectedFormat);
            matched = true;
        }
    }
    return matched;
}


void XLHighlighter::setSelectedRanges(const XL::stream_ranges &selected)
// ----------------------------------------------------------------------------
//   Remember the ranges of text that correspond to selected items
// ----------------------------------------------------------------------------
{
    this->selected = selected;
    IFTRACE(srcview)
    {
        std::cerr << "Selected objects (src pos):\n";
        XL::stream_ranges::iterator it;
        for (it = this->selected.begin(); it != this->selected.end(); it++)
            std::cerr << (*it).first << " - " << (*it).second << "\n";
    }
}


void XLHighlighter::clearSelectedRanges()
// ----------------------------------------------------------------------------
//   Clear "selected" ranges when no object has the graphical selection
// ----------------------------------------------------------------------------
{
    this->selected.clear();
    rehighlight();
}

TAO_END
