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

struct portability
{
public:
    portability();
    QString toHTML();
    XL::Tree * fromHTML(QString html);

    XL::Tree * docToTree(const QTextDocument &doc );
    XL::Tree * blockToTree(const QTextBlock &block );
    XL::Tree * fragmentToTree(const QTextFragment &fragment );
};

#endif // PORTABILITY_H
