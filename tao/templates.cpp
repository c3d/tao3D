// ****************************************************************************
//  templates.cpp                                                  Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "templates.h"
#include "tao_utf8.h"
#include "process.h"

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
    bool ok = recursiveCopy(dir, dst);
    if (!ok)
        return false;
    // Remove auxiliary files
    dst.remove("template.ini");
    if (thumbFile != "")
        dst.remove(thumbFile);
    return true;
}


std::ostream& Template::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Template] ";
    return std::cerr;
}


bool Template::recursiveCopy(const QDir &src, QDir &dst)
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
    args << "-c" << "cp -R \"" + srcPath + "\"/* " + dstPath;

#elif defined (Q_OS_WIN)

    QString cmd("xcopy");
    QStringList args;
    args << "/E" << "/B" << "/Q" << "/Y" << srcPath << dstPath;

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
    if (name.contains(keyword))
        return true;
    if (description.contains(keyword))
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
