// *****************************************************************************
// font_file_manager.cpp                                           Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "font_file_manager.h"
#include "font.h"
#include "options.h"
#include "tao_utf8.h"
#include "application.h"

#include <QFontDatabase>
#include <QFileInfoList>
#include <QDir>
#include <QTime>
#include <iostream>

#ifdef CONFIG_MINGW
#  include <windows.h>
#endif


TAO_BEGIN

// ----------------------------------------------------------------------------
// Supported font file formats
// ----------------------------------------------------------------------------
QStringList FontFileManager::fontFileFilter = QStringList("*.ttf")
                                                       << "*.ttc"
                                                       << "*.otf";

FontFileManager::FontFileManager()
// ----------------------------------------------------------------------------
//   Initialize the FontFileManager
// ----------------------------------------------------------------------------
{
#ifdef CONFIG_MINGW
    // Enumerate installed fonts

    HKEY  key = NULL;
    LONG  res = 0;
    char  valueName[256];
    DWORD valueNameSize;
    char  value[512];
    DWORD valueSize;
    long  index = 0;
    char *sysroot;

    sysroot = getenv("SYSTEMROOT");
    if (!sysroot)
        return;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                      0,
                      KEY_QUERY_VALUE,
                      &key) != ERROR_SUCCESS)
        return;
    for (;;)
    {
        valueNameSize = sizeof(valueName);
        valueSize     = sizeof(value);
        memset(value, 0, valueSize);
        res = RegEnumValueA(key, index++, valueName, &valueNameSize,
                            0,  NULL,
                            (BYTE*)value, &valueSize);
        if (res == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (res == ERROR_SUCCESS)
        {
            QString path = QString("%1\\Fonts\\%2").arg(sysroot).arg(value);
            fontFaceToFile[valueName] = path;
        }
    }

    RegCloseKey(key);
#endif
}


void FontFileManager::AddFontFiles(const QFont &font)
// ----------------------------------------------------------------------------
//   Record that current document uses fonts from the family of 'font'
// ----------------------------------------------------------------------------
{
    QString family = font.family();
    record(fonts, "Searching files for font family %s", family);
    QStringList list = FilesForFontFamily(family);
    if (list.isEmpty())
    {
        record(fonts_warning, "No font file found for %s", family);
        errors << QString("No font file found for family '%1'").arg(family);
        return;
    }
    foreach (QString path, list)
    {
        record(fonts, "Looking in path %s", path);
        if (fontFiles.contains(path))
        {
            record(fonts, "Font path %s is already in the list", path);
            return;
        }
        if (IsLoadable(path))
            fontFiles << path;
        else
            errors << QString("%1: unsupported format").arg(path);
    }
}


bool FontFileManager::IsLoadable(QString fileName)
// ----------------------------------------------------------------------------
//   Test if a font file is loadable by Qt
// ----------------------------------------------------------------------------
{
    int id = QFontDatabase::addApplicationFont(fileName);
    if (id == -1)
    {
        record(fonts_warning, "Font file %s cannot be loaded", fileName);
        return false;
    }
    record(fonts, "Font file %s loaded with ID %d", fileName, id);
    QFontDatabase::removeApplicationFont(id);
    return true;
}


void FontFileManager::loadApplicationFonts()
// ----------------------------------------------------------------------------
//    Load application fonts
// ----------------------------------------------------------------------------
{
    FontFileManager ffm;
    ffm.LoadFonts(QDir(Application::defaultTaoFontsFolderPath()));
}


QList<int> FontFileManager::LoadDocFonts(const QString &docPath)
// ----------------------------------------------------------------------------
//    Load the fonts associated with a document (give path to .ddd file)
// ----------------------------------------------------------------------------
{
    QDir fontDir(FontPathFor(docPath));
    QDir appFontDir(Application::defaultTaoFontsFolderPath());
    if (fontDir == appFontDir)
    {
        // Application fonts, already loaded
        return QList<int>();
    }

    return LoadFonts(fontDir);
}


QList<int> FontFileManager::LoadFonts(const QDir &dir)
// ----------------------------------------------------------------------------
//    Load all fonts in the specified directory
// ----------------------------------------------------------------------------
{
    QList<int> ids;
    if (!dir.exists())
        return ids;
    QFileInfoList contents = dir.entryInfoList(fontFileFilter);
    unsigned count = 0;
    record(fonts, "Looking for fonts in %s", +dir.absolutePath());
    foreach (QFileInfo f, contents)
    {
        if (f.isFile())
        {
            QString path = f.absoluteFilePath();
            record(fonts, "Loading font #%u file %s", ++count, f.fileName());
            int id = QFontDatabase::addApplicationFont(path);
            if (id != -1)
            {
                record(fonts, "Font path %s added, ID %d", path, id);
                ids.append(id);
            }
            else
            {
                record(fonts_warning, "Font path %s could not be added", path);
                errors << QString(QObject::tr("Cannot load font file: %1"))
                                 .arg(f.fileName());
            }
        }
    }
    record(fonts, "Loaded %u fonts", count);
    return ids;
}


void FontFileManager::UnloadFonts(const QList<int> &ids)
// ----------------------------------------------------------------------------
//    Unload previously loaded fonts
// ----------------------------------------------------------------------------
{
    foreach(int id, ids)
    {
        record(fonts, "Unloading font ID %d", id);
        QFontDatabase::removeApplicationFont(id);
    }
    FontParsingAction::exactMatchCache.clear();
}


QString FontFileManager::FontPathFor(const QString &docPath)
// ----------------------------------------------------------------------------
//    Return the directory where to store embedded fonts for a document
// ----------------------------------------------------------------------------
{
    QString projPath, fontPath;
    projPath = QFileInfo(docPath).absolutePath();
    fontPath = QString("%1/fonts").arg(projPath);
    return fontPath;
}


#ifdef CONFIG_MINGW

QStringList FontFileManager::FilesForFontFamily(const QString &family)
// ----------------------------------------------------------------------------
//   Find the font files that define the given font family (Windows version)
// ----------------------------------------------------------------------------
{
    QStringList ret;
    QMap<QString, QString>::const_iterator i;
    for (i =  fontFaceToFile.constBegin(); i != fontFaceToFile.constEnd(); i++)
        if (i.key().startsWith(family))
            ret << i.value();
    return ret;
}

#elif defined(CONFIG_MACOSX)

// MacOS implementation of FilesForFontFamily is in font_file_manager.mm

#else

QStringList FontFileManager::FilesForFontFamily(const QString &family)
// ----------------------------------------------------------------------------
//   Dummy implementation, pending Linux-specific version
// ----------------------------------------------------------------------------
{
    (void)family;
    return QStringList();
}

#endif

TAO_END
