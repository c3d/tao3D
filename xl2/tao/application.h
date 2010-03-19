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


TAO_BEGIN

struct Widget;
struct Repository;


class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{

public:
    Application(int & argc, char ** argv);
    ~Application();

    bool        openLibrary();
    bool        openLibrary(QString path, bool confirm = true);
    QDir        libraryDirectory();
    Repository *library()       { return repository; }

    void        internalCleanEverythingAsIfTaoWereNeverRun();

protected:
    bool        recursiveDelete(QString path);
    QString     defaultDocumentsFolderPath()
    QString     defaultDocumentLibraryPath();
    QString     userDocumentLibraryPath();

private:
    Repository  *repository;    // REVISIT: One per application or per doc?
};

#define TaoApp  ((Application *) qApp)

TAO_END

#endif // APPLICATION_H
