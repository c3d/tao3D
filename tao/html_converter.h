#ifndef HTML_CONVERTER_H
#define HTML_CONVERTER_H
// ****************************************************************************
//  html_converter.h						   Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao_tree.h"
#include "layout.h"
#include <QString>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>

TAO_BEGIN

struct HTMLConverter
// ----------------------------------------------------------------------------
//   Convert from HTML (or QT document) format to XL code
// ----------------------------------------------------------------------------
{
public:
    HTMLConverter(XL::Tree *where, Layout *layout)
        : where(where), layout(layout), indentLevel(0) {}

public:
    XL::Tree   *fromHTML(QString html, QString css="");
    XL::Tree   *docToTree(const QTextDocument &doc);
    text        blockToTree(const QTextBlock &block, scale indentWidth);
    text        fragmentToTree(const QTextFragment &fragment);

public:
    XL::Tree_p          where;
    Layout *            layout;
    QTextBlockFormat    blockFormat;
    QTextCharFormat     charFormat;
    uint                indentLevel;
};

bool modifyBlockFormat(QTextBlockFormat &blockFormat,  Layout * where);
bool modifyCharFormat(QTextCharFormat &format, Layout * where);

TAO_END
#endif // HTML_CONVERTER_H
