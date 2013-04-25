// ****************************************************************************
//  license_download.h                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//     Request and download one or several Tao license file(s) (.taokey) for
//     the current host from a web server.
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include "application.h"
#include "base.h"
#include "license_download.h"
#include "license.h"
#include "license_dialog.h"
#include "login_dialog.h"
#include "tao.h"
#include "tao_utf8.h"
#include <QCoreApplication>
#include <QProgressDialog>
#include <QSettings>
#include <iostream>


namespace Tao {


LicenseDownload::LicenseDownload()
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : canceled(false), manager(new QNetworkAccessManager(this)), code(0)
{
    connect(this, SIGNAL(endLoop()), &loop, SLOT(quit()));
    connect(manager,
            SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            this,
            SLOT(provideAuthentication(QNetworkReply*, QAuthenticator*)));
}


void LicenseDownload::processFiles(const QFileInfoList &files)
// ----------------------------------------------------------------------------
//   Request a license file for each .taokey.src file
// ----------------------------------------------------------------------------
{
    if (files.isEmpty())
    {
        IFTRACE(lic)
            debug() << "No files given\n";
        return;
    }

    int count = 0;
    foreach (QFileInfo file, files)
    {
        emit progressChanged((int)(100*(qreal)count++/files.size()));

        processFile(file);

        if (canceled)
            break;
    }
    emit progressChanged(100);
    canceled = false;
}


void LicenseDownload::processFile(QFileInfo &file)
// ----------------------------------------------------------------------------
//   Download the license described in a file (.taokey.src)
// ----------------------------------------------------------------------------
{
    Source src(file);
    if (src.testing)
        return;
    processSource(src);
}


void LicenseDownload::processAuthString(QString auth)
// ----------------------------------------------------------------------------
//   Download the license described a specially encoded string
// ----------------------------------------------------------------------------
{
    Source src(auth);
    processSource(src);
}


void LicenseDownload::processSource(Source &src)
// ----------------------------------------------------------------------------
//   Download the license described in src
// ----------------------------------------------------------------------------
{
    IFTRACE(lic)
        debug() << "Processing\n";

    if (!src.isValid())
    {
        if (!src.path.isEmpty())
            emit invalidFile(src.path, src.errorString);
        return;
    }
    if (src.targetFile().exists())
    {
        IFTRACE(lic)
            debug() << +src.targetFile().absoluteFilePath() << " exists\n";
        return;
    }

    current = &src;
    request.setUrl(current->url);
    code = 0;
    reply = manager->get(request);
    connectSignals(reply);
    loop.exec();
    current = NULL;
}


void LicenseDownload::connectSignals(QNetworkReply * reply)
// ----------------------------------------------------------------------------
//    Connect signals of reply object to 'this'
// ----------------------------------------------------------------------------
{
    connect(reply, SIGNAL(metaDataChanged()), this, SLOT(replyUpdated()));
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
}


void LicenseDownload::cancel()
// ----------------------------------------------------------------------------
//   Stop current download
// ----------------------------------------------------------------------------
{
    IFTRACE(lic)
        debug() << "cancel()\n";
    canceled = true;
    emit endLoop();
}


void LicenseDownload::replyUpdated()
// ----------------------------------------------------------------------------
//    Called as we receive new headers
// ----------------------------------------------------------------------------
{
    // Hande redirections
    code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (code >= 300 && code < 400)
    {
        disconnect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));

        // Do we have a valid redirect URL yet?
        QUrl url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute)
                        .toUrl();
        if (!url.isValid())
            return;

        // In case URL is relative
        url = reply->url().resolved(url);

        IFTRACE(lic)
            debug() << "HTTP " << code << " redirect to: " << +url.toString()
                    <<  "\n";

        request.setUrl(url);
        reply->deleteLater();
        reply = manager->get(request);
        connectSignals(reply);
    }
}


