// ****************************************************************************
//  portability.cpp						   Tao project
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
#include "portability.h"
#include <QTextBlock>
#include <iostream>
#include "tao_tree.h"
#include "tao_utf8.h"
#include "options.h"

TAO_BEGIN


XL::Infix * portability::fromHTML(QString html)
// ----------------------------------------------------------------------------
//   Translate an HTML formated text into XL::Tree
// ----------------------------------------------------------------------------
{
    QTextDocument doc;
    doc.setHtml(html);
    return docToTree(doc);
}

XL::Infix* portability::docToTree(const QTextDocument &doc)
// ----------------------------------------------------------------------------
//   Translate a QTextDocument into XL::Tree
// ----------------------------------------------------------------------------
{
    XL::Infix *t = NULL;
    IFTRACE(clipboard)
            std::cerr << "-> portability::docToTree\n";

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
            std::cerr << "<- portability::docToTree\n";
    head = first->right->AsInfix();
    tail = t;
    return first->right->AsInfix();
}


XL::Infix * portability::blockToTree(const QTextBlock &block, XL::Infix *parent)
// ----------------------------------------------------------------------------
//   Translate a QTextBlock into XL::Tree
// ----------------------------------------------------------------------------
{
    IFTRACE(clipboard)
            std::cerr << "-> portability::blockToTree\n";
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
            std::cerr << "<- portability::blockToTree\n";

    return toReturn;
}


XL::Infix * portability::fragmentToTree(const QTextFragment &fragment, XL::Infix *parent)
// ----------------------------------------------------------------------------
//   Translate a QTextFragment into XL::Tree
// ----------------------------------------------------------------------------
{
    IFTRACE(clipboard)
            std::cerr << "-> portability::fragmentToTree\n";

    QTextCharFormat charFormat = fragment.charFormat();
    XL::Prefix * customWeight = NULL;
    XL::Prefix * customStretch = NULL;
    //////////////////////////
    // Text font
    //////////////////////////
    // color
    QColor txtColor = charFormat.foreground().color().toRgb();

    XL::Real * red = new XL::Real(txtColor.redF());
    XL::Real * green = new XL::Real(txtColor.greenF());
    XL::Real * blue = new XL::Real(txtColor.blueF());
    XL::Real * alpha = new XL::Real(txtColor.alphaF());
    XL::Name * n = new XL::Name("color");

    XL::Infix * comma = new XL::Infix(",", red, green);
    comma = new XL::Infix(",", comma, blue);
    comma = new XL::Infix(",", comma, alpha);
    XL::Prefix *textColor = new XL::Prefix(n, comma);

    // Font
    XL::Name *style = NULL;
    QFont::Style st = charFormat.font().style();
    switch (st) {
        case QFont::StyleNormal  : style = new XL::Name("roman")  ; break;
        case QFont::StyleItalic  : style = new XL::Name("italic") ; break;
        case QFont::StyleOblique : style = new XL::Name("oblique"); break;
    }

    XL::Name *caps = NULL;
    QFont::Capitalization capital = charFormat.fontCapitalization();
    switch (capital) {
        case QFont::MixedCase    : caps = new XL::Name("mixed_case") ; break;
        case QFont::AllUppercase : caps = new XL::Name("uppercase")  ; break;
        case QFont::AllLowercase : caps = new XL::Name("lowercase")  ; break;
        case QFont::SmallCaps    : caps = new XL::Name("small_caps") ; break;
        case QFont::Capitalize   : caps = new XL::Name("capitalized"); break;
    }
    XL::Infix *lf = new XL::Infix(",", caps, style);

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
        lf = new XL::Infix(",", lf, weight);

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
        lf = new XL::Infix(",", lf, stretch);

    if (charFormat.fontUnderline())
    {
        lf = new XL::Infix(",", lf, new XL::Name("underline"));
    }
    if (charFormat.fontOverline())
    {
        lf = new XL::Infix(",", lf, new XL::Name("overline"));
    }
    if (charFormat.fontStrikeOut())
    {
        lf = new XL::Infix(",", lf, new XL::Name("strike_out"));
    }
    if (charFormat.fontKerning())
    {
        lf = new XL::Infix(",", lf, new XL::Name("kerning"));
    }

    XL::Integer * size = new XL::Integer(charFormat.font().pointSize());
    lf = new XL::Infix(",", lf, size);

    XL::Text * fontName = new XL::Text(charFormat.fontFamily().toUtf8().constData(),"\"", "\"");
    lf = new XL::Infix(",", lf, fontName);

    n = new XL::Name("font");
    XL::Prefix *font = new XL::Prefix(n, lf);

    // The text
    XL::Prefix * txt = new XL::Prefix(new XL::Name("text"),
                                      new XL::Text(fragment.text().toUtf8().constData(),"<<", ">>"));
    //////////////////////////
    // Building the resulting tree
    //////////////////////////

    XL::Infix *toReturn = new XL::Infix("\n", txt, XL::xl_nil);
    lf = new XL::Infix("\n", textColor, toReturn);
    if (customStretch)
        lf = new XL::Infix("\n", customStretch, lf);
    if (customWeight)
        lf = new XL::Infix("\n", customWeight, lf);
    lf = new XL::Infix("\n", font, lf);

    // hang this tree to the parent one
    parent->right = lf;

    IFTRACE(clipboard)
            std::cerr << "<- portability::fragmentToTree\n";

    return toReturn;
}

TAO_END
