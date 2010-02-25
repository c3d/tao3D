#ifndef TEXT_FLOW_H
#define TEXT_FLOW_H
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

struct TextFlow : XL::Info, QTextLayout
//-----------------------------------------------------------------------------
//  A particular text flow, that we may then render in various frames
//-----------------------------------------------------------------------------
{
    typedef QTextLayout::FormatRange    FormatRange;
    typedef QList<FormatRange>          FormatRanges;

public:
    TextFlow(QTextOption parOption);
    ~TextFlow();

    int                 addText(QString pieceOfText, QTextCharFormat aFormat);

public:
    QString             completeText;
    FormatRanges        formats;
    QTextOption         paragraphOption;
    double              topLineY;
};

TAO_END

#endif // TEXT_FLOW_H