void LicenseDownload::replyFinished()
// ----------------------------------------------------------------------------
//   HTTP response fully received
// ----------------------------------------------------------------------------
{
    IFTRACE(lic)
    {
        debug() << "replyFinished()\n";
    }

    if (code == 200)
    {
        Q_ASSERT(current);
        QString path(current->targetFile().absoluteFilePath());
        QByteArray data(reply->readAll());
        QByteArray invalid("signature \"INVALID-TESTING-ONLY\"");
        if (!data.contains(invalid))
        {
            saveFile(path, data);
        }
        else
        {
            IFTRACE(lic)
                debug() << "Invalid license file received - not saved\n";
        }
    }

    emit endLoop();
}


void LicenseDownload::onNetworkError(QNetworkReply::NetworkError err)
// ----------------------------------------------------------------------------
//   A network error occured.
// ----------------------------------------------------------------------------
{
    QString errStr = reply->errorString();
    IFTRACE(lic)
        debug() << "Network error " << err << ": " << +errStr << "\n";

    emit networkError(err, current->path, errStr + "\n" +
                      "~~~~~~~~~~~~~~~~~~~~\n" +
                      QString::fromUtf8(reply->readAll().data()) +
                      "~~~~~~~~~~~~~~~~~~~~\n" +
                      current->toString());

    // Note: reply still emits the finished() signal after networkError()
}


void LicenseDownload::provideAuthentication(QNetworkReply *reply,
                                            QAuthenticator *authenticator)
// ----------------------------------------------------------------------------
//   Called when server requests authentication
// ----------------------------------------------------------------------------
{
    Q_UNUSED(reply);
    Q_ASSERT(current);

    QString & user = current->user;
    QString & password = current->password;
    if (current->nbAuth > 0 || user.isEmpty() || password.isNull())
    {
        // No user/password, or previous attempt failed
        IFTRACE(lic)
            debug() << "Authentication required\n";
        Authenticator auth(*authenticator, user, password);
        emit authenticationRequired(reply, &auth);

        if (!auth.changed())
        {
            disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                       this,
                       SLOT(onNetworkError(QNetworkReply::NetworkError)));
            reply->abort();
            return;
        }
    }
    authenticator->setUser(user);
    authenticator->setPassword(password);
    current->nbAuth++;
}


void LicenseDownload::saveFile(QString path, QByteArray data)
// ----------------------------------------------------------------------------
//   Save license file received from the server
// ----------------------------------------------------------------------------
{
    IFTRACE(lic)
        debug() << "Saving " << +path << "\n";
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (file.write(data))
        {
            file.close();
            return;
        }
    }
    std::cerr << +tr("Could not save license file %1: %2")
                 .arg(path)
                 .arg(file.errorString()) << "\n";
}


std::ostream & LicenseDownload::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[LicenseDownload] ";
    return std::cerr;
}



// ============================================================================
//
//   LicenseDownload::Source
//
// ============================================================================

/*
; Input file read by LicenseDownload::Source::Source(QFileInfo &):
; .ini format (read by QSettings), encoding: UTF-8.

; Service URL (optional)
;url=http://taodyne.com/licenses/gen.php?bundle=$bundle&hostid=$hostid

; Which license to request (mandatory)
bundle=<bundle name>

; User name and password for HTTP digest authentication. Both are optional.
; User will be prompted if:
;   - user is empty or unspecified, or
;   - password is unspecified.
; Note: an empty password is valid and does not cause a prompt by itself.
user=<user name>
password=<password>

; Test mode (optional). If defined:
; - the file is parsed but ignored (no license is downloaded).
; - 'tao -tlic' shows the 'authkey' value
;testing=

; Base64 encoded form of above values (optional).
; When authkey is defined, url/bundle/user/password are ignored (if present).
;authkey=
 */

static const char encodingSeparator = '\0';

