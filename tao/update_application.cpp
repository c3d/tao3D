// ****************************************************************************
//  update_application.cpp                                         Tao project
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
#include "application.h"
#include "tao_utf8.h"
#include <QDir>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextStream>
#include <QDesktopServices>
#include <QMessageBox>
#include <sstream>

namespace Tao {


static void setBoxMinimumWidth(QMessageBox &box, int width)
// ----------------------------------------------------------------------------
//    Set minimum width of a message box
// ----------------------------------------------------------------------------
{
    QSpacerItem* spacer = new QSpacerItem(width, 0, QSizePolicy::Minimum,
                                          QSizePolicy::Expanding);
    QGridLayout* layout = dynamic_cast<QGridLayout*>(box.layout());
    if (!layout)
        return;
    layout->addItem(spacer, layout->rowCount(), 0, 1,
                    layout->columnCount());
}


static QPixmap loadIcon(QString path)
// ----------------------------------------------------------------------------
//    Load image file for use as a message box icon
// ----------------------------------------------------------------------------
{
    return QPixmap(path).scaled(64, 64,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation);
}


// ============================================================================
//
//                      Update Application
//
// ============================================================================

UpdateApplication::UpdateApplication()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
  : state(Idle), file(NULL), progress(NULL),
    dialogTitle(QString(tr("Tao Presentations Update"))),
    downloadIcon(loadIcon(":/images/download.png")),
    checkmarkIcon(loadIcon(":images/checkmark.png")),
    connectionErrorIcon(loadIcon(":/images/not_connected.png")),
    reply(NULL), manager(NULL), code(-1)
{
    // download.png from http://www.iconfinder.com/icondetails/2085/128
    // Author: Alexandre Moore
    // License: LGPL
    //
    // checkmark.png from http://www.iconfinder.com/icondetails/3207/128
    // Author: Everaldo Coelho
    // License: LGPL

    // not_connected.png is a merge from:
    //
    // - Red cross from http://www.iconfinder.com/icondetails/3206/128
    //   Author: Everaldo Coelho
    //   License: LGPL
    // - Earth from http://www.iconfinder.com/icondetails/17829/128
    //   Author: Everaldo Coelho
    //   License: LGPL

   resetRequest();

   progress = new QProgressDialog(TaoApp->windowWidget());
   progress->setFixedSize(500, 100);
   connect(progress, SIGNAL(canceled()),
           this, SLOT(cancel()));
   progress->setWindowTitle(dialogTitle);

   IFTRACE(update)
       debug() << "Current version: edition='" << +edition
               << "' version=" << version
               << " target='" << +target << "'"
               << " User-Agent='" << +userAgent() << "'\n";
}


UpdateApplication::~UpdateApplication()
// ----------------------------------------------------------------------------
//    Destructor
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Destructor\n";
    // FIXME crash on MacOSX
    // delete progress;
}


void UpdateApplication::resetRequest()
// ----------------------------------------------------------------------------
//    Initialize HTTP request
// ----------------------------------------------------------------------------
{
    request = QNetworkRequest();
    request.setRawHeader("User-Agent", userAgent().toAscii());
}


QString UpdateApplication::appName()
// ----------------------------------------------------------------------------
//    Return "Tao Presentations <EditionName>" or "Tao Presentations"
// ----------------------------------------------------------------------------
{
    QString ed;
    if (!edition.isEmpty() && edition != "unified")
        ed = " " + edition;
    return QString("Tao Presentations%1").arg(ed);
}


QString UpdateApplication::remoteVer()
// ----------------------------------------------------------------------------
//    Remote version formatted as a string with 2 decimal digits
// ----------------------------------------------------------------------------
{
    return QString("%1").arg(remoteVersion, 0, 'f', 2);
}


void UpdateApplication::connectSignals(QNetworkReply *reply)
// ----------------------------------------------------------------------------
//    Connect signals of reply object to 'this'
// ----------------------------------------------------------------------------
{
    connect(reply, SIGNAL(metaDataChanged()), this, SLOT(processReply()));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));

}


