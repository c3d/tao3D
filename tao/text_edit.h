// ****************************************************************************
//  text_edit.h						   Tao project
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#ifndef PORTABILITY_H
#define PORTABILITY_H

#include "tao_tree.h"
#include "layout.h"
#include <QString>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>

TAO_BEGIN
struct text_portability
{
public:
    text_portability() {}
    XL::Infix * fromHTML(QString html);

    XL::Infix * docToTree(const QTextDocument &doc );
    XL::Infix * blockToTree(const QTextBlock &block, XL::Infix *parent,
                            qreal indentWidth );
    XL::Infix * fragmentToTree(const QTextFragment &fragment, XL::Infix *parent );

};

// Helper function
bool modifyBlockFormat(QTextBlockFormat &blockFormat,  Layout * where);
bool modifyCharFormat(QTextCharFormat &format, Layout * where);
XL::Prefix * color_to_tree(QColor const&color, text name);

TAO_END
#endif // PORTABILITY_H
