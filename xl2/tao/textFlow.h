#ifndef TEXTFLOW_H
#define TEXTFLOW_H
// ****************************************************************************
//  textFlow.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    This is the description of a text with its formatting information
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
//  (C) 2010 Catherine Burvelle <catherine@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

// Include for QString
#include <QString>
// Include for QList
#include <QList>
// Include for QTextCharFormat
#include <QTextCharFormat>
// Include for QTextLayout and QFormatRange
#include <QTextLayout>
// Include for QTextOption
#include <QTextOption>

class textFlow 
//-----------------------------------------------------------------------------
// This is the builder of the text
//-----------------------------------------------------------------------------
{


public:
    textFlow(QTextOption parOption);
    textFlow(QString pieceOfText, QTextCharFormat aFormat, QTextOption parOption);
    ~textFlow();
    int addText(QString pieceOfText, QTextCharFormat aFormat);

    QString getCompleteText();
    QList<QTextLayout::FormatRange> getListOfFormat();
    QTextOption & getParagraphOption();

private :
    QString completeText;
    QList<QTextLayout::FormatRange> listOfFormat;
    QTextOption paragraphOption;
};

#endif // TEXTFLOW_H
