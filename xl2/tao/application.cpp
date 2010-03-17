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

TAO_BEGIN

void Application::OpenLibrary()
// ----------------------------------------------------------------------------
//    Open the user's default doc library, create it and save pref if needed
// ----------------------------------------------------------------------------
{
    bool ok = false;
    QString path = UserDocumentLibraryPath();
    if (path.isNull())
        path = QInputDialog::getText(NULL, tr("Welcome to Tao!"),
                                     tr("Please choose a folder where your "
                                        "Tao documents will be stored:"),
                                     QLineEdit::Normal,
                                     DefaultDocumentLibraryPath(),
                                     &ok);
    if (path.isNull())
        return;
    docLibrary.curPath = path;
    if (!docLibrary.Open())
    {
        QMessageBox::information(0, tr("Tao Library"),
                                 tr("Failed to open Tao library"));
        return;
    }
    if (ok)
        QSettings().setValue("defaultlibrary", path);
}

void Application::InternalCleanEverythingAsIfTaoWereNeverRun()
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
    QString path = DefaultDocumentLibraryPath();
    ret = QMessageBox::question(0, tr("Tao"),
                                tr("Do you want to delete:\n\n"
                                   "Default document library?") +
                                " (" + path + ")",
                                QMessageBox::Yes    | QMessageBox::No |
                                QMessageBox::YesAll | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel)
        return;
    if (ret == QMessageBox::Yes || ret == QMessageBox::YesAll)
        RecursiveDelete(path);

    // User's document library, if not the default
    path = UserDocumentLibraryPath();
    if (!path.isNull() && path != DefaultDocumentLibraryPath())
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
            RecursiveDelete(path);
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

QString Application::DefaultDocumentLibraryPath()
// ----------------------------------------------------------------------------
//    The path proposed by default (first time run) for the user's doc library
// ----------------------------------------------------------------------------
{
    return QDir::homePath() + tr("/Tao Library.tao");
}

QString Application::UserDocumentLibraryPath()
// ----------------------------------------------------------------------------
//    The path to the current user's document library
// ----------------------------------------------------------------------------
{
    return QSettings().value("defaultlibrary").toString();
}

bool Application::RecursiveDelete(QString path)
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
                err = RecursiveDelete(path);
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
