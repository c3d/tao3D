#ifndef DIFF_HIGHLIGHTER_H
#define DIFF_HIGHLIGHTER_H
// ****************************************************************************
//  diff_highlighter.h                                             Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
