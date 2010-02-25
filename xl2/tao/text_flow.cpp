// ****************************************************************************
//  text_flow.cpp                                                  Tao project
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

#include "tao.h"
#include "text_flow.h"


TextFlow::TextFlow(QTextOption paraOption)
// ----------------------------------------------------------------------------
//  Create an empty text flow.
// ----------------------------------------------------------------------------
{
    paragraphOption = paraOption;
}


TextFlow::TextFlow(QString pieceOfText,
                   QTextCharFormat aFormat, QTextOption paraOption)
// ----------------------------------------------------------------------------
// Create a text flow with the initial value.
// ----------------------------------------------------------------------------
{
    paragraphOption = paraOption;
    addText( pieceOfText, aFormat);
}


int TextFlow::addText(QString pieceOfText, QTextCharFormat aFormat)
// ----------------------------------------------------------------------------
// Add a piece of text and its associated format to the flow.
// ----------------------------------------------------------------------------
{
    QTextLayout::FormatRange range;
    range.start = completeText.length();
    range.length = pieceOfText.length();
    range.format = aFormat;
    listOfFormat.append(range);

    completeText.append(pieceOfText);    
}


TextFlow::~TextFlow()
// ----------------------------------------------------------------------------
//  Destroy the flow.
// ----------------------------------------------------------------------------
{ }


QString TextFlow::getCompleteText()
// ----------------------------------------------------------------------------
//  Return the complete plain text of the flow
// ----------------------------------------------------------------------------
{
    return completeText;
}


QList<QTextLayout::FormatRange> TextFlow::getListOfFormat()
// ----------------------------------------------------------------------------
// Return the list of format associated with the text of this flow.
// ----------------------------------------------------------------------------
{
    return listOfFormat;
}


QTextOption & TextFlow::getParagraphOption()
// ----------------------------------------------------------------------------
// Return a reference onto the text option to allow modification.
// ----------------------------------------------------------------------------
{
    return paragraphOption;
}


