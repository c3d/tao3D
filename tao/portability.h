// ****************************************************************************
//  portability.h						   Tao project
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
#ifndef PORTABILITY_H
#define PORTABILITY_H

#include "tao_tree.h"
#include <QString>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>

TAO_BEGIN
struct portability
{
public:
    portability(): head(NULL), tail(NULL){}
    QString toHTML();
    XL::Infix * fromHTML(QString html);

    XL::Infix * docToTree(const QTextDocument &doc );
    XL::Infix * blockToTree(const QTextBlock &block, XL::Infix *parent );
    XL::Infix * fragmentToTree(const QTextFragment &fragment, XL::Infix *parent );

    XL::Infix * getHead() { return head;}
    XL::Infix * getTail() { return tail;}

protected:
    XL::Infix_p head;
    XL::Infix_p tail;

};


TAO_END
#endif // PORTABILITY_H
