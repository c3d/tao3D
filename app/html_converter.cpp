// *****************************************************************************
// html_converter.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
//
//     Convert from HTML or other formats using a QTextDocument
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "html_converter.h"
#include <QTextList>
#include <iostream>
#include "runtime.h"
#include "tao_tree.h"
#include "tao_utf8.h"
#include "options.h"

TAO_BEGIN

// Helper functions
static text color(text name, QColor const&color);
static text indent(text src);
static inline text fontFlag(bool f) { return f ? "" : "no_"; }


XL::Tree *HTMLConverter::fromHTML(QString html, QString css)
// ----------------------------------------------------------------------------
//   Translate an HTML formated text into XL::Tree
// ----------------------------------------------------------------------------
{
    QTextDocument doc;
    doc.setDefaultFont(layout->font);
    if (css != "")
        doc.setDefaultStyleSheet(css);
    doc.setHtml(html);
    return docToTree(doc);
}


XL::Tree *HTMLConverter::docToTree(const QTextDocument &doc)
// ----------------------------------------------------------------------------
//   Translate a QTextDocument into XL::Tree
// ----------------------------------------------------------------------------
{
    // The first node will be hanged under the right leg of this dummy infix.
    text source = "";

    // Only record changes compared to the initial block format
    QTextBlock firstBlock = doc.firstBlock();
    scale indentWidth = doc.indentWidth();
    blockFormat = firstBlock.blockFormat();
    charFormat = firstBlock.charFormat();

    for (QTextBlock block = firstBlock; block.isValid(); block = block.next())
        source += blockToTree(block, indentWidth);

    IFTRACE(html)
        std::cerr << "HTML SOURCE:\n" << source << "\n-----\n";
    Tree *result = XL::xl_parse_text(source);
    if (where)
    {
        result->SetPosition(where->Position());
        result->SetSymbols(where->Symbols());
    }
    return result;
}


text HTMLConverter::blockToTree(const QTextBlock &block, scale indentWidth)
// ----------------------------------------------------------------------------
//   Translate a QTextBlock into XL::Tree
// ----------------------------------------------------------------------------
// The build tree is hang under the parent's right leg
// The returned value is the latest created infix with its right leg set to NIL
{
    text result = "";
    text prefix = "";
    uint indentLevel = 0;
    QTextBlockFormat format = block.blockFormat();


    // List management
    if (QTextList * list = block.textList())
    {
        QTextListFormat listFormat = list->format();
        text attribs = "";
        text bullet = "";

        scale size   = charFormat.font().pointSizeF();
        scale posX   = -indentWidth * size * 0.01;
        scale posY   = size * 0.3;
        scale radius = size * 0.15;

        indentLevel = listFormat.indent();
        if (listFormat.foreground().color() != format.foreground().color())
            attribs = color("color", listFormat.foreground().color());

        switch (listFormat.style())
        {
        case QTextListFormat::ListDisc:
            bullet = +(QString("circle %1,%2,%3\n")
                       .arg(posX).arg(posY).arg(radius));
            break;
        case QTextListFormat::ListCircle:
            attribs = "color \"transparent\"\n"
                + color("line_color", listFormat.foreground().color());
            bullet = +(QString("circle %1,%2,%3\n")
                       .arg(posX).arg(posY).arg(radius));
            break;
        case QTextListFormat::ListSquare :
            bullet = +(QString("square %1,%2,%3\n")
                       .arg(posX).arg(posY).arg(radius*2));
            break;
        case QTextListFormat::ListDecimal :    // decimal values
        case QTextListFormat::ListLowerAlpha : // lower case Latin characters
        case QTextListFormat::ListUpperAlpha : // upper case Latin characters
        case QTextListFormat::ListLowerRoman : // lower case roman numerals
        case QTextListFormat::ListUpperRoman : // upper case roman numerals
            bullet = +list->itemText(block);
            bullet = +(QString("translate %1-text_width \"%2\", 0, 0\n"
                               "text \"%2\"\n")
                       .arg(posX).arg(+bullet));
            break;
        default:
            break;
        }

        result += "anchor\n" + indent(attribs + bullet);
    }


    // Margin
    if (blockFormat.leftMargin() != format.leftMargin()   ||
        blockFormat.rightMargin() != format.rightMargin() ||
        this->indentLevel != indentLevel)
    {
        prefix += +(QString("margins %1,%2\n")
                    .arg(format.leftMargin() + indentLevel * indentWidth)
                    .arg(format.rightMargin()));
        this->indentLevel = indentLevel;
    }

    // Paragraph space
    if (blockFormat.topMargin() != format.topMargin() ||
        blockFormat.bottomMargin() != format.bottomMargin())
    {
        prefix += +(QString("paragraph_space %1,%2\n")
                    .arg(format.topMargin() * 0.5)
                    .arg(format.bottomMargin() * 0.5));
    }

    // Text alignment
    Qt::Alignment align = format.alignment();
    Qt::Alignment existingAlign = blockFormat.alignment();
    if (align != existingAlign)
    {
        ulong diffs = align ^ existingAlign;
        ulong hmask = (Qt::AlignLeft    |
                       Qt::AlignRight   |
                       Qt::AlignHCenter |
                       Qt::AlignJustify);
        ulong vmask = (Qt::AlignTop     |
                       Qt::AlignBottom  |
                       Qt::AlignVCenter);

        if (diffs & hmask)
        {
            scale justif = (align & Qt::AlignJustify) ? 1.0 : 0.0;
            scale position =
                (align & Qt::AlignLeft)    ? 0.0 :
                (align & Qt::AlignRight)   ? 1.0 :
                (align & Qt::AlignHCenter) ? 0.5 :
                0.0;
            prefix += +(QString("align %1,%2\n").arg(position).arg(justif));
        }

        if (diffs & vmask)
        {
            scale position =
                (align & Qt::AlignTop)     ? 0.0 :
                (align & Qt::AlignBottom)  ? 1.0 :
                (align & Qt::AlignVCenter) ? 0.5 :
                0.0;
            prefix += +(QString("align_vertically %1\n")).arg(position);
        }
    }


    // Record the current format before iterating on the block fragments
    blockFormat = format;

    // Build the source for the fragments
    text source = "";
    for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it)
    {
        const QTextFragment fragment = it.fragment();
        source += fragmentToTree(fragment);
    }

    // Emit the final text for the bullets
    if (prefix != "")
        prefix += "line_break\n";
    return prefix + "paragraph\n" + indent(result + source);
}


