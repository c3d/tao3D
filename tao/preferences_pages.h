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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "module_manager.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QLineEdit;
class QStackedWidget;
class QProgressBar;
class QComboBox;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

namespace Tao {

class GeneralPage : public QWidget
// ----------------------------------------------------------------------------
//   Page to show and modify general settings
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    GeneralPage(QWidget *parent = 0);

private:
    QStringList  installedLanguages();

private slots:
    void         setLanguage(int index);

private:
    QComboBox *  combo;
    QLabel *     message;
};

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
    void         saveClicked();

private:
    QPushButton *save;
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
    void         onCFUComplete(bool updatesAvailable);
    void         endCheckForUpdate();
    void         updateOne();
    void         onUpdateOneComplete(bool success);
    void         onCellClicked(int row, int col);
    void         doSearch();

private:
    void         updateTable();

private:
    ModuleManager *                          mmgr;
    QList<ModuleManager::ModuleInfoPrivate>  modules;
    QTableWidget *                           table;
    QLineEdit *                              search;
    QStackedWidget *                         sw;
    QLabel *                                 lb;
    QProgressBar *                           pb;
    bool                                     findUpdatesInProgress;
};
#endif // !CFG_NOMODPREF


class PerformancesPage : public QWidget
// ----------------------------------------------------------------------------
//   Show OpenGL information and allow configuration of performance parameters
// ----------------------------------------------------------------------------
{

    Q_OBJECT

public:
    PerformancesPage(QWidget *parent = 0);
};

}

#endif