LicenseDownload::Source::Source(QFileInfo &file)
// ----------------------------------------------------------------------------
//   Extract information from a license source file
// ----------------------------------------------------------------------------
    : testing(false), path(file.absoluteFilePath()), nbAuth(0)
{
    if (file.isReadable())
    {
        QSettings settings(file.absoluteFilePath(), QSettings::IniFormat);
        settings.setIniCodec("UTF-8");

        QString authKey = settings.value("authkey").toString();
        if (authKey.isNull())
        {
            bundle = settings.value("bundle").toString();
            urlStr = settings.value("url").toString();
            url = makeUrl(urlStr.isEmpty() ? defaultUrlStr() : urlStr);
            user = settings.value("user").toString();
            password = settings.value("password").toString();
        }
        else
        {
            parseEncoded(authKey);
        }
        testing = !settings.value("testing").toString().isNull();
    }
    checkValid();
    IFTRACE(lic)
        debug() << +toString() << "\n";
}


LicenseDownload::Source::Source(QString auth)
// ----------------------------------------------------------------------------
//   Construct license Source from an encoded string
// ----------------------------------------------------------------------------
    : testing(false), nbAuth(0)
{
    parseEncoded(auth);
}


LicenseDownload::Source::Source(QString bundle, QString user, QString password,
                                QString urlStr)
// ----------------------------------------------------------------------------
//   Construct license Source from various information
// ----------------------------------------------------------------------------
    : urlStr(urlStr), user(user), password(password), bundle(bundle),
      testing(false), nbAuth(0)
{
    url = makeUrl(urlStr);
    checkValid();
}


bool LicenseDownload::Source::parseEncoded(QString auth)
// ----------------------------------------------------------------------------
//   Extract urlStr, bundle, user etc. from ancoded string 'auth'
// ----------------------------------------------------------------------------
{
    IFTRACE(lic)
        debug() << "parseEncoded(" << +auth << ")\n";

    QString invalid = tr("Invalid authorization string @%1:%2").arg(__FILE__);
    QByteArray ba = QByteArray::fromBase64(auth.toUtf8());
    unsigned int sz = ba.size();
    if (sz < 3)
    {
        errorString = invalid.arg(__LINE__);
        return false;
    }

    quint8 hi = ba[sz-1];
    quint8 lo = ba[sz-2];
    quint16 sum = (hi << 8) | lo;
    quint16 expected = qChecksum(ba.data(), sz - 2);
    if (sum != expected)
    {
        errorString = invalid.arg(__LINE__);
        return false;
    }

    ba.chop(3);
    QList<QByteArray> array = ba.split(encodingSeparator);
    if (array.size() % 2)
    {
        errorString = invalid.arg(__LINE__);
        return false;
    }
    for (int i = 0; i < array.size(); i += 2)
    {
        bool ok = false;
        int id = array[i].toInt(&ok);
        if (!ok)
        {
            errorString = invalid.arg(__LINE__);
            return false;
        }
        QByteArray value = array[i+1];
        switch (id)
        {
        case 0:
            urlStr = QString::fromUtf8(value.data());
            break;
        case 1:
            bundle = QString::fromUtf8(value.data());
            break;
        case 2:
            user = QString::fromUtf8(value.data());
            break;
        case 3:
            password = QString::fromUtf8(value.data());
            break;
        default:
            break;
        }
    }
    url = makeUrl(urlStr.isEmpty() ? defaultUrlStr() : urlStr);
    return true;
}


QUrl LicenseDownload::Source::makeUrl(QString str)
// ----------------------------------------------------------------------------
//   Replace bundle and host ID in raw URL
// ----------------------------------------------------------------------------
{
    QString tmp(str);
    tmp.replace("$bundle", bundle);
    tmp.replace("$hostid", +Licenses::hostID());
    return QUrl(tmp);
}


QString LicenseDownload::Source::defaultUrlStr()
// ----------------------------------------------------------------------------
//   The URL to use when not specified
// ----------------------------------------------------------------------------
{
    return "http://taodyne.com/licenses/gen.php?bundle=$bundle&hostid=$hostid";
}


std::ostream & LicenseDownload::Source::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[LicenseDownload::Source] ";
    return std::cerr;
}


