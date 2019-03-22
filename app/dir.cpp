// *****************************************************************************
// dir.cpp                                                         Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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
#if defined(Q_OS_WIN)
    // Handle patterns like "E:/some/path"
    else if (pattern.size() >= 3 &&
             pattern[1] == QChar(':') &&
             pattern[2] == QChar('/'))
    {
        int slash = pattern.indexOf("/");
        QString drive = pattern.left(slash + 1); // E:/
        QString rem = pattern.mid(slash + 1);    // some/path
        result << Dir(drive).entryInfoGlobList(rem);
    }
#endif
    else
    {
        int slash = pattern.indexOf("/");
        if (slash != -1)
        {
            QString top = pattern.left(slash);
            QString rem = pattern.mid(slash + 1);
            while (rem.startsWith("/"))
                rem.remove(0, 1);
            QFileInfoList elements = entryInfoList(QStringList() << top,
                                                   QDir::Dirs | QDir::Hidden);
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
            result << entryInfoList(QStringList() << pattern,
                                    QDir::Files | QDir::Hidden);
        }
    }
    return result;
}

TAO_END
