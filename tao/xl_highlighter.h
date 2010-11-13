#ifndef XL_HIGHLIGHTER_H
#define XL_HIGHLIGHTER_H
// ****************************************************************************
//  xl_highlighter.h                                               Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

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

    void   setXLNames(const QStringList &words);

protected:
    void   highlightBlock(const QString &txt);

private:
    struct TextCharFormat : public QTextCharFormat
    {
        TextCharFormat() { setFontFamily("unifont"); setFontPointSize(16); }
    };

    struct HighlightingRule
    {
        QRegExp         pattern;
        TextCharFormat  format;
    };

    bool   applyRule(const HighlightingRule &rule, const QString &txt);

    QVector<HighlightingRule>   highlightingRules;
    HighlightingRule            nameRule;

    QRegExp                     commentStartExpression;
    QRegExp                     commentEndExpression;

    TextCharFormat              floatFormat;
    TextCharFormat              quotationFormat;
    TextCharFormat              singleLineCommentFormat;
    TextCharFormat              multiLineCommentFormat;
};

}

#endif // XL_HIGHLIGHTER_H
