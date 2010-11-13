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
}


void XLHighlighter::setXLNames(const QStringList &words)
// ----------------------------------------------------------------------------
//    Set the words to highlight as XL names
// ----------------------------------------------------------------------------
{
    QString exp = QString("\\b(%1)\\b").arg(words.join("|"));
    nameRule.pattern = QRegExp(exp);
    nameRule.format.setForeground(QColor("#7F007F"));
}


void XLHighlighter::highlightBlock(const QString &txt)
// ----------------------------------------------------------------------------
//    Parse a block of text and apply formatting
// ----------------------------------------------------------------------------
{
    TextCharFormat fontFormat;
    setFormat(0, txt.length(), fontFormat);

    if (!nameRule.pattern.isEmpty())
        applyRule(nameRule, txt);
    foreach (const HighlightingRule &rule, highlightingRules)
        applyRule(rule, txt);

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
        } else
        {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(txt, startIndex +
                                                    commentLength);
    }
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

TAO_END
