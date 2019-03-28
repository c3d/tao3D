#ifndef HTML_CONVERTER_H
#define HTML_CONVERTER_H
// *****************************************************************************
// html_converter.h                                                Tao3D project
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
