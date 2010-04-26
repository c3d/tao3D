#ifndef APPLICATION_H
#define APPLICATION_H
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

#include "tao.h"

#include <QApplication>
#include <QDir>
#include <QStringList>


TAO_BEGIN

struct Widget;


class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{

public:
    Application(int & argc, char ** argv);
    virtual ~Application();

    void           internalCleanEverythingAsIfTaoWereNeverRun();
    static QString defaultProjectFolderPath();
    static QString defaultTaoPreferencesFolderPath();
    static QString defaultTaoApplicationFolderPath();
    static QString defaultUserImagesFolderPath();

public:
    QStringList    pathCompletions();
    QStringList    urlCompletions();
    void           addPathCompletion(QString path);
    void           addUrlCompletion(QString url);

protected:
    void           saveSettings();
    void           loadSettings();

protected:
    static bool    recursiveDelete(QString path);
    static QString defaultUserDocumentsFolderPath();
    static QString defaultPreferencesFolderPath();
    static bool    createDefaultProjectFolder();

public:
    QString      currentProjectFolder;
    static bool  createDefaultTaoPrefFolder();

private:
    QStringList  pathList;
    QStringList  urlList;
};

#define TaoApp  ((Application *) qApp)

TAO_END

#endif // APPLICATION_H
