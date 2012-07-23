// ****************************************************************************
//  update_application.cpp                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Update Tao application.
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
//  (C) 2012 Baptiste Soulisse <baptiste.soulisse@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************
#include "update_application.h"
#include "tree.h"
#include "tao_tree.h"
#include "main.h"
#include "context.h"
#include "version.h"
#include "application.h"
#include "git_backend.h"
#include <QDir>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextStream>
#include <sstream>

namespace Tao {


// ============================================================================
//
//                      Update Application
//
// ============================================================================

UpdateApplication::UpdateApplication() : updating(false)
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
{
#ifdef TAO_EDITION
   // Get current edition
   edition = TAO_EDITION;
#endif

#if defined(Q_OS_MACX)
   system = "MacOSX";
#elif defined(Q_OS_WIN)
   system = "Windows";
#else
   // Check if we are on Debian or Ubuntu distribution to get .deb package
   QString cmd("uname");
   QStringList args;
   args << "-a";
   Process cp(cmd, args);
   text errors, output;
   if(cp.done(&errors, &output))
   {
       // Check os name
       if(output.find("Ubuntu") != output.npos ||
          output.find("Debian") != output.npos)
           system = "Linux Ubuntu";
       else
           system = "Others linux";

       // Check bits number
       if(output.find("x86_64") != output.npos)
           system += " x86_64";
       else
           system += " x86";
   }
#endif

   // Get current version of Tao
   QString ver = GITREV;
   QRegExp rxp("([^-]*)");
   rxp.indexIn(ver);
   version = rxp.cap(1).toDouble();

   // Create network manager
   manager = new QNetworkAccessManager();

   // Create a dialog to display download progress
   dialog = new QProgressDialog();
   dialog->setWindowTitle(tr("New update available"));
   dialog->setFixedSize(250, 100);
}


UpdateApplication::~UpdateApplication()
// ----------------------------------------------------------------------------
//    Destructor
// ----------------------------------------------------------------------------
{
    close();

    // Delete manager
    manager->deleteLater();
    manager = NULL;

    // Delete dialog
    dialog->deleteLater();
    dialog = NULL;

    IFTRACE(update)
            debug() << "Delete update application\n";
}


void UpdateApplication::close()
// ----------------------------------------------------------------------------
//    Close update
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Close update\n";

    fileName = "";

    dialog->close();

    // Disconnect slots
    disconnect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
    disconnect(dialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

    // Delete I/O
    if(file)
    {
        file->close();
        delete file;
        file = NULL;
    }

    // Delete reply
    if(reply)
    {
        reply->abort();
        reply->deleteLater();
        reply = NULL;
    }
}


void UpdateApplication::check(bool msg)
// ----------------------------------------------------------------------------
//    Check for new update
// ----------------------------------------------------------------------------
{    
    if((! updating) && (! edition.isEmpty()))
    {
        useMsg = msg;

        // Define url where check for update
        QUrl url("http://www.taodyne.com/shop/download/update.ini");

        IFTRACE(update)
                debug() << "Check for update from "
                        <<  url.toString().toStdString() << "\n";

        // Set update file name
        QString updateName = QDir::temp().absolutePath() + "/update.ini";
        if(QFile::exists(updateName))
            QFile::remove(updateName);

        // Create file
        file = new QFile(updateName);
        info.setFile(*file);

        // Check if we can write file
        if(!file->open(QIODevice::WriteOnly))
        {
            delete file;
            file = NULL;
            return;
        }

        updating = true;

        // Specify url
        request.setUrl(url);

        // Set a own user-agent as QT default user agent
        // gets rejected (QT known issue).
        QString user("Tao Presentations %1");
        request.setRawHeader("User-Agent", (user.arg(version)).toAscii());

        // Sent request
        reply = manager->get(request);
        connect(reply, SIGNAL(finished()), this,
                SLOT(processCheckForUpdate()), Qt::UniqueConnection);
    }
}


void UpdateApplication::update()
// ----------------------------------------------------------------------------
//    Launch update
// ----------------------------------------------------------------------------
{
    downloadRequestAborted = false;

    // Start timer
    downloadTime.start();

    IFTRACE(update)
            debug() << "Update from "
                    << url.toString().toStdString() << "\n";

    // Create request
    request.setUrl(url);

    // Send request
    reply = manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()),
            Qt::UniqueConnection);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
    connect(dialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
}


void UpdateApplication::readIniFile()
// ----------------------------------------------------------------------------
//    Parse update.ini file
// ----------------------------------------------------------------------------
{
    // Read ini file
    QString iniPath = QDir::temp().absolutePath() + "/update.ini";
    QSettings settings(iniPath, QSettings::IniFormat);

    // Get version, name and path of latest update
    remoteVersion = settings.value("version", "").toDouble();
    settings.beginGroup(system);
    url.setUrl(settings.value(edition, "").toString());
    settings.endGroup();
}


