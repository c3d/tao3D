#ifndef PREFERENCES_PAGES_H
#define PREFERENCES_PAGES_H
// ****************************************************************************
//  preferences_pages.h                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Various pages for the Preferences Dialog.
//     Largely inspired from the Qt "Configuration Dialog" example.
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "module_manager.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QStackedWidget;
class QProgressBar;
QT_END_NAMESPACE

namespace Tao {

class DebugPage : public QWidget
// ----------------------------------------------------------------------------
//   Page to show and modify the state of debug traces
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    DebugPage(QWidget *parent = 0);

private:
    QStringList  allTraceNames();
    bool         isTraceEnabled(QString tname);
    void         toggleAllTraces(bool on);

private slots:
    void         toggleTrace(bool on);
    void         enableAllTraces();
    void         disableAllTraces();
};


#ifndef CFG_NOMODPREF
class ModulesPage : public QWidget
// ----------------------------------------------------------------------------
//   Show all configured Tao modules and allow some operations
// ----------------------------------------------------------------------------
{

    Q_OBJECT

public:
    ModulesPage(QWidget *parent = 0);

private slots:
    void         toggleModule();
    void         findUpdates();
    void         onCFUComplete();
    void         endCheckForUpdate();
    void         updateOne();
    void         onUpdateOneComplete();

private:
    void         updateTable();

private:
    ModuleManager *                   mmgr;
    QList<ModuleManager::ModuleInfo>  modules;
    QTableWidget *                    table;
    QStackedWidget *                  sw;
    QProgressBar *                    pb;
    bool                              findUpdatesInProgress;
};
#endif // !CFG_NOMODPREF

}

#endif
