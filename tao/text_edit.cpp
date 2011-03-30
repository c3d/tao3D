// ****************************************************************************
//  text_edit.cpp						   Tao project
// ****************************************************************************
//
//   File Description:
//
//     The portability between tao text environment and QTextDocument.
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include "text_edit.h"
#include <QTextBlock>
#include <iostream>
#include "tao_tree.h"
#include "tao_utf8.h"
#include "options.h"

TAO_BEGIN

Tree_p list2tree(std::vector<Tree*> v, text infix);

XL::Infix * text_portability::fromHTML(QString html)
// ----------------------------------------------------------------------------
//   Translate an HTML formated text into XL::Tree
// ----------------------------------------------------------------------------
{
    QTextDocument doc;
    doc.setHtml(html);
    return docToTree(doc);
}

XL::Infix* text_portability::docToTree(const QTextDocument &doc)
// ----------------------------------------------------------------------------
//   Translate a QTextDocument into XL::Tree
// ----------------------------------------------------------------------------
{
    XL::Infix *t = NULL;
    IFTRACE(clipboard)
            std::cerr << "-> text_portability::docToTree\n";

    XL::Infix *first = new XL::Infix("\n", XL::xl_nil, XL::xl_nil);

    for ( QTextBlock block = doc.firstBlock();
         block.isValid();
         block = block.next())
    {
        if (!t)
        {
            t = blockToTree( block, first );
        }
        else
        {
            // Insert a paragraph_break between two blocks
            t->right = new XL::Infix("\n", new XL::Name("paragraph_break"), XL::xl_nil);
            t = blockToTree( block, t->right->AsInfix() );
        }
        for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it)
        {
            const QTextFragment fragment = it.fragment();
            t = fragmentToTree(fragment, t);
        }
    }
    IFTRACE(clipboard)
            std::cerr << "<- text_portability::docToTree\n";
    head = first->right->AsInfix();
    tail = t;
    return first->right->AsInfix();
}


XL::Infix * text_portability::blockToTree(const QTextBlock &block, XL::Infix *parent)
// ----------------------------------------------------------------------------
//   Translate a QTextBlock into XL::Tree
// ----------------------------------------------------------------------------
{
    IFTRACE(clipboard)
            std::cerr << "-> text_portability::blockToTree\n";
    QTextBlockFormat blockFormat = block.blockFormat();
    //////////////////////////
    // Margin
    //////////////////////////
    // margins l, r ==> PREFIX("margins") - INFIX(REAL(l),",",REAL(r))
    XL::Real * l = new XL::Real(blockFormat.leftMargin() +
                                blockFormat.textIndent());
    XL::Real * r = new XL::Real(blockFormat.rightMargin());
    XL::Infix * comma = new XL::Infix(",", l, r);
    XL::Name * n = new XL::Name("margins");
    XL::Prefix * margin = new XL::Prefix(n, comma);

    // paragraph_space b, a
    XL::Real * b = new XL::Real(blockFormat.topMargin());
    XL::Real * a = new XL::Real(blockFormat.bottomMargin());
    comma = new XL::Infix(",", b, a);
    n = new XL::Name("paragraph_space");
    XL::Prefix * para_space = new XL::Prefix(n, comma);

    //////////////////////////
    // Text alignment
    //////////////////////////
    Qt::Alignment align = blockFormat.alignment();

    // Horizontal justify
    double hJust = 0.0;
    if (align & Qt::AlignJustify)
        hJust = 1.0;
    XL::Real *hj = new XL::Real(hJust);

    // Horizontal alignment
    double hAlign = 0.0;
    if (align & Qt::AlignHCenter)
        hAlign = 0.5;
    else if (align & Qt::AlignRight)
        hAlign = 1.0;
    XL::Real *ha = new XL::Real(hAlign);
    n = new XL::Name("align");
    XL::Prefix *hAlignment = new XL::Prefix(n, new XL::Infix(",", ha, hj));

    // Vertical justify
    // Vertical alignment
    double vAlign = 0.0;
    if (align & Qt::AlignVCenter)
        vAlign = 0.5;
    else if (align & Qt::AlignBottom)
        vAlign = 1.0;
    XL::Real *va = new XL::Real(vAlign);
    n = new XL::Name("vertical_align");
    XL::Prefix *vAlignment = new XL::Prefix(n, va);

    //////////////////////////
    // Building the resulting tree
    //////////////////////////
    XL::Infix * toReturn = new XL::Infix("\n", vAlignment, XL::xl_nil);
    XL::Infix * lf = new XL::Infix("\n", hAlignment, toReturn);
    lf = new XL::Infix("\n", para_space, lf);
    lf = new XL::Infix("\n", margin, lf);

    // hang this tree to the parent one
    parent->right = lf;

    IFTRACE(clipboard)
            std::cerr << "<- text_portability::blockToTree\n";

    return toReturn;
}


XL::Infix * text_portability::fragmentToTree(const QTextFragment &fragment,
                                             XL::Infix *parent)
