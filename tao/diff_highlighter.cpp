// ****************************************************************************
//  diff_highlighter.h                                             Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    QColor color;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^\\+[^+].*");
    color = QColor(Qt::darkGreen);
    rule.format.setForeground(color);
    color.setAlphaF(0.2);
    rule.format.setBackground(color);
    highlightingRules << rule;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^-[^-].*");
    color = QColor(Qt::darkRed);
    rule.format.setForeground(color);
    color.setAlphaF(0.1);
    rule.format.setBackground(color);
    highlightingRules << rule;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^@@.*");
    color = QColor(Qt::darkBlue);
    rule.format.setForeground(color);
    color.setAlphaF(0.1);
    rule.format.setBackground(color);
    highlightingRules << rule;

    rule = HighlightingRule();
    rule.pattern = QRegExp("^diff.*|^index.*|---.*|\\+\\+\\+.*|^Identical");
    color = QColor(Qt::black);
    rule.format.setFontWeight(QFont::Bold);
    color.setAlphaF(0.1);
    rule.format.setBackground(color);
    highlightingRules << rule;
}


void DiffHighlighter::highlightBlock(const QString &txt)
// ----------------------------------------------------------------------------
//    Parse a block of text and apply formatting
// ----------------------------------------------------------------------------
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(txt);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            DiffBlockData *data = new DiffBlockData(rule.format);
            setCurrentBlockUserData(data);
            index = expression.indexIn(txt, index + length);
        }
    }
}

TAO_END