text HTMLConverter::fragmentToTree(const QTextFragment &fragment)
// ----------------------------------------------------------------------------
//   Translate a QTextFragment into XL::Tree
// ----------------------------------------------------------------------------
{
    if (!fragment.isValid())
        return "";

    QTextCharFormat format = fragment.charFormat();
    text result = "";

    // Color
    QColor textColor = format.foreground().color();
    if (textColor != charFormat.foreground().color())
        result += color("color", textColor.toRgb());

    // Font
    QFont font = format.font();
    QFont existingFont = charFormat.font();
    if (font != existingFont)
    {
        text fontSpec = "";

        // Family change
        QString fontFamily = font.family();
        if (fontFamily != existingFont.family())
            fontSpec += "\"" + +fontFamily + "\",";

        // Size change
        scale fontSize = font.pointSizeF();
        if (fontSize != existingFont.pointSizeF())
            fontSpec += +(QString("%1,").arg(fontSize));

        // Style change
        QFont::Style fontStyle = font.style();
        if (fontStyle != existingFont.style())
        {
            switch(fontStyle)
            {
            case QFont::StyleNormal:  fontSpec += "roman,"; break;
            case QFont::StyleItalic:  fontSpec += "italic,"; break;
            case QFont::StyleOblique: fontSpec += "oblique,"; break;
            }
        }

        // Weight change
        int fontWeight = font.weight();
        if (fontWeight != existingFont.weight())
        {
            switch(fontWeight)
            {
            case QFont::Light    : fontSpec += "light,"   ; break;
            case QFont::Normal   : fontSpec += "regular," ; break;
            case QFont::DemiBold : fontSpec += "demibold,"; break;
            case QFont::Bold     : fontSpec += "bold,"    ; break;
            case QFont::Black    : fontSpec += "black,"   ; break;
            default              : fontSpec += +(QString("weight %1,")
                                                 .arg(fontWeight));
            }
        }

        // Capitalization change
        if (font.capitalization() != existingFont.capitalization())
        {
            switch(font.capitalization())
            {
            case QFont::MixedCase    : fontSpec += "mixed_case,"; break;
            case QFont::AllUppercase : fontSpec += "uppercase,"  ; break;
            case QFont::AllLowercase : fontSpec += "lowercase,"  ; break;
            case QFont::SmallCaps    : fontSpec += "small_caps," ; break;
            case QFont::Capitalize   : fontSpec += "capitalized,"; break;
            }
        }

        // Stretch change
        int fontStretch = font.stretch();
        if (fontStretch != existingFont.stretch())
        {
            switch(fontStretch)
            {
            case QFont::UltraCondensed : fontSpec += "ultra_condensed,"; break;
            case QFont::ExtraCondensed : fontSpec += "extra_condensed,"; break;
            case QFont::Condensed      : fontSpec += "condensed,"      ; break;
            case QFont::SemiCondensed  : fontSpec += "semi_condensed," ; break;
            case QFont::Unstretched    : fontSpec += "unstretched,"    ; break;
            case QFont::SemiExpanded   : fontSpec += "semi_expanded,"  ; break;
            case QFont::Expanded       : fontSpec += "expanded,"       ; break;
            case QFont::ExtraExpanded  : fontSpec += "extra_expanded," ; break;
            case QFont::UltraExpanded  : fontSpec += "ultra_expanded," ; break;
            default                    : fontSpec += +(QString("stretch %1,")
                                                       .arg(fontStretch));
            }
        }

        // Font flags
        if (font.underline() != existingFont.underline())
            fontSpec += fontFlag(font.underline()) + "underline,";
        if (font.overline() != existingFont.overline())
            fontSpec += fontFlag(font.overline()) + "overline,";
        if (font.strikeOut() != existingFont.strikeOut())
            fontSpec += fontFlag(font.strikeOut()) + "strike_out,";
        if (font.kerning() != existingFont.kerning())
            fontSpec += fontFlag(font.kerning()) + "kerning,";

        // Check if there was a font change in the end
        if (fontSpec != "")
        {
            // Remove the trailing comma
            fontSpec = fontSpec.substr(0, fontSpec.length()-1);
            result += "font " + fontSpec + "\n";
        }
    }
    charFormat = format;

    // Check if there is an image
    QTextImageFormat image = format.toImageFormat();
    if (image.isValid())
        result += "text_span {"
            "color \"white\"; image \"" + +image.name() + "\" }\n";

    // Add the text
    QString fragText = fragment.text();
    if (fragText != "")
    {
        if (fragText.indexOf("\n") >= 0)
        {
            fragText = fragText.replace(">>", ">> & \">>\" & <<");
            fragText = "text <<" + fragText + ">>\n";
            result += +fragText;
        }
        else
        {
            fragText = fragText.replace("\"", "\"\"");
            fragText = "text \"" + fragText + "\"\n";
            result += +fragText;
        }
    }

    return result;
}


