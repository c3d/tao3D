#ifndef FONT_FILE_MANAGER_H
#define FONT_FILE_MANAGER_H
// ****************************************************************************
//  font_file_manager.h                                            Tao project
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"

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
    QList<int>         LoadEmbeddedFonts(const QString &docPath);

    static void        loadApplicationFonts();
    static void        UnloadEmbeddedFonts(const QList<int> &ids);
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
