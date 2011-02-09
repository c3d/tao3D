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

TAO_BEGIN

QStringList Dir::entryList(QString pattern)
// ----------------------------------------------------------------------------
//   Enumerate files that match pattern relative to the current directory
// ----------------------------------------------------------------------------
{
    QStringList result;
    if (pattern.startsWith("/"))
    {
        while (pattern.startsWith("/"))
            pattern.remove(0, 1);
        result << Dir("/").entryList(QStringList() << pattern);
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
                if (elem.isDir())
                    result << Dir(elem.absoluteFilePath()).entryList(rem);
        }
        else
        {
            result << QDir::entryList(QStringList() << pattern, QDir::Files);
        }
    }
    return result;
}


QStringList Dir::entryList(const QStringList &nameFilters)
// ----------------------------------------------------------------------------
//   Like QDir::entryList(), but support patterns with subdirectories
// ----------------------------------------------------------------------------
{
    QStringList result;
    foreach (QString pattern, nameFilters)
        result << entryList(pattern);
    return result;
}

TAO_END
