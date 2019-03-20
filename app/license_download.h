#ifndef LICENSE_DOWNLOAD_H
#define LICENSE_DOWNLOAD_H
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "http_ua.h"
#include <QEventLoop>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QFileInfo>
#include <QFileInfoList>
#include <QtNetwork>

class QProgressDialog;

namespace Tao {


class LicenseDownload : public QObject, HttpUserAgent
// ------------------------------------------------------------------------
//   Download Tao license files from a web server
// ------------------------------------------------------------------------
{
    Q_OBJECT

public:
    LicenseDownload();
    ~LicenseDownload() {}

    void           processFiles(const QFileInfoList &files);
    void           processFile(QFileInfo &files);
    void           processAuthString(QString auth);

public slots:
    void           cancel();

public:
    // To fill login dialog with previous user name/login values in case of
    // several authentication attempts
    struct Authenticator {

        Authenticator(QAuthenticator &auth,
                      QString &user, QString &password)
            : _auth(auth), _user(user), _password(password),
              _changed(false) {}

        QString user() { return _user; }
        QString password() { return _password; }

        void setUser(QString user)
            { _user = user ; _auth.setUser(_user); _changed = true; }

        void setPassword(QString password)
            { _password = password ; _auth.setPassword(_password);
              _changed = true; }

        bool changed() { return _changed; }

    protected:
        QAuthenticator & _auth;
        QString & _user;
        QString & _password;
        bool      _changed;
    };

signals:
    void           invalidFile(QString path, QString reason);
    void           networkError(QNetworkReply::NetworkError err,
                                QString path, QString details);
    void           progressChanged(int progress);
    void           authenticationRequired(QNetworkReply *reply,
                                          LicenseDownload::Authenticator *auth);

    // Private
    void           endLoop();

protected:
    // Information about a license source
    struct Source {

        Source(QFileInfo &file);
        Source(QString auth);
        Source(QString bundle, QString user, QString password = QString(),
               QString urlStr = QString());

        QString     toString();
        QString     toEncodedString();
        bool        isValid();
        QString     urlHash();
        QFileInfo   targetFile();

    protected:
        void        checkValid();
        QString     defaultUrlStr();
        QUrl        makeUrl(QString str);
        bool        parseEncoded(QString auth);
        std::ostream & debug();

    public:
        // Raw parameters read from file or passed to constructor
        QString     urlStr;
        QString     user;
        QString     password;
        QString     bundle;
        bool        testing;

        QString     path;
        QUrl        url;
        bool        loginInfoUpdated;
        int         nbAuth;
        QString     errorString;
    };

protected:
    void           processSource(Source &src);
    void           connectSignals(QNetworkReply * reply);
    void           saveFile(QString path, QByteArray data);

protected slots:
    void           replyUpdated();
    void           replyFinished();
    void           onNetworkError(QNetworkReply::NetworkError err);
    void           provideAuthentication(QNetworkReply * reply,
                                         QAuthenticator * authenticator);

private:
    std::ostream & debug();

private:
    QEventLoop               loop;
    bool                     canceled;
    Source *                 current;

    // Network
    QNetworkReply *          reply;
    QNetworkRequest          request;
    QNetworkAccessManager *  manager;
    int                      code;
    QString                  errorString;

};



struct LicenseDownloadUI : QObject
// ------------------------------------------------------------------------
//   LicenseDownload with a graphical user interface (login/error dialogs)
// ------------------------------------------------------------------------
{
private:
    Q_OBJECT

public:
    LicenseDownloadUI();
    ~LicenseDownloadUI() {}

    void           processFiles(const QFileInfoList &files);

protected slots:
    void           showFileInvalid(QString path, QString reason);
    void           showNetworkError(QNetworkReply::NetworkError err,
                                    QString path, QString details);
    void           authenticate(QNetworkReply *reply,
                                LicenseDownload::Authenticator *auth);
    void           acceptLogin(QString &user, QString &password);

protected:
    LicenseDownload    dl;
    QProgressDialog *  progress;
    QString            user, password;
    bool               userPassChanged;
};

}

#endif // LICENSE_DOWNLOAD_H
