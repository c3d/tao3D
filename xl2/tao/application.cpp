// ****************************************************************************
//  application.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    The Tao application
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

#include "application.h"
#include "tao.h"
#include <QString>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QDir>
#include <QDebug>

TAO_BEGIN

void Application::openLibrary()
// ----------------------------------------------------------------------------
//    Open the user's doc library, create it if needed
// ----------------------------------------------------------------------------
{
    bool ok = false;
    QString path = userDocumentLibraryPath();
    if (path.isNull())
        path = QInputDialog::getText(NULL, tr("Welcome to Tao!"),
                                     tr("Please choose a folder where your "
                                        "Tao documents will be stored:"),
                                     QLineEdit::Normal,
                                     defaultDocumentLibraryPath(),
                                     &ok);
    else
        if (!QDir(path).isReadable())
            path = QInputDialog::getText(NULL, tr("Tao"),
                                     tr("Your Tao document library cannot be "
                                        "opened.\n"
                                        "Please choose a folder where your "
                                        "Tao documents will be stored:"),
                                     QLineEdit::Normal,
                                     path,
                                     &ok);
    if (path.isNull())
        return;
    docLibrary.curPath = path;
    if (!docLibrary.Open())
    {
        QMessageBox::information(0, tr("Tao Library"),
                                 tr("Failed to open Tao library:\n") +
                                 path);
        return;
    }
    if (ok)
        QSettings().setValue("defaultlibrary", path);
}

void Application::internalCleanEverythingAsIfTaoWereNeverRun()
// ----------------------------------------------------------------------------
//    Clean persistent stuff that previous Tao runs may have created
// ----------------------------------------------------------------------------
{
    int ret;
    ret = QMessageBox::warning(0, tr("Tao"),
                               tr("Cleaning the Tao environment"
                                  "\n\n"
                                  "This command allows you to clean the Tao "
                                  "environment: user preferences, default "
                                  "document library, etc.\n"
                                  "A confirmation will be asked for each "
                                  "item to be deleted. You may also choose to "
                                  "delete all items at once."),
                             QMessageBox::Ok | QMessageBox::Cancel,
                             QMessageBox::Cancel);
    if (ret  != QMessageBox::Ok)
        return;

    // Default document library
    QString path = defaultDocumentLibraryPath();
    ret = QMessageBox::question(0, tr("Tao"),
                                tr("Do you want to delete:\n\n"
                                   "Default document library?") +
                                " (" + path + ")",
                                QMessageBox::Yes    | QMessageBox::No |
                                QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        recursiveDelete(path);

    // User's document library, if not the default
    path = userDocumentLibraryPath();
    if (!path.isNull() && path != defaultDocumentLibraryPath())
    {
        if (ret != QMessageBox::YesAll)
            ret = QMessageBox::question(0, tr("Tao"),
                                        tr("Do you want to delete:\n\n"
                                           "User's document library?") +
                                        " (" + path + ")",
                                        QMessageBox::Yes    | QMessageBox::No |
                                        QMessageBox::YesAll | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return;
        if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
            recursiveDelete(path);
    }

    // User preferences
    if (ret != QMessageBox::YesAll)
        ret = QMessageBox::question(0, tr("Tao"),
                                    tr("Do you want to delete:\n\n"
                                       "Tao user preferences?"),
                                    QMessageBox::Yes    | QMessageBox::No |
                                    QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        QSettings().clear();
}

QString Application::documentsPath()
// ----------------------------------------------------------------------------
//    Try to guess the best documents folder to use by default
// ----------------------------------------------------------------------------
{
#ifdef QT_WS_WIN
    // Looking at the Windows registry
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\CurrentVersion\\Explorer",
                       QSettings::NativeFormat);
    // For Windows Vista/7
    QString path = settings.value("User Shell Folders\\Personal").toString();
    if (!path.isNull())
    {
        // Typically C:\Users\username\Documents
        return path;
    }
    // For Windows XP
    path = settings.value("User Shell Folders\\Personal").toString();
    if (!path.isNull())
    {
        // Typically C:\Documents and Settings\username\My Documents
        return path;
    }
#endif // QT_WS_WIN

    // Trying to ding a home sub-directory ending with "Documents"
    QFileInfoList list = QDir::home().entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs );
    for (int i = 0; i < list.size(); i++)
    {
        QFileInfo info = list[i];
        if (info.fileName().endsWith("documents", Qt::CaseInsensitive))
        {
            return info.canonicalFilePath();
        }
    }
    // Last default would be home itself
    return QDir::homePath();
}

QString Application::defaultDocumentLibraryPath()
// ----------------------------------------------------------------------------
//    The path proposed by default (first time run) for the user's doc library
// ----------------------------------------------------------------------------
{
    return documentsPath() + tr("/Tao Document Library");
}

QString Application::userDocumentLibraryPath()
// ----------------------------------------------------------------------------
//    The path to the current user's document library
// ----------------------------------------------------------------------------
{
    return QSettings().value("defaultlibrary").toString();
}

bool Application::recursiveDelete(QString path)
// ----------------------------------------------------------------------------
//    Delete a directory including all its files and sub-directories
// ----------------------------------------------------------------------------
{
    QDir dir(path);

    bool err = false;
    if (dir.exists())
    {
        QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot |
                                               QDir::Dirs | QDir::Files |
                                               QDir::Hidden);
        for (int i = 0; (i < list.size()) && !err; i++)
        {
            QFileInfo entryInfo = list[i];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
                err = recursiveDelete(path);
            else
                if (!QFile(path).remove())
                    err = true;
        }
        if (!QDir().rmdir(path))
            err = true;
    }
    return err;
}

TAO_END