void LicenseDownload::Source::checkValid()
// ----------------------------------------------------------------------------
//   Check the mandatory fields for validity, update errorString
// ----------------------------------------------------------------------------
{
    if (!errorString.isEmpty())
        return;
    if (!url.isValid())
        errorString = tr("Invalid URL");
    else if (bundle.isNull())
        errorString = tr("Name of license bundle is missing");
    else if (bundle.isEmpty())
        errorString = tr("Name of license bundle is empty");
}


bool LicenseDownload::Source::isValid()
// ----------------------------------------------------------------------------
//   Is the object ready to be used to request the license file?
// ----------------------------------------------------------------------------
{
    return errorString.isEmpty();
}


QString LicenseDownload::Source::urlHash()
// ----------------------------------------------------------------------------
//   8-character hex hash of the URL, empty if URL is invalid
// ----------------------------------------------------------------------------
{
    if (!url.isValid())
        return "";

    QCryptographicHash hash(QCryptographicHash::Md5);
    QString urlStr = url.toString();
    hash.addData(urlStr.toUtf8());
    QByteArray res = hash.result();
    const char * p = res.data();
    QByteArray buf;
    for (int i = 0; i < 4; i++)
        buf[i] = p[i] ^ p [i+4] ^ p[i+8] ^ p[i+12];
    return buf.toHex();
}


QFileInfo LicenseDownload::Source::targetFile()
// ----------------------------------------------------------------------------
//   Where to save the license file for this source
// ----------------------------------------------------------------------------
{
    return QFileInfo(Application::userLicenseFolderPath() + "/" +
                     "dl-" + urlHash() + ".taokey");
}


QString LicenseDownload::Source::toString()
// ----------------------------------------------------------------------------
//   Format object as a human readble string
// ----------------------------------------------------------------------------
{
    QString ret;

    if (!path.isEmpty())
        ret += "Path: " + path;

    if (isValid())
    {
        ret +=  "\nURL: " + url.toString() + " (hash " + urlHash() + ")";
        ret += "\nBundle: " + bundle;
        ret += "\nUser: ";
        ret += (user.isNull() ? "(null)" : user);
        ret += "\nPassword: ";
        if (password.isNull())
            ret += "(null)";
        else
            if (!password.isEmpty())
                ret += "******";
        if (testing)
        {
            ret += "\nTesting: yes";
            ret += "\nEncoded: " + toEncodedString();
        }
    }
    else
    {
        ret += " ** invalid ** (" + errorString + ")";
    }

    return ret;
}


QString LicenseDownload::Source::toEncodedString()
// ----------------------------------------------------------------------------
//   Format object as a string suitable for Source::Source(QString)
// ----------------------------------------------------------------------------
{
    QStringList values;
    values << urlStr << bundle << user << password;

    QByteArray ba;
    int n = 0;
    foreach (QString val, values)
    {
        if (!val.isNull())
        {
            ba.append(QString("%1").arg(n).toUtf8());
            ba.append(encodingSeparator);
            ba.append(val.toUtf8());
            ba.append(encodingSeparator);
        }
        n++;
    }
    if (ba.size())
    {
        // Append CRC16 in big-endian order
        quint16 sum = qChecksum(ba.data(), ba.size());
        quint8 lo = sum & 0xFFU;
        quint8 hi = (sum & 0xFF00U) >> 8;
        ba.append(lo);
        ba.append(hi);
    }

    return ba.toBase64();
}



// ============================================================================
//
//   License download with user interface (dialogs)
//
// ============================================================================

LicenseDownloadUI::LicenseDownloadUI()
    : userPassChanged(false)
{
    connect(&dl, SIGNAL(invalidFile(QString,QString)),
            this, SLOT(showFileInvalid(QString,QString)));
    connect(&dl,
            SIGNAL(networkError(QNetworkReply::NetworkError,QString,QString)),
            this,
            SLOT(showNetworkError(QNetworkReply::NetworkError,QString,QString)));
    connect(&dl,
            SIGNAL(authenticationRequired(QNetworkReply*,LicenseDownload::Authenticator*)),
            this,
            SLOT(authenticate(QNetworkReply*,LicenseDownload::Authenticator*)));
    progress = new QProgressDialog(tr("Fetching licenses"),
                                   tr("Abort"), 0, 100, NULL);
    progress->setWindowTitle(QCoreApplication::applicationName());
    connect(progress, SIGNAL(canceled()),
            &dl, SLOT(cancel()));
    connect(&dl, SIGNAL(progressChanged(int)),
            progress, SLOT(setValue(int)));

}