void UpdateApplication::check(bool show)
// ----------------------------------------------------------------------------
//    Check for update
// ----------------------------------------------------------------------------
{
    // This method starts the "check for update" algorithm which is
    // asynchronous.
    // If 'show' is true, a progress dialog may be shown during the check, and
    // a dialog is shown at the end to tell if update is available or not.
    // If 'show' is false, no interaction with the user occurs
    // until the algorithm detects that an update is available.

    IFTRACE(update)
        debug() << "Checking for update (interactive: " << show << ")\n";

    this->show = show;
    if (show && state != Idle)
    {
        // Already running. Just show the progress dialog.
        progress->show();
        return;
    }

    if (edition.isEmpty())
    {
        showNoUpdateAvailable();
        return;
    }

    if (!manager)
        manager = new QNetworkAccessManager();

    progress->setLabelText(tr("Checking for update"));
    if (show)
        progress->show();


    // The URL where to get update information from. Redirects to a .ini file.
    QUrl url("http://www.taodyne.com/taopresentations/1.0/update");

    IFTRACE(update)
        debug() << "Downloading: '" << +url.toString() << "'\n";

    state = WaitingForUpdate;

    // Create and send HTTP request
    request.setUrl(url);
    reply = manager->get(request);
    connectSignals(reply);
}


QDateTime UpdateApplication::lastChecked()
// ----------------------------------------------------------------------------
//    The date and time when the last check for update returned
// ----------------------------------------------------------------------------
{
    return QSettings().value("LastCheckForUpdate").toDateTime();
}


void UpdateApplication::setLastChecked(QDateTime when)
// ----------------------------------------------------------------------------
//    Record date and time of last successful check for update
// ----------------------------------------------------------------------------
{
    QSettings().setValue("LastCheckForUpdate", QVariant(when));
}


void UpdateApplication::startDownload()
// ----------------------------------------------------------------------------
//    Start downloading the URL of the Tao file
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Downloading: '" << +url.toString() << "'\n";

    Q_ASSERT(!reply);
    Q_ASSERT(!url.isEmpty());

    progress->setLabelText(tr("Downloading %1 %2...").arg(appName())
                                                     .arg(remoteVer()));
    progress->show();

    state = Downloading;
    downloadTime.start();

    request.setUrl(url);
    reply = manager->get(request);
    connect(reply, SIGNAL(metaDataChanged()), this, SLOT(processReply()));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
}


void UpdateApplication::readIniFile()
// ----------------------------------------------------------------------------
//    Get version, name and path of latest update from received .ini file
// ----------------------------------------------------------------------------
{
    Q_ASSERT(file);

    IFTRACE(update)
    {
        file->open(QIODevice::ReadOnly | QIODevice::Text);
        while (file->bytesAvailable())
            debug() << " " << +QString(file->readLine());
        file->close();
    }

    QSettings settings(file->fileName(), QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    remoteVersion = settings.value("version", "-1").toDouble();
    // Read optional, localized description text
    // For instance for french: read description-fr then description
    QString descKey = QString("description-%1").arg(TaoApp->lang);
    description = settings.value(descKey,
                                 settings.value("description")).toString();
    settings.beginGroup(target);
    url.setUrl(settings.value(edition).toString());
    settings.endGroup();

    IFTRACE(update)
    {
        debug() << "Remote version is " << +remoteVersion
                << " download URL: '" << +url.toString() << "'\n";
        if (!description.isEmpty())
            debug() << "Description string for update dialog:\n"
                    << +description << "\n";
    }
}


void UpdateApplication::showNoUpdateAvailable()
// ----------------------------------------------------------------------------
//    Show message box: no update available
// ----------------------------------------------------------------------------
{
    if (!show)
        return;

    QString msg = tr("<h3>%1 is up to date</h3>").arg(appName());
    QString info = tr("<p>There is no new version available for download.<p>");
    QMessageBox box(TaoApp->windowWidget());
    box.setIconPixmap(checkmarkIcon);
    box.setWindowTitle(dialogTitle);
    box.setText(msg);
    box.setInformativeText(info);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}


void UpdateApplication::showDownloadSuccessful()
// ----------------------------------------------------------------------------
//    Show message box: download success
// ----------------------------------------------------------------------------
{
    QString msg = tr("<h3>Download succesful</h3>");
    QString info = tr("<p>%1 %2 was downloaded successfully.</p>"
                      "<p>To complete the upgrade, you have to quit "
                      "the application and install the new package.</p>"
                      "<p>You may do it at your convenience.</p>")
                     .arg(appName()).arg(remoteVer());
    QMessageBox box(TaoApp->windowWidget());
    setBoxMinimumWidth(box, 400);
    box.setIconPixmap(checkmarkIcon);
    box.setWindowTitle(dialogTitle);
    box.setText(msg);
    box.setInformativeText(info);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void UpdateApplication::downloadProgress(qint64 bytesRcvd, qint64 bytesTotal)
// ----------------------------------------------------------------------------
//    Update progress bar
// ----------------------------------------------------------------------------
{
    if (state == Downloading)
    {
        // Calculate the download speed
        // REVISIT: compute speed for the last couple of seconds only?
        double speed = bytesRcvd * 1000.0 / (downloadTime.elapsed() + 1.0);
        QString unit;
        if (speed < 1024) {
            unit = tr("B/s");
        } else if (speed < 1024*1024) {
            speed /= 1024;
            unit = tr("KB/s");
        } else {
            speed /= 1024*1024;
            unit = tr("MB/s");
        }
        QString speedTxt;
        if (speed < 1000)
            speedTxt = QString(" (%1 %2)").arg(speed, 3, 'f', 1).arg(unit);

        // Update progress dialog
        QString msg = tr("Downloading Tao Presentations %1%2");
        progress->setLabelText(msg.arg(remoteVer()).arg(speedTxt));
    }
    progress->setMaximum(bytesTotal);
    progress->setValue(bytesRcvd);
}


void UpdateApplication::networkError(QNetworkReply::NetworkError err)
// ----------------------------------------------------------------------------
//    Handle network error
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Network error " << err << ": "
                << +reply->errorString() << "\n";

    state = (state == WaitingForUpdate) ? NetworkErrorCheck
                                        : NetworkErrorDownload;
}


void UpdateApplication::cancel()
// ----------------------------------------------------------------------------
//    Cancel check for update or download
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
        debug() << "Cancel\n";
    if (file)
    {
        file->remove();
        delete file;
        file = NULL;
    }
    state = Idle;
    if (reply)
    {
        reply->abort();
        reply->deleteLater();
        reply = NULL;
    }
    resetRequest();
}


