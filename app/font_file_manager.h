#ifndef FONT_FILE_MANAGER_H
#define FONT_FILE_MANAGER_H
// *****************************************************************************
// font_file_manager.h                                             Tao3D project
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

#include "tao.h"

#include <QDir>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QMap>

TAO_BEGIN

struct FontFileManager
// ----------------------------------------------------------------------------
//   Manage fonts (import/export font files)
// ----------------------------------------------------------------------------
{
    FontFileManager();
    ~FontFileManager() {};

    void               AddFontFiles(const QFont &font);
    QList<int>         LoadDocFonts(const QString &docPath);
    QList<int>         LoadFonts(const QDir &dir);

    static void        loadApplicationFonts();
    static void        UnloadFonts(const QList<int> &ids);
    static QString     FontPathFor(const QString &docPath);

    QStringList        fontFiles;
    QStringList        errors;

protected:

    QStringList        FilesForFontFamily(const QString &family);
    bool               IsLoadable(QString fileName);

#ifdef CONFIG_MINGW

private:
    QMap<QString, QString> fontFaceToFile;

#endif

private:
    static QStringList fontFileFilter;
};

TAO_END

#endif // FONT_FILE_MANAGER_H
