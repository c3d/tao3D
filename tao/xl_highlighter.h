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

#include "renderer.h"
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
        QRegExp         pattern;
        TextCharFormat  format;
    };

    bool   applyRule(const HighlightingRule &rule, const QString &txt);
    bool   showSelectionInFragment(QTextFragment fragment);


    QVector<HighlightingRule>   highlightingRules;
    HighlightingRule            nameRule;

    QRegExp                     commentStartExpression;
    QRegExp                     commentEndExpression;

    TextCharFormat              floatFormat;
    TextCharFormat              quotationFormat;
    TextCharFormat              singleLineCommentFormat;
    TextCharFormat              multiLineCommentFormat;
    TextCharFormat              selectedFormat;

    XL::stream_ranges           selected;
};

}

#endif // XL_HIGHLIGHTER_H