void UpdateApplication::processReply()
// ----------------------------------------------------------------------------
//    Called as we receive new headers
// ----------------------------------------------------------------------------
{
    // Hande redirections
    code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (code >= 300 && code < 400)
    {
        // Get redirect URL
        url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (!url.isValid())
        {
            // Not yet received
            return;
        }

        // In case URL is relative
        url = reply->url().resolved(url);

        IFTRACE(update)
                debug() << "Redirected to: '" << +url.toString() <<  "'\n";

        request.setUrl(url);
        reply->deleteLater();
        reply = manager->get(request);
        connect(reply, SIGNAL(metaDataChanged()), this, SLOT(processReply()));
        connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(downloadProgress(qint64,qint64)));
    }
    else if (code == 200 && state == Downloading)
    {
        if (!createFile())
        {
            progress->hide();
            cancel();
        }
    }
}


void UpdateApplication::downloadFinished()
// ----------------------------------------------------------------------------
//    Process downloaded data reply
// ----------------------------------------------------------------------------
{
    if (state == NetworkErrorCheck || state == NetworkErrorDownload)
    {
        QString details = reply->errorString();
        reply->abort();
        progress->close();

        if (state == NetworkErrorDownload || show)
        {
            QString msg = tr("<h3>Network error</h3>");
            QString info = tr("<p>Impossible to obtain update information.</p>"
                              "<p>Please make sure that you are connected to "
                              "the Internet and try again.</p>");
            QMessageBox box(TaoApp->windowWidget());
            setBoxMinimumWidth(box, 400);
            box.setIconPixmap(connectionErrorIcon);
            box.setWindowTitle(dialogTitle);
            box.setText(msg);
            box.setInformativeText(info);
            box.setDetailedText(details);
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
        }

        return;
    }

    if (code != 200)
        return;

    bool updateAvailable = false;
    qint64 pid = 0;
    QString name;
    switch (state)
    {
    case WaitingForUpdate:

        // Save and process the update information we have just received

        progress->hide();

        Q_ASSERT(!file);
        pid = QCoreApplication::applicationPid();
        name = QString("tao_update.%1").arg(pid);
        file = new QFile(QDir::temp().filePath(name));
        if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            IFTRACE(update)
                debug() << "Failed to open: " << +file->fileName() << "\n";
            delete file;
            file = NULL;
            return;
        }
        file->write(reply->readAll());
        file->close();

        reply->deleteLater();
        reply = NULL;

        readIniFile();

        file->remove();
        delete file;
        file = NULL;

        setLastChecked(QDateTime::currentDateTime());

        // Propose to update if current version is older than the remote one
        updateAvailable = (version < remoteVersion) && !url.isEmpty();
        IFTRACE(update)
            debug() << "Update available: " << updateAvailable
                    << " (local " << version << " remote " << remoteVersion
                    << ")\n";
        if (updateAvailable)
        {
            bool ok = false;
            double min = QSettings().value("AppUpdatePromptMinVersion", -1.0)
                                    .toDouble(&ok);
            if (!ok)
                min = -1.0;

            if (show || remoteVersion > min)
            {
                if (remoteVersion > min)
                {
                    // Remember that user was prompted to download  this
                    // version. He/she will not be prompted again on application
                    // startup, unless a newer version exists.
                    QSettings().setValue("AppUpdatePromptMinVersion",
                                         remoteVersion);
                    IFTRACE2(update, settings)
                            debug() << "Saving AppUpdatePromptMinVersion="
                                    << remoteVersion << "\n";
                }

                // Show update dialog
                QString msg = tr("<h3>Update available</h3>");
                QString info = tr("<p>%1 version %2 is available."
                                  " Do you want to download it now?</p>")
                        .arg(appName()).arg(remoteVer());
                QMessageBox box(TaoApp->windowWidget());
                setBoxMinimumWidth(box, 400);
                box.setIconPixmap(downloadIcon);
                box.setWindowTitle(dialogTitle);
                box.setText(msg);
                box.setInformativeText(info);
                if (!description.isEmpty())
                    box.setDetailedText(description);
                box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                if (box.exec() == QMessageBox::Yes)
                {
                    startDownload();
                    break;
                }
            }
            else
            {
                IFTRACE(update)
                    debug() << "Update available dialog not shown due to "
                               "AppUpdatePromptMinVersion=" << min << "\n";
            }
        }
        else
        {
            if (show)
                showNoUpdateAvailable();
        }

        progress->close();
        break;

    case Downloading:

        IFTRACE(update)
                debug() << "Download finished\n";

        progress->hide();
        state = Downloaded;
        if (file)
            saveDownloadedData();

        break;

    default:
        break;
    }
}


