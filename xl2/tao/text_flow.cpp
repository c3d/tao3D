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


TAO_BEGIN

TextFlow::TextFlow(QTextOption paraOption)
// ----------------------------------------------------------------------------
//  Create an empty text flow.
// ----------------------------------------------------------------------------
    : completeText(""),
      formats(),
      paragraphOption(paraOption),
      topLineY(0)
{}


TextFlow::~TextFlow()
// ----------------------------------------------------------------------------
//  Destroy the flow.
// ----------------------------------------------------------------------------
{}


int TextFlow::addText(QString pieceOfText, QTextCharFormat aFormat)
// ----------------------------------------------------------------------------
//   Add a piece of text and its associated format to the flow.
// ----------------------------------------------------------------------------
{
    QTextLayout::FormatRange range;
    range.start = completeText.length();
    range.length = pieceOfText.length();
    range.format = aFormat;
    formats.append(range);

    completeText.append(pieceOfText);

    topLineY = 0;
}


void TextFlow::clear()
// ----------------------------------------------------------------------------
//   Discard the contents of a text flow
// ----------------------------------------------------------------------------
{
    completeText = "";
    formats.clear();
    topLineY = 0;
}

TAO_END
