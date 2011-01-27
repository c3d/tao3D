// ****************************************************************************
//  xl_highlighter.h                                               Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "xl_highlighter.h"
#include "tao_utf8.h"
#include <iostream>

TAO_BEGIN

XLHighlighter::XLHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
// ----------------------------------------------------------------------------
//    Define the patterns and formats for the XL syntax
// ----------------------------------------------------------------------------
{
    HighlightingRule rule;

    floatFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("\\d*\\.?\\d+");
    rule.format = floatFormat;
    highlightingRules << rule;

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules << rule;

    multiLineCommentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

    selectedFormat.setBackground(QColor("#D0D0D0"));
    nameRule.format.setForeground(QColor("#7F007F"));
}


void XLHighlighter::setXLNames(const QStringList &words)
// ----------------------------------------------------------------------------
//    Set the words to highlight as XL names
// ----------------------------------------------------------------------------
{
    if (words.isEmpty())
    {
        nameRule.pattern = QRegExp("");
        return;
    }
    QStringList w(words);
    w.replaceInStrings("*", "\\*");
    w.replaceInStrings("+", "\\+");
    QString exp = QString("\\b(%1)\\b").arg(words.join("|"));
    nameRule.pattern = QRegExp(exp);
}


void XLHighlighter::highlightBlock(const QString &txt)
// ----------------------------------------------------------------------------
//    Parse a block of text and apply formatting
// ----------------------------------------------------------------------------
{
    TextCharFormat fontFormat;
    setFormat(0, txt.length(), fontFormat);

    // Deal with single-line patterns

    if (!nameRule.pattern.isEmpty())
        applyRule(nameRule, txt);
    foreach (const HighlightingRule &rule, highlightingRules)
        applyRule(rule, txt);

    // Deal with multi-line comments

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(txt);

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression.indexIn(txt, startIndex);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = txt.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(txt, startIndex +
                                                    commentLength);
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


bool XLHighlighter::applyRule(const HighlightingRule &rule, const QString &txt)
// ----------------------------------------------------------------------------
//    Apply one highlighting rule to a block of text
// ----------------------------------------------------------------------------
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
