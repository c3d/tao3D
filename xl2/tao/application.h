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


class Application : public QApplication
// ----------------------------------------------------------------------------
//    The main Tao application
// ----------------------------------------------------------------------------
{

public:
    Application(int & argc, char ** argv);

    void           internalCleanEverythingAsIfTaoWereNeverRun();
    static QString defaultProjectFolderPath();

protected:
    static bool    recursiveDelete(QString path);
    static QString defaultDocumentsFolderPath();
    static bool    createDefaultProjectFolder();
};

#define TaoApp  ((Application *) qApp)

TAO_END

#endif // APPLICATION_H