void UpdateApplication::saveDownloadedData()
// ----------------------------------------------------------------------------
//    Finish download: write data to file, show confirmation, cleanup
// ----------------------------------------------------------------------------
{
    Q_ASSERT(file);

    file->write(reply->readAll());
    file->close();

    showDownloadSuccessful();
    delete file;
    file = NULL;

    reply->deleteLater();
    reply = NULL;

    progress->close();
    state = Idle;
}


bool UpdateApplication::createFile()
// ----------------------------------------------------------------------------
//    Create the update file
// ----------------------------------------------------------------------------
{
    Q_ASSERT(!file);

    // Keep the name of the remote file in the URL
    QString fileName = QFileInfo(url.path()).fileName();

    // Choose folder
    QString desktop = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    QString folder = QFileDialog::getExistingDirectory(NULL,
                                                       tr("Select destination folder"),
                                                       desktop);

    if (!folder.isEmpty())
    {
        // Set complete filename
        QString completeFileName = folder + "/" + fileName;

        file = new QFile(completeFileName);

        if (file->exists())
        {
            QString msg = tr("<h3>File exists</h3>");
            QString info = tr("<p>The selected folder already contains a file "
                              "called <b>%1</b>.</p>"
                              "<p>Save anyway?</p>").arg(fileName);
            QMessageBox box(TaoApp->windowWidget());
            setBoxMinimumWidth(box, 400);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle(dialogTitle);
            box.setText(msg);
            box.setInformativeText(info);
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            box.setDefaultButton(QMessageBox::No);

            if (box.exec() == QMessageBox::No)
            {
                delete file;
                file = NULL;
                return false;
            }
        }

        if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            IFTRACE(update)
                debug() << "Could not open file for writing: '"
                        << +completeFileName
                        << "' error: " << +file->errorString() << "\n";

            QString msg = tr("<h3>Write error</h3>");
            QString info = tr("<p>The upgrade cannot be saved to <b>%1</b>.</p>")
                           .arg(fileName);
            QString details = file->errorString();
            QMessageBox box(TaoApp->windowWidget());
            setBoxMinimumWidth(box, 400);
            box.setIcon(QMessageBox::Critical);
            box.setWindowTitle(dialogTitle);
            box.setText(msg);
            box.setInformativeText(info);
            box.setDetailedText(details);
            box.setStandardButtons(QMessageBox::Ok);

            box.exec();

            delete file;
            file = NULL;
            return false;
        }

        IFTRACE(update)
            debug() << "Save to: '" << +completeFileName << "'\n";

        if (state == Downloaded)
        {
            // Download completed while user was still choosing file path
            saveDownloadedData();
        }

        return true;
    }

    return false;
}


std::ostream & UpdateApplication::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[UpdateApplication] ";
    return std::cerr;
}

}
