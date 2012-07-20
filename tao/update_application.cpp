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
           system = "Debian";
       else
           system = "Others";

       // Check bits number
       if(output.find("x86_64") != output.npos)
           system += " 64";
       else
           system += " 32";
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
            debug() << "Delete update application" << std::endl;
}


void UpdateApplication::close()
// ----------------------------------------------------------------------------
//    Close update
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Close update" << std::endl;

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
    if(! updating)
    {
        // Define url to check for update
        QUrl url("http://localhost/update.ini");

        IFTRACE(update)
                debug() << "Check for update from " <<  url.toString().toStdString()
                        << std::endl;

        // Set complete filename
        QString filename = QDir::temp().absolutePath() + "/update.ini";
        if(QFile::exists(filename))
            QFile::remove(filename);

        // Create file
        file = new QFile(filename);
        info.setFile(*file);

        // Check if we can write file
        if(!file->open(QIODevice::WriteOnly))
        {
            delete file;
            file = NULL;
            return;
        }

        updating = true;

        // Sent request
        QNetworkRequest request(url);
        reply = manager->get(request);
        connect(reply, SIGNAL(finished()), this,
                SLOT(processCheckForUpdate()), Qt::UniqueConnection);
    }
}


void UpdateApplication::start()
// ----------------------------------------------------------------------------
//    Prepare to launch update
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Prepare to update from: " << url.toStdString()
                    << std::endl;

    QFileInfo urlInfo(url);

    // If a filename is set in the ini file, use it
    // Otherwise use filename of the url
    if(fileName.isEmpty())
        fileName = urlInfo.fileName();

    // Ask for update
    QString title = tr("%1 available").arg(urlInfo.completeBaseName());
    QString msg = tr("A new version of Tao Presentations is available, would you download it now ?");
    int ret = QMessageBox::question(NULL, title, msg,
                                    QMessageBox::Yes | QMessageBox::No);

    if(ret == QMessageBox::Yes)
    {
        // Choose folder
        QString folder = QFileDialog::getExistingDirectory(NULL,
                             tr("Select destination folder"),
                             Application::defaultProjectFolderPath());

        // Verify if folder exists
        if(! folder.isEmpty())
        {
            // Set complete filename
            QString completeFileName = folder + "/" + fileName;
            if(QFile::exists(completeFileName))
            {
                ret = QMessageBox::question(NULL, tr("File existing"),
                                            tr("There already exists a file called %1 in "
                                               "the specified directory. Overwrite it?").arg(fileName),
                                            QMessageBox::Yes|QMessageBox::No,
                                            QMessageBox::No);
                if(ret == QMessageBox::No)
                {
                    close();
                    updating = false;
                    return;
                }

                QFile::remove(completeFileName);
            }

            // Create file
            file = new QFile(completeFileName);
            info.setFile(*file);

            // Check if we can write file
            if(!file->open(QIODevice::WriteOnly))
            {
                QMessageBox::information(NULL, "Update failed",
                                         tr("Unable to save the file %1: %2.")
                                         .arg(completeFileName)
                                         .arg(file->errorString()));
                close();
                updating = false;
                return;

            }

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
        // Close update
        updating = false;
        close();
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
            debug() << "Update from " << url.toStdString()
                    << " to " << file->fileName().toStdString()
                    << std::endl;

    // Create request
    QUrl tmp(url);
    QNetworkRequest request(tmp);

    // Set a own user-agent as QT default user agent
    // gets rejected (QT known issue).
    QString user("Tao Presentations %1");
    request.setRawHeader("User-Agent", (user.arg(version)).toAscii());

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
    settings.beginGroup(system + " - " + edition);
    fileName = settings.value("filename", "").toString();
    url      = settings.value("url", "").toString();
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
               debug() << "Check for update finished" << std::endl;

    // Write file
    downloadReadyRead();

    // Close I/O
    file->flush();
    file->close();

    // Read update.ini file
    readIniFile();

    // Remove file and delete reply
    disconnect(reply, SIGNAL(finished()), this,
               SLOT(processCheckForUpdate()));
    file->remove();
    close();

    IFTRACE(update)
            debug() << "Remote version is " << remoteVersion << std::endl;

    // Update if current version is older than the remote one
    bool upToDate = (version >= remoteVersion);
    if(!upToDate)
    {
        // Start update
        start();
    }
    else
    {
        updating = false;

        QString title = tr("No update available");
        QString msg = tr("Tao Presentations %1 is up-to-date.")
                      .arg(edition);
        QMessageBox::information(NULL, title, msg);
    }
}


void UpdateApplication::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
// ----------------------------------------------------------------------------
//    Update progress bar
// ----------------------------------------------------------------------------
{
    // Show progress dialog
    if(dialog->isHidden())
        dialog->show();

    if(downloadRequestAborted)
        return;

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
}


void UpdateApplication::cancelDownload()
// ----------------------------------------------------------------------------
//    Cancel download
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Abort download" << std::endl;

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
            debug() << "Write file to " << info.path().toStdString()
                    << std::endl;

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
    updating = false;

    // Case of download aborted
    if(downloadRequestAborted)
    {
        file->remove();
        close();
        return;
    }

    if(reply->error())
    {
        IFTRACE(update)
                debug() << "Download failed: " << reply->errorString().toStdString()
                        << std::endl;

        // Download failed
        QMessageBox::information(NULL, tr("Download failed"),
                                 tr("Download failed: %1").arg(reply->errorString()));
        // Remove file
        file->remove();
    }
    else
    {
        IFTRACE(update)
                debug() << "Download successfull" << std::endl;

        // Write file
        downloadReadyRead();

        QString msg = tr("Download of %1 successfull (saved to %2)\n")
                      .arg(info.completeBaseName()).arg(info.path());
        QMessageBox::information(NULL, tr("Download successfull"), msg);
    }

    // Close I/O
    close();
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