void UpdateApplication::processCheckForUpdate()
// ----------------------------------------------------------------------------
//    Process to check for update
// ----------------------------------------------------------------------------
{
    if(reply->error())
        return;

    IFTRACE(update)
               debug() << "Check for update finished\n";

    // Write file
    downloadReadyRead();

    // Close I/O
    file->flush();
    file->close();

    // Read update.ini file
    readIniFile();

    // Remove file and close reply
    disconnect(reply, SIGNAL(finished()), this,
               SLOT(processCheckForUpdate()));
    file->remove();
    close();

    IFTRACE(update)
            debug() << "Remote version is "
                    << remoteVersion << "\n";

    // Update if current version is older than the remote one
    bool upToDate = (version >= remoteVersion);
    if(!upToDate && !url.isEmpty())
    {
        // Ask for update
        QString title = tr("Tao Presentations %1 available").arg(remoteVersion);
        QString msg = tr("A new version of Tao Presentations is available, would you download it now ?");
        int ret = QMessageBox::question(NULL, title, msg,
                                        QMessageBox::Yes | QMessageBox::No);

        if(ret == QMessageBox::Yes)
        {
            // Start update
            update();
        }
        else
        {
            // Close update
            updating = false;
            close();
        }

    }
    else
    {
        updating = false;
        close();

        if(useMsg)
        {
            QString title = tr("No update available");
            QString msg = tr("Tao Presentations %1 is up-to-date.")
                          .arg(edition);
            QMessageBox::information(NULL, title, msg);
        }
    }
}


void UpdateApplication::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
// ----------------------------------------------------------------------------
//    Update progress bar
// ----------------------------------------------------------------------------
{
    // Assure that downloaded file is
    // big enough to show dialog
    //(avoid it in case of redirection)
    if(bytesTotal > 1000)
    {
        if(downloadRequestAborted)
            return;

        if(! file)
        {
            createFile();
        }
        else
        {
            // Calculate the download speed
            double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
            QString unit;
            if (speed < 1024) {
                unit = tr("bytes/sec");
            } else if (speed < 1024*1024) {
                speed /= 1024;
                unit = tr("kB/s");
            } else {
                speed /= 1024*1024;
                unit = tr("MB/s");
            }

            // Update progress dialog
            QString msg = tr("Downloading %1 : %2 %3");
            dialog->setLabelText(msg.arg(info.completeBaseName())
                                 .arg(speed, 3, 'f', 1).arg(unit));
            dialog->setMaximum(bytesTotal);
            dialog->setValue(bytesReceived);

            // Show progress dialog
            if(dialog->isHidden())
                dialog->show();
        }
    }
}


void UpdateApplication::cancelDownload()
// ----------------------------------------------------------------------------
//    Cancel download
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Abort download\n";

    // Abort download
    downloadRequestAborted = true;
    reply->abort();
}


void UpdateApplication::downloadReadyRead()
// ----------------------------------------------------------------------------
//    Write current download into a file
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Write file to "
                    << info.path().toStdString() << "\n";

    // Write file from reply
    if(file)
        file->write(reply->readAll());
}


void UpdateApplication::downloadFinished()
// ----------------------------------------------------------------------------
//    Finish download
// ----------------------------------------------------------------------------
{
    dialog->hide();

    // Redirection
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (code >= 300 && code < 400) // Redirection
    {
        // Get the redirection url
        url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

        // Because the redirection url can be relative,
        // we have to use the previous one to resolve it
        url = reply->url().resolved(url);

        // Close previous update
        close();

        IFTRACE(update)
                debug() << "Redirected to "
                        << url.toString().toStdString() << "\n";

        // Restart update
        update();

    }
    else
    {
        updating = false;

        // Case of download aborted
        if(downloadRequestAborted)
        {
            if(file)
                file->remove();
            close();
            return;
        }

        // Case of error
        if(reply->error())
        {
            IFTRACE(update)
                    debug() << "Download failed: "
                            << reply->errorString().toStdString() << "\n";


            // Download failed
            QMessageBox::information(NULL, tr("Download failed"),
                                     tr("Download failed: %1").arg(reply->errorString()));
            // Remove file
            if(file)
                file->remove();
        }
        else
        {
            IFTRACE(update)
                    debug() << "Download successfull\n";

            // Write file
            downloadReadyRead();

            QString msg = tr("Download of %1 successfull (saved to %2)\n")
                          .arg(info.completeBaseName()).arg(info.path());
            QMessageBox::information(NULL, tr("Download successfull"), msg);
        }

        // Close I/O
        close();
    }

}


void UpdateApplication::createFile()
// ----------------------------------------------------------------------------
//    Create the update file
// ----------------------------------------------------------------------------
{
    if(! fileName.isEmpty())
        return;

    // Get correct filename
    QFileInfo info(url.toString());
    fileName = (info.fileName().split("?"))[0];

    // Choose folder
    QString folder = QFileDialog::getExistingDirectory(NULL,
                                                       tr("Select destination folder"),
                                                       Application::defaultProjectFolderPath());

    if(! folder.isEmpty())
    {
        // Set complete filename
        QString completeFileName = folder + "/" + fileName;

        if(QFile::exists(completeFileName))
        {
            int ret = QMessageBox::question(NULL, tr("File existing"),
                                            tr("There already exists a file called %1 in "
                                               "the specified directory. Overwrite it?").arg(fileName),
                                            QMessageBox::Yes|QMessageBox::No,
                                            QMessageBox::No);
            if(ret == QMessageBox::No)
            {
                close();
                updating = false;
                downloadRequestAborted = true;
                return;
            }

            QFile::remove(completeFileName);
        }

        IFTRACE(update)
                debug() << "Create file " << completeFileName.toStdString() << std::endl;

        // Set complete filename
        file = new QFile(completeFileName);
        info.setFile(*file);

        if(!file->open(QIODevice::WriteOnly))
        {
            QMessageBox::information(NULL, "Update failed",
                                     tr("Unable to save the file %1: %2.")
                                     .arg(completeFileName)
                                     .arg(file->errorString()));
            downloadRequestAborted = true;
            // Close update
            updating = false;
            close();
        }
    }
    else
    {
        downloadRequestAborted = true;
        // Close update
        updating = false;
        close();
    }
}


std::ostream & UpdateApplication::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Update] ";
    return std::cerr;
}


}
