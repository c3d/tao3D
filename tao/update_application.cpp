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
    url = "http://localhost/software/win/";

#ifdef TAO_EDITION
    // Get current edition
    edition = TAO_EDITION;
    url += edition;
#endif

    // Create a message box to display download progress
    dialog = new QProgressDialog();
}


void UpdateApplication::check(bool msg)
{
    update();
}


void UpdateApplication::update()
// ----------------------------------------------------------------------------
//    Prepare to launch update
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Prepare to update" << std::endl;

    version = 1.11;
    QString name = QString("/Tao Presentations %1 %2").arg(edition).arg(version, 0, 'f', 2);
    url += name + ".exe";

    // Ask for update
    QString title = tr("%1 available").arg(name);
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
            QString filename = folder + "/" + name + ".exe";
            if(QFile::exists(filename))
            {
                ret = QMessageBox::question(NULL, tr("Downloader"),
                                            tr("There already exists a file called %1 in "
                                               "the specified directory. Overwrite?").arg(name),
                                            QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
                if(ret == QMessageBox::No)
                    return;

                QFile::remove(filename);
            }

            // Check if we can write file
            file = new QFile(filename);
            info = QFileInfo((*file));

            if(!file->open(QIODevice::WriteOnly))
            {
                QMessageBox::information(NULL, "Downloader",
                                         tr("Unable to save the file %1: %2.")
                                         .arg(name).arg(file->errorString()));
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
            debug() << "Start update" << std::endl;

    QUrl tmp(url);
    QNetworkRequest request(tmp);
    reply = manager.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(dialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

    dialog->exec();
}


void UpdateApplication::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
// ----------------------------------------------------------------------------
//    Update progress bar
// ----------------------------------------------------------------------------
{
    if(downloadRequestAborted)
        return;

    // calculate the download speed
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

    //progressDialog->hide();
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
