// *****************************************************************************
// templates.cpp                                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    Implementation of the Templates class.
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
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "templates.h"
#include "tao_utf8.h"
#include "tao_process.h"
#include "application.h"

#include <QSettings>
#include <QTextStream>

TAO_BEGIN

Template::Template(const QDir &dir)
// ----------------------------------------------------------------------------
//   Open a document template and read its properties
// ----------------------------------------------------------------------------
    : dir(dir), valid(false)
{
    QString path = dir.absolutePath();

    IFTRACE(templates)
        debug() << "Reading template in " << +path << "\n";

    if (!dir.exists())
        return;

    QString inipath = path + "/template.ini";
    QSettings ini(inipath, QSettings::IniFormat);

    // Read template name, default to directory name
    QVariant dirname(QString(dir.dirName()));
    name = ini.value("name", dirname).toString();

    // Read template description, default to directory name
    QVariant noDesc(QObject::tr("No description available."));
    description = ini.value("description", noDesc).toString();

    // Read template type
    QVariant defaultType("theme");
    type = ini.value("type", defaultType).toString();

    // Read thumbnail picture
    thumbFile = ini.value("thumbnail").toString();
    thumbnail = QPixmap(path + "/" + thumbFile);

    // Read name of main document file (.ddd)
    mainFile = ini.value("main_file").toString();
    if (mainFile == "")
    {
        // See if there is a single .ddd file
        QStringList ddd = dir.entryList(QStringList("*.ddd"), QDir::Files);
        if (ddd.size() == 1)
            mainFile = ddd.first();
    }

    // Read translations, if present
    QString lang = TaoApp->lang;
    if (lang != "")
    {
        QSettings ini(inipath, QSettings::IniFormat);
        ini.setIniCodec("UTF-8");
        ini.beginGroup(lang);
        name = ini.value("name", name).toString();
        description = ini.value("description", description).toString();
        ini.endGroup();
    }

    valid = true;

    IFTRACE(templates)
    {
        QString thumb;
        if (!thumbnail.isNull())
            thumb = thumbFile;
        debug() << " name: " << +name << "\n";
        debug() << " thumbnail: " << +thumb << "\n";
        debug() << " main file: " << +mainFile << "\n";
        debug() << "Template is valid\n";
    }
}


bool Template::copyTo(QDir &dst)
// ----------------------------------------------------------------------------
//   Copy the template into dst
// ----------------------------------------------------------------------------
{
    // Delete destination directory. Ignore errors, because file may be
    // currently open (in which case it will be overwritten and reloaded)
    if (dst.exists())
        Application::recursiveDelete(dst.absolutePath());
    if (!recursiveCopy(dir, dst, mainFile))
        return false;
    // Remove auxiliary files
    dst.remove("template.ini");
    if (thumbFile != "")
        dst.remove(thumbFile);
    return true;
}


bool Template::operator<(const Template t) const
// ----------------------------------------------------------------------------
//   Overload the < operator for a template (used to sort templates)
// ----------------------------------------------------------------------------
{
    QString name1(name);
    QString name2(t.name);

    // Strip "(Demo) " or "(Démo) "
    name1.replace(QRegExp("^\\([^)]+\\) "), "");
    name2.replace(QRegExp("^\\([^)]+\\) "), "");

    // Case of blank document
    if(t.mainFile == "blank.ddd")
        return false;
    if (mainFile == "blank.ddd")
        return true;

    return (name1 < name2);
}


std::ostream& Template::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Template] ";
    return std::cerr;
}


#if defined (Q_OS_UNIX)

static QString escapeShellArg(QString &str)
// ----------------------------------------------------------------------------
//   Convert string so that it can be passed unchanged to a shell command
// ----------------------------------------------------------------------------
{
    str.replace("'", "\\'");
    return "'" + str + "'";
}

#endif

bool Template::recursiveCopy(const QDir &src, QDir &dst, QString mainFile)
// ----------------------------------------------------------------------------
//   Recursively copy the contents of src into dst (created if does not exist)
// ----------------------------------------------------------------------------
{
    bool ok = true;
    if (!dst.exists())
    {
        // Copy commands below assume destination exists
        ok = dst.mkpath(dst.absolutePath());
        if (!ok)
            return false;
    }

    QString srcPath = QDir::toNativeSeparators(src.absolutePath());
    QString dstPath = QDir::toNativeSeparators(dst.absolutePath());

#if defined (Q_OS_UNIX)

    QString cmd("/bin/sh");
    QStringList args;
    args << "-c" << "cp -R " + escapeShellArg(srcPath) + "/* "
                             + escapeShellArg(dstPath);

#elif defined (Q_OS_WIN)

    QString cmd("xcopy");
    QStringList args;
    args << "/E" << "/Q" << "/Y" << srcPath << dstPath;

#else
#error "Don't know how to do a recursive copy!"
#endif

    Process cp(cmd, args);
    ok = cp.done();

#if defined (Q_OS_WIN)
    // xcopy preserves the modification time of each file. We don't want
    // that for the main document file, because we want Tao to detect that
    // the file has changed whenever an existing document is overwritten.
    if (mainFile != "")
    {
        QString dstMainFile(dstPath + "\\" + mainFile);
        IFTRACE(templates)
            debug() << "'Touching' " << +dstMainFile << "\n";
        QFile f(dstMainFile);
        f.open(QIODevice::ReadWrite);
        qint64 sz = f.size();
        f.resize(sz+1);
        f.resize(sz);
        f.close();
    }
#else
    Q_UNUSED(mainFile);
#endif

    return ok;
}


QString Template::mainFileFullPath()
// ----------------------------------------------------------------------------
//   Full path to the main .ddd file of the template (if known)
// ----------------------------------------------------------------------------
{
    if (mainFile.isEmpty())
        return "";
    return dir.absoluteFilePath(mainFile);
}


bool Template::contains(const QString &keyword, bool searchSource)
// ----------------------------------------------------------------------------
//   Check if name, description or main file contains keyword
// ----------------------------------------------------------------------------
{
    if (name.contains(keyword, Qt::CaseInsensitive))
        return true;
    if (description.contains(keyword, Qt::CaseInsensitive))
        return true;
    if (searchSource)
    {
        if (source.isNull())
        {
            QFile file(mainFileFullPath());
            if (file.open(QIODevice::ReadOnly))
            {
                QTextStream s(&file);
                source = s.readAll();
            }
        }
        if (source.contains(keyword))
            return true;
    }

    return false;
}


Templates::Templates(const QDir &dir)
// ----------------------------------------------------------------------------
//   Find document templates in a directory
// ----------------------------------------------------------------------------
    : dir(dir)
{
    read(dir);
}


Templates::Templates(const QList<QDir> &dirs)
// ----------------------------------------------------------------------------
//   Find document templates in several directories
// ----------------------------------------------------------------------------
{
    foreach(QDir d, dirs)
        read(d);
}

void Templates::read(const QDir &dir)
// ----------------------------------------------------------------------------
//   Find document templates in a directory
// ----------------------------------------------------------------------------
{
    IFTRACE(templates)
        debug() << "Looking for templates in " << +dir.absolutePath()
                << "\n";

    foreach (QFileInfo f, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QDir d(f.absoluteFilePath());
        Template t(d);
        if (t.isValid())
            append(t);
    }

    // Sort templates by alphabetical order
    qSort(begin(), end());
}


std::ostream& Templates::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Templates] ";
    return std::cerr;
}

TAO_END
