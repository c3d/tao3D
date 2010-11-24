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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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

public:
    static XL::Name_p xl_nil;

};


TAO_END
#endif // PORTABILITY_H
