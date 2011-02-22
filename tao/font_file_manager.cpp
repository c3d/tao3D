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
    IFTRACE(fonts)
        std::cerr << "Searching files for font family '" << +family << "'\n";
    QStringList list = FilesForFontFamily(family);
    if (list.isEmpty())
    {
        IFTRACE(fonts)
            std::cerr << "No font file found!\n";
        errors << QString("No font file found for family '%1'").arg(family);
        return;
    }
    foreach (QString path, list)
    {
        IFTRACE(fonts)
            std::cerr << "  " << +path << "\n";
        if (fontFiles.contains(path))
        {
            IFTRACE(fonts)
                std::cerr << "  Already in list\n";
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
        IFTRACE(fonts)
            std::cerr << "    Font file cannot be loaded";
        return false;
    }
    IFTRACE(fonts)
        std::cerr << "    Font file can be loaded\n";
    QFontDatabase::removeApplicationFont(id);
    return true;
}


void FontFileManager::loadApplicationFonts()
// ----------------------------------------------------------------------------
//    Load application fonts
// ----------------------------------------------------------------------------
{
    QString fontPath = Application::defaultTaoApplicationFolderPath();
    QDir fontDir(fontPath);
    QFileInfoList contents = fontDir.entryInfoList(fontFileFilter);

    fontPath = Application::defaultTaoFontsFolderPath();
    fontDir = QDir(fontPath);
    contents.append(fontDir.entryInfoList(fontFileFilter));

    QTime time;
    int count = 0;
    IFTRACE(fonts)
        time.start();
    foreach (QFileInfo f, contents)
    {
        if (f.isFile())
        {
            QString path = f.absoluteFilePath();
            IFTRACE(fonts)
            {
                std::cerr << "Loading default font file '" << +path << "'...";
                count++;
            }
            int id = QFontDatabase::addApplicationFont(path);
            if (id != -1)
            {
                IFTRACE(fonts)
                    std::cerr << " done (id=" << id << ")\n";
            }
            else
            {
                IFTRACE(fonts)
                    std::cerr << " failed\n";
            }
        }
    }
    IFTRACE(fonts)
    {
        int ms = time.elapsed();
        std::cerr << count << " font files loaded in " << ms << " ms\n";
    }
}


QList<int> FontFileManager::LoadEmbeddedFonts(const QString &docPath)
// ----------------------------------------------------------------------------
//    Load the fonts associated with a document (font embedding)
// ----------------------------------------------------------------------------
{
    QList<int> ids;
    QString fontPath = FontPathFor(docPath);
    QDir fontDir(fontPath);
    QDir appFontDir(Application::defaultTaoFontsFolderPath());
    if (fontDir.canonicalPath() == appFontDir.canonicalPath())
    {
        // Application fonts, already loaded
        return ids;
    }
    QFileInfoList contents = fontDir.entryInfoList(fontFileFilter);
    QTime time;
    int count = 0;
    IFTRACE(fonts)
        time.start();
    foreach (QFileInfo f, contents)
    {
        if (f.isFile())
        {
            QString path = f.absoluteFilePath();
            IFTRACE(fonts)
            {
                std::cerr << "Loading font file '" << +path << "'...";
                count++;
            }
            int id = QFontDatabase::addApplicationFont(path);
            if (id != -1)
            {
                IFTRACE(fonts)
                    std::cerr << " done (id=" << id << ")\n";
                ids.append(id);
            }
            else
            {
                IFTRACE(fonts)
                    std::cerr << " failed\n";
                errors << QString(QObject::tr("Cannot load font file: %1"))
                                 .arg(path);
            }
        }
    }
    IFTRACE(fonts)
    {
        int ms = time.elapsed();
        std::cerr << count << " font files loaded in " << ms << " ms\n";
    }

    return ids;
}

void FontFileManager::UnloadEmbeddedFonts(const QList<int> &ids)
// ----------------------------------------------------------------------------
//    Unload previously loaded fonts
// ----------------------------------------------------------------------------
{
    foreach(int id, ids)
    {
        IFTRACE(fonts)
                std::cerr << "Unloading font id " << id << "\n";
        QFontDatabase::removeApplicationFont(id);
    }
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
