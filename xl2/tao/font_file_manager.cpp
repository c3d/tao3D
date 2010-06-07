// ****************************************************************************
//  font_file_manager.cpp                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//     Tools to embbed fonts into Tao documents.
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "font_file_manager.h"

#include <QFontDatabase>

TAO_BEGIN

void FontFileManager::AddFontFile(const QFont &font)
// ----------------------------------------------------------------------------
//   Declare that a font is used by the current document
// ----------------------------------------------------------------------------
{
    QString path = FontToFile(font);
    if (!fontFiles.contains(path))
        if (IsLoadable(path))
            fontFiles << path;
        else
            errors << QString("%1: unsupported format").arg(path);
}


bool FontFileManager::IsLoadable(QString fileName)
// ----------------------------------------------------------------------------
//   Test if a font file is loadable by Qt
// ----------------------------------------------------------------------------
{
    int id = QFontDatabase::addApplicationFont(fileName);
    if (id == -1)
        return false;
    QFontDatabase::removeApplicationFont(id);
    return true;
}

TAO_END
