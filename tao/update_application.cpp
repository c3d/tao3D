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


UpdateApplication::UpdateApplication()
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

    // Create a message box to display download progress
    dialog = new QProgressDialog();
}


void UpdateApplication::check(bool msg)
{    
    // Define url to check for update
    QUrl url("http://localhost/update.ini");

    IFTRACE(update)
            debug() << "Check for update from " <<  url.toString().toStdString() << std::endl;

    // Set complete filename
    QString filename = QDir::temp().absolutePath() + "/update.ini";
    if(QFile::exists(filename))
        QFile::remove(filename);

    // Create file
    file = new QFile(filename);
    info = QFileInfo((*file));

    // Check if we can write file
    if(!file->open(QIODevice::WriteOnly))
    {
        delete file;
        file = NULL;
        return;
    }

    // Sent request
    QNetworkRequest request(url);
    reply = manager.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(processCheckForUpdate()));
}


void UpdateApplication::update()
// ----------------------------------------------------------------------------
//    Prepare to launch update
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Prepare to update from: " << url.toStdString() << std::endl;

    QFileInfo urlInfo(url);

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
            QString fileName = urlInfo.fileName();
            QString completeFileName = folder + "/" + fileName;
            if(QFile::exists(completeFileName))
            {
                ret = QMessageBox::question(NULL, tr("File existing"),
                                            tr("There already exists a file called %1 in "
                                               "the specified directory. Overwrite it?").arg(fileName),
                                            QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
                if(ret == QMessageBox::No)
                    return;

                QFile::remove(completeFileName);
            }

            // Create file
            file = new QFile(completeFileName);
            info = QFileInfo((*file));

            // Check if we can write file
            if(!file->open(QIODevice::WriteOnly))
            {
                QMessageBox::information(NULL, "Update failed",
                                         tr("Unable to save the file %1: %2.")
                                         .arg(completeFileName).arg(file->errorString()));
                delete file;
                file = NULL;
                return;
            }

            downloadRequestAborted = false;

            // Start update
            start();
        }
    }
}


void UpdateApplication::start()
// ----------------------------------------------------------------------------
//    Start update
// ----------------------------------------------------------------------------
{
    // Start timer
    downloadTime.start();

    IFTRACE(update)
            debug() << "Start update from " << url.toStdString() << std::endl;

    // Sent request
    QUrl tmp(url);
    QNetworkRequest request(tmp);
    reply = manager.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(dialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

    // Show progress dialog
    dialog->exec();
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

    // Read update.ini file to get version and path of latest update
    QString inipath = QDir::temp().absolutePath() + "/update.ini";
    QSettings settings(inipath, QSettings::IniFormat);
    version = settings.value("version", "").toDouble();
    settings.beginGroup(system + " - " + edition);
    url = settings.value("url", "").toString();
    settings.endGroup();

    // Delete reply and remove file
    disconnect(reply, SIGNAL(finished()), this, SLOT(processCheckForUpdate()));
    reply->deleteLater();
    reply = NULL;
    file->remove();
    delete file;
    file = NULL;

    // Get current version
    QString ver = GITREV;
    QRegExp rxp("([^-]*)");
    rxp.indexIn(ver);
    double current = rxp.cap(1).toDouble();

    IFTRACE(update)
            debug() << "Remote version is " << version << std::endl;

    // Update if current version is older than the remote one
    bool upToDate = (current >= version);
    if(!upToDate)
    {
        update();
    }
    else
    {
        QString title = tr("No update available");
        QString msg = tr("Tao Presentations %1 is up-to-date.").arg(edition);
        QMessageBox::information(NULL, title, msg);
    }
}


void UpdateApplication::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
// ----------------------------------------------------------------------------
//    Update progress bar
// ----------------------------------------------------------------------------
{
    if(downloadRequestAborted)
        return;

    // Calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
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
            debug() << "Download aborted" << std::endl;

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
            debug() << "Write file to " << info.path().toStdString() << std::endl;

    // Write file from reply
    if(file)
        file->write(reply->readAll());
}


void UpdateApplication::downloadFinished()
// ----------------------------------------------------------------------------
//    Finish download
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Download finished" << std::endl;

    // Hide progress dialog
    dialog->hide();

    // Case of download aborted
    if(downloadRequestAborted)
    {
        if(file)
        {
            file->close();
            file->remove();
            delete file;
            file = NULL;
        }
        reply->deleteLater();
        return;
    }

    if(reply->error())
    {
        //Download failed
        QMessageBox::information(NULL, "Download failed", tr("Failed: %1").arg(reply->errorString()));
    }
    else
    {
        IFTRACE(update)
                debug() << "Download successfull" << std::endl;

        // Write file
        downloadReadyRead();

        QString msg = tr("Download of %1 successfull (saved to %2)\n").arg(info.completeBaseName()).arg(info.path());
        QMessageBox::information(NULL, tr("Download successfull"), msg);
    }

    // Close I/O
    file->flush();
    file->close();
    reply->deleteLater();
    reply = NULL;
    delete file;
    file = NULL;
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