text color(text name, QColor const &color)
// ----------------------------------------------------------------------------
//   Build a tree that represent the color
// ----------------------------------------------------------------------------
{
    QString colorText("%1 %2,%3,%4,%5\n");
    colorText = colorText
        .arg(+name)
        .arg(color.redF())
        .arg(color.greenF())
        .arg(color.blueF())
        .arg(color.alphaF());
    return +colorText;
}


text indent(text source)
// ----------------------------------------------------------------------------
//    Indent the given text with the given indent
// ----------------------------------------------------------------------------
{
    QString str = +(" " + source);
    str = str.replace("\n", "\n ");
    if (str.endsWith("\n "))
        str.truncate(str.length() - 1);
    return +str;
}



// ============================================================================
//
//    Helper functions for external text editing
//
// ============================================================================

bool modifyBlockFormat(QTextBlockFormat &blockFormat,
                       Layout * where)
// ----------------------------------------------------------------------------
//   Modify a blockFormat with the given layout
// ----------------------------------------------------------------------------
{
    bool modified = false;
    if (blockFormat.alignment() !=
        (where->alongX.toQtHAlign() | where->alongY.toQtVAlign()))
    {
        blockFormat.setAlignment(where->alongX.toQtHAlign() |
                                 where->alongY.toQtVAlign());
        modified = true;
    }

    if (blockFormat.topMargin() != where->top)
    {
        blockFormat.setTopMargin(where->top);
        modified = true;
    }

    if ( blockFormat.bottomMargin() != where->bottom)
    {
        blockFormat.setBottomMargin(where->bottom);
        modified = true;
    }

    if ( blockFormat.leftMargin() != where->left)
    {
        blockFormat.setLeftMargin(where->left);
        modified = true;
    }

    if ( blockFormat.rightMargin() != where->right)
    {
        blockFormat.setRightMargin(where->right);
        modified = true;
    }

    return modified;

}


bool modifyCharFormat(QTextCharFormat &format,
                      Layout * where)
// ----------------------------------------------------------------------------
//   Modify a charFormat with the given layout
// ----------------------------------------------------------------------------
{
    bool modified = false;
    if (format.font() != where->font)
    {
        format.setFont(where->font);
        modified = true;
    }

    QColor fill;
    fill.setRgbF(where->fillColor.red,
                 where->fillColor.green,
                 where->fillColor.blue,
                 where->fillColor.alpha);
    if ( format.foreground().color() != fill)
    {
        format.setForeground(fill);//QBrush(fill));
        modified = true;
    }
    return modified;
}

TAO_END
