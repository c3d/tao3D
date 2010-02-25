#ifndef TEXTFLOW_H
#define TEXTFLOW_H
// ****************************************************************************
//  text_flow.h                                                     Tao project
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
//  (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QString>
#include <QList>
#include <QTextCharFormat>
#include <QTextLayout>
#include <QTextOption>
#include "tree.h"

TAO_BEGIN

struct TextFlow : XL::Info
//-----------------------------------------------------------------------------
// This is the builder of the text
//-----------------------------------------------------------------------------
{
    TextFlow(QTextOption parOption);
    TextFlow(QString pieceOfText, QTextCharFormat format, QTextOption options);
    ~TextFlow();
    int addText(QString pieceOfText, QTextCharFormat aFormat);

    QString getCompleteText();
    QList<QTextLayout::FormatRange> getListOfFormat();
    QTextOption & getParagraphOption();

private :
    QString completeText;
    QList<QTextLayout::FormatRange> listOfFormat;
    QTextOption paragraphOption;
};


struct LayoutInfo : XL::Info, QTextLayout
// ----------------------------------------------------------------------------
//   Record the current state of the layout for a given flow
// ----------------------------------------------------------------------------
{
    LayoutInfo(): XL::Info(), QTextLayout() {}
};

TAO_END

#endif // TEXTFLOW_H