// ----------------------------------------------------------------------------
//   Translate a QTextFragment into XL::Tree
// ----------------------------------------------------------------------------
{
    IFTRACE(clipboard)
            std::cerr << "-> text_portability::fragmentToTree\n";

    QTextCharFormat charFormat = fragment.charFormat();
    XL::Prefix * customWeight = NULL;
    XL::Prefix * customStretch = NULL;
    //////////////////////////
    // Text font
    //////////////////////////
    // color
    QColor txtColor = charFormat.foreground().color().toRgb();
    std::vector<Tree*> color_list;
    color_list.push_back(new XL::Real(txtColor.redF()));
    color_list.push_back(new XL::Real(txtColor.greenF()));
    color_list.push_back(new XL::Real(txtColor.blueF()));
    color_list.push_back(new XL::Real(txtColor.alphaF()));
    XL::Name * n = new XL::Name("color");

    Tree * comma = list2tree(color_list, ",");
    XL::Prefix *textColor = new XL::Prefix(n, comma);

    // Font
    std::vector<Tree*> font_list;
    XL::Name *style = NULL;
    QFont::Style st = charFormat.font().style();
    switch (st) {
        case QFont::StyleNormal  : style = new XL::Name("roman")  ; break;
        case QFont::StyleItalic  : style = new XL::Name("italic") ; break;
        case QFont::StyleOblique : style = new XL::Name("oblique"); break;
    }
    font_list.push_back(style);
    XL::Name *caps = NULL;
    QFont::Capitalization capital = charFormat.fontCapitalization();
    switch (capital) {
        case QFont::MixedCase    : caps = new XL::Name("mixed_case") ; break;
        case QFont::AllUppercase : caps = new XL::Name("uppercase")  ; break;
        case QFont::AllLowercase : caps = new XL::Name("lowercase")  ; break;
        case QFont::SmallCaps    : caps = new XL::Name("small_caps") ; break;
        case QFont::Capitalize   : caps = new XL::Name("capitalized"); break;
    }
    font_list.push_back(caps);

    XL::Name *weight = NULL;
    /*Font::Weight*/
    int w = charFormat.fontWeight();
    switch (w) {
        case QFont::Light    : weight = new XL::Name("light")   ; break;
        case QFont::Normal   : weight = new XL::Name("regular") ; break;
        case QFont::DemiBold : weight = new XL::Name("demibold"); break;
        case QFont::Bold     : weight = new XL::Name("bold")    ; break;
        case QFont::Black    : weight = new XL::Name("black")   ; break;
        default :
                customWeight = new XL::Prefix(new XL::Name("weight"),
                                              new XL::Integer(w));
                break;
    }
    if (weight)
        font_list.push_back(weight);


    XL::Name *stretch = NULL;
    /*QFont::Stretch*/
    int str = charFormat.font().stretch();
    switch (str) {
        case QFont::UltraCondensed :
            stretch = new XL::Name("ultra_condensed"); break;
        case QFont::ExtraCondensed :
            stretch = new XL::Name("extra_condensed"); break;
        case QFont::Condensed :
            stretch = new XL::Name("condensed"      ); break;
        case QFont::SemiCondensed :
            stretch = new XL::Name("semi_condensed" ); break;
        case QFont::Unstretched :
            stretch = new XL::Name("unstretched"    ); break;
        case QFont::SemiExpanded :
            stretch = new XL::Name("semi_expanded"  ); break;
        case QFont::Expanded :
            stretch = new XL::Name("expanded"       ); break;
        case QFont::ExtraExpanded :
            stretch = new XL::Name("extra_expanded" ); break;
        case QFont::UltraExpanded :
            stretch = new XL::Name("ultra_expanded" ); break;
    default :
            customStretch = new XL::Prefix(new XL::Name("stretch"),
                                           new XL::Integer(str));
            break;
    }
    if (stretch)
        font_list.push_back(stretch);

    if (charFormat.fontUnderline())
    {
        font_list.push_back(new XL::Name("underline"));
    }
    if (charFormat.fontOverline())
    {
        font_list.push_back(new XL::Name("overline"));
    }
    if (charFormat.fontStrikeOut())
    {
        font_list.push_back(new XL::Name("strike_out"));
    }
    if (charFormat.fontKerning())
    {
        font_list.push_back(new XL::Name("kerning"));
    }

    font_list.push_back(new XL::Integer(charFormat.font().pointSize()));

    font_list.push_back(new XL::Text(charFormat.fontFamily().toUtf8().constData(),
                                     "\"", "\""));
    comma = list2tree(font_list, ",");

    n = new XL::Name("font");
    XL::Prefix *font = new XL::Prefix(n, comma);

    // The text
    XL::Prefix * txt = new XL::Prefix(new XL::Name("text"),
                                      new XL::Text(fragment.text().toUtf8().constData(),
                                                   "<<", ">>"));
    //////////////////////////
    // Building the resulting tree
    //////////////////////////

    XL::Infix *toReturn = new XL::Infix("\n", txt, XL::xl_nil);
    XL::Infix *lf = new XL::Infix("\n", textColor, toReturn);
    if (customStretch)
        lf = new XL::Infix("\n", customStretch, lf);
    if (customWeight)
        lf = new XL::Infix("\n", customWeight, lf);
    lf = new XL::Infix("\n", font, lf);

    // hang this tree to the parent one
    parent->right = lf;

    IFTRACE(clipboard)
            std::cerr << "<- text_portability::fragmentToTree\n";

    return toReturn;
}


Tree_p list2tree(std::vector<Tree*> v, text infix)
// ----------------------------------------------------------------------------
//   Builds a tree from a list of tree with the given infix.
// ----------------------------------------------------------------------------
{
    if (v.size() <= 0) return XL::xl_nil;
    if (v.size() == 1) return v[0];

    Tree* result = NULL;
    std::vector<Tree*>::reverse_iterator rit;
    // First build the bottom of the tree
    rit = v.rbegin();
    result = *rit;
    rit++;
    // Build the common part
    for (; rit < v.rend(); ++rit)
    {
        result = new XL::Infix(infix, *rit, result);
    }
    return result;
}


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
