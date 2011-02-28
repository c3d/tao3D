// ****************************************************************************
//  dir.cpp                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of the Dir class.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "dir.h"
#include "tao_utf8.h"
#include <iostream>

TAO_BEGIN

QFileInfoList Dir::entryInfoGlobList(QString pattern)
// ----------------------------------------------------------------------------
//   Enumerate files that match pattern relative to the current directory
// ----------------------------------------------------------------------------
{
    QFileInfoList result;
    if (pattern.startsWith("/"))
    {
        while (pattern.startsWith("/"))
            pattern.remove(0, 1);
        result << Dir("/").entryInfoGlobList(pattern);
    }
    else
    {
        int slash = pattern.indexOf("/");
        if (slash != -1)
        {
            QString top = pattern.left(slash);
            QString rem = pattern.mid(slash + 1);
            while (rem.startsWith("/"))
                rem.remove(1, 1);
            QFileInfoList elements = entryInfoList(QStringList() << top);
            foreach (QFileInfo elem, elements)
            {
                if (elem.isDir())
                {
                    Dir dir(elem.absoluteFilePath());
                    result << dir.entryInfoGlobList(rem);
                }
            }
        }
        else
        {
            result << entryInfoList(QStringList()<<pattern, QDir::Files);
        }
    }
    return result;
}

TAO_END
