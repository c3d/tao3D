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
#include <iostream>

#ifdef CONFIG_MINGW
#  include <windows.h>
#endif

TAO_BEGIN


FontFileManager::FontFileManager()
// ----------------------------------------------------------------------------
//   Initialize the FontFileManager
// ----------------------------------------------------------------------------
{
#ifdef CONFIG_MINGW
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
            char *p = strstr(valueName, " (TrueType)");
            if (p)
                *p = '\0';
            QString path = QString("%1\\Fonts\\%2").arg(sysroot).arg(value);
            fontFaceToFile[valueName] = path;
        }
    }

    RegCloseKey(key);
#endif
}


void FontFileManager::AddFontFile(const QFont &font)
// ----------------------------------------------------------------------------
//   Declare that a font is used by the current document
// ----------------------------------------------------------------------------
{
    QString path = FontToFile(font);
    if (fontFiles.contains(path))
        return;
    if (path.isEmpty())
    {
        QString family = font.family();
        errors << QString("Font file not found for family: '%1'").arg(family);
        return;
    }
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


#ifdef CONFIG_MINGW

QString FontFileManager::FontToFile(const QFont &font)
// ----------------------------------------------------------------------------
//   Find the font file that defines the given font (Windows version)
// ----------------------------------------------------------------------------
{
    QString ret;
    Qt::HANDLE f = font.handle();
    if (!f)
        return ret;

    HFONT g;
    HDC dc;
    char fface[LF_FACESIZE];
    memset(fface, 0, LF_FACESIZE);
    dc = GetDC(NULL);
    g = (HFONT)SelectObject(dc, f);
    GetTextFaceA(dc, sizeof(fface), fface);
    const char notFoundFont[] = "MS Shell Dlg";
    if (!strncmp(fface, notFoundFont, sizeof(notFoundFont) - 1))
        return ret;

    if (fontFaceToFile.contains(fface))
        ret = fontFaceToFile[fface];

    return ret;
}

#elif defined(Q_WS_MAC)

// MacOS implementation of FontToFile is in font_file_manager.mm

#else

QString FontFileManager::FontToFile(const QFont &font)
// ----------------------------------------------------------------------------
//   Dummy implementation, pending Windows and Linux-specific versions
// ----------------------------------------------------------------------------
{
    return QString();
}

#endif

TAO_END