void LicenseDownloadUI::processFiles(const QFileInfoList &files)
// ----------------------------------------------------------------------------
//   Request a license file for each .taokey.src file
// ----------------------------------------------------------------------------
{
    dl.processFiles(files);
}


void LicenseDownloadUI::showFileInvalid(QString path, QString reason)
// ----------------------------------------------------------------------------
//   Tell user to remove invalid .taokey.src file
// ----------------------------------------------------------------------------
{
    QString message;
    message  = tr("<h3>License Error</h3>");
    message += tr("<p>There is a problem with license file:</p>"
                  "<center>'%1'</center>"
                  "<p>The following error was detected: %2.</p>"
                  "<p>The program will now terminate. "
                  "You need to remove the offending license file "
                  "before trying to run the application again.</p>")
                  .arg(path).arg(reason);
    message += tr("<p>Please contact "
                  "<a href=\"http://taodyne.com/\">Taodyne</a> "
                  "to obtain valid license files.</p>");
    LicenseDialog oops(message);
    oops.show();
    oops.raise();
    oops.exec();
    ::exit(16);
}


void LicenseDownloadUI::showNetworkError(QNetworkReply::NetworkError err,
                                         QString path, QString details)
// ----------------------------------------------------------------------------
//   Tell user that a network error prevented the download of a license file
// ----------------------------------------------------------------------------
{
    QString txt = tr("License download error.");
    // HACK - QTBUG-7851 - To get a wider dialog box
    int n = 150 - txt.size();
    if (n > 0)
        for (int i = 0; i < n; i++)
            txt += " ";

    QString info;
    if (err == QNetworkReply::ContentOperationNotPermittedError)
    {
        info = tr("The license described in file '%1' could not "
                  "be downloaded. The most probable cause is "
                  "that the maximum number of licences allowed "
                  "was exceeded. Please contact Taodyne to "
                  "obtain more licenses.\n"
                  "The download will be attempted again the next "
                  "time the application is run. Delete the "
                  "above file if you do not want this to occur.")
                .arg(path);
    }
    else
    {
        info = tr("Due to a network issue, the license "
                  "described in file '%1' could not be "
                  "downloaded.\n"
                  "The download will be attempted again the "
                  "next time the application is run.")
                .arg(path);
    }
    QMessageBox warn;
    warn.setWindowTitle(QCoreApplication::applicationName());
    warn.setText(txt);
    warn.setInformativeText(info);
    warn.setDetailedText(details);
    warn.exec();
}


void LicenseDownloadUI::authenticate(QNetworkReply *reply,
                                     LicenseDownload::Authenticator *auth)
// ----------------------------------------------------------------------------
//   Ask username/password because LicenseDownload needs them
// ----------------------------------------------------------------------------
{
    QString host = reply->url().host();

    LoginDialog * dlg = new LoginDialog;
    dlg->setWindowTitle(tr("Tao License Download"));
    dlg->setDescription(tr("Please enter a user name and a password "
                           "to obtain a license file from %1.")
                        .arg(host));
    dlg->setUsername(auth->user());
    dlg->setPassword(auth->password());
    connect(dlg, SIGNAL(acceptLogin(QString&,QString&,int&)),
            this, SLOT(acceptLogin(QString&,QString&)));
    userPassChanged = false;
    dlg->exec();
    dlg->deleteLater();
    if (!userPassChanged)
        return;

    auth->setUser(user);
    auth->setPassword(password);
}


void LicenseDownloadUI::acceptLogin(QString &user, QString &password)
// ----------------------------------------------------------------------------
//   License download: called when LoginDialog completes
// ----------------------------------------------------------------------------
{
    this->user = user;
    this->password = password;
    userPassChanged = true;
}

}
