// ****************************************************************************
//  preferences_pages.cpp                                          Tao project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of Preferences dialog pages.
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtGui>
#include "preferences_pages.h"
#include "main.h"
#include "application.h"


namespace Tao {

// ============================================================================
//
//   The debug page is used to show and modify the state of debug traces
//
// ============================================================================

DebugPage::DebugPage(QWidget *parent)
     : QWidget(parent)
// ----------------------------------------------------------------------------
//   Create the page and show the current state of debug traces
// ----------------------------------------------------------------------------
{
    QStringList traces = allTraceNames();

    QGroupBox *tracesGroup = new QGroupBox(tr("Debug traces"));
    QGridLayout *tracesGridLayout = new QGridLayout;
    int row = 0, col = 0;
    foreach (QString t, traces)
    {
        QCheckBox *b = new QCheckBox(t);
        b->setObjectName("traceCheckBox");
        b->setChecked(isTraceEnabled(t));
        connect(b, SIGNAL(toggled(bool)), this, SLOT(toggleTrace(bool)));
        tracesGridLayout->addWidget(b, row, col);
        if (col++ == 4)
            col = 0, row++;
    }
    tracesGroup->setLayout(tracesGridLayout);

    QWidget *buttonsWidget = new QWidget;
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QPushButton *selectAll = new QPushButton(tr("Select all"));
    connect(selectAll, SIGNAL(clicked()), this, SLOT(enableAllTraces()));
    buttonsLayout->addWidget(selectAll);
    QPushButton *deselectAll = new QPushButton(tr("Deselect all"));
    connect(deselectAll, SIGNAL(clicked()), this, SLOT(disableAllTraces()));
    buttonsLayout->addWidget(deselectAll);
    buttonsLayout->addSpacing(12);
    QPushButton *save = new QPushButton(tr("Save for next run"));
    connect(save, SIGNAL(clicked()), TaoApp, SLOT(saveDebugTraceSettings()));
    buttonsLayout->addWidget(save);
    buttonsWidget->setLayout(buttonsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tracesGroup);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(buttonsWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}


QStringList DebugPage::allTraceNames()
// ----------------------------------------------------------------------------
//   Return the list of all Tao trace names
// ----------------------------------------------------------------------------
{
    QStringList ret;

    std::set<std::string>::iterator it;
    std::set<std::string> all = XL::Traces::names();
    for (it = all.begin(); it != all.end(); it++)
        ret << +*it;

    return ret;
}


void DebugPage::toggleTrace(bool on)
// ----------------------------------------------------------------------------
//   Toggle debug trace when state of a checkbox is changed
// ----------------------------------------------------------------------------
{
    QCheckBox *b = (QCheckBox *)sender();
    QString toggled = b->text();

    XL::Traces::enable(+toggled, on);
}


bool DebugPage::isTraceEnabled(QString tname)
// ----------------------------------------------------------------------------
//   Return true if trace is currently enabled
// ----------------------------------------------------------------------------
{
    return XL::Traces::enabled(+tname);
}


void DebugPage::toggleAllTraces(bool on)
// ----------------------------------------------------------------------------
//   Set or clear all trace flags
// ----------------------------------------------------------------------------
{
    QList<QCheckBox *> boxes = findChildren<QCheckBox *>("traceCheckBox");
    foreach (QCheckBox *b, boxes)
        b->setChecked(on);
}


void DebugPage::enableAllTraces()
// ----------------------------------------------------------------------------
//   Set all trace flags
// ----------------------------------------------------------------------------
{
    toggleAllTraces(true);
}


void DebugPage::disableAllTraces()
// ----------------------------------------------------------------------------
//   Clear all trace flags
// ----------------------------------------------------------------------------
{
    toggleAllTraces(false);
}

// ============================================================================
//
//   The modules page shows all modules and allow some operations on them
//
// ============================================================================

ModulesPage::ModulesPage(QWidget *parent)
     : QWidget(parent), findUpdatesInProgress(false)
// ----------------------------------------------------------------------------
//   Create the page and show a list of modules
// ----------------------------------------------------------------------------
{
    mmgr = ModuleManager::moduleManager();
    modules = mmgr->allModules();

    QGroupBox *gb = new QGroupBox(tr("Installed modules"));
    QVBoxLayout *vbLayout = new QVBoxLayout;

    table = new QTableWidget;
    table->setColumnCount(5);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem("Name"));
    table->setHorizontalHeaderItem(3, new QTableWidgetItem("Version"));
    table->setHorizontalHeaderItem(4, new QTableWidgetItem("Status"));
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
    updateTable();
    vbLayout->addWidget(table);
    gb->setLayout(vbLayout);

    QWidget *cfuWidget = new QWidget;
    QHBoxLayout *cfuLayout = new QHBoxLayout;
    QPushButton *findUpdates = new QPushButton(tr("Check for updates"));
    connect(findUpdates, SIGNAL(clicked()), this, SLOT(findUpdates()));
    if (!modules.count())
        findUpdates->setEnabled(false);
    cfuLayout->addWidget(findUpdates);
    cfuLayout->addSpacing(12);
    sw = new QStackedWidget;
    sw->insertWidget(0, new QFrame);
    sw->insertWidget(1, (pb = new QProgressBar));
    cfuLayout->addWidget(sw);
    cfuWidget->setLayout(cfuLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gb);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(cfuWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}


void ModulesPage::toggleModule()
// ----------------------------------------------------------------------------
//   Update module enabled/disabled state when button is clicked
// ----------------------------------------------------------------------------
{
    QAction *act = (QAction *)sender();
    int row = act->data().toInt();
    ModuleManager::ModuleInfoPrivate m = modules[row];

    mmgr->setEnabled(m.id, !m.enabled);
    updateTable();
}


void ModulesPage::findUpdates()
// ----------------------------------------------------------------------------
//   Start looking for updates
// ----------------------------------------------------------------------------
{
    if (findUpdatesInProgress)
        return;
    findUpdatesInProgress = true;

    sw->setCurrentIndex(1);
    CheckAllForUpdate *cafu = new CheckAllForUpdate(*mmgr);
    connect(cafu, SIGNAL(complete(bool)), this, SLOT(onCFUComplete()));
    connect(cafu, SIGNAL(minimum(int)),   pb,   SLOT(setMinimum(int)));
    connect(cafu, SIGNAL(maximum(int)),   pb,   SLOT(setMaximum(int)));
    connect(cafu, SIGNAL(progress(int)),  pb,   SLOT(setValue(int)));
    cafu->start();
}


void ModulesPage::updateTable()
// ----------------------------------------------------------------------------
//   Fill module table
// ----------------------------------------------------------------------------
{
    modules = mmgr->allModules();
    table->setRowCount(modules.count());
    int row = 0;
    foreach (ModuleManager::ModuleInfoPrivate m, modules)
    {
        Qt::ItemFlags enFlag = m.enabled ? Qt::ItemIsEnabled : Qt::NoItemFlags;

        QTableWidgetItem *item = new QTableWidgetItem;
        QString txt = m.enabled ? tr("Disable") : tr("Enable");
        QToolButton *b = new QToolButton;
        QAction *act = new QAction(txt, this);
        act->setData(QVariant(row));
        connect(act, SIGNAL(triggered()), this, SLOT(toggleModule()));
        b->setDefaultAction(act);
        table->setCellWidget(row, 0, b);

        if (m.icon == "")
            m.icon = ":/images/modules.png";
        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setIcon(QIcon(m.icon));
        item->setFlags(enFlag);
        table->setItem(row, 1, item);

        item = new QTableWidgetItem(m.name);
        item->setFlags(enFlag);
        table->setItem(row, 2, item);

        item = new QTableWidgetItem(m.ver);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(enFlag);
        table->setItem(row, 3, item);

        QWidget *widget = NULL;
        if (m.updateAvailable)
        {
            QString msg = QString("Update to %1").arg(m.latest);
            QToolButton *b = new QToolButton;
            QAction *act = new QAction(msg, this);
            act->setData(QVariant(row));
            connect(act, SIGNAL(triggered()), this, SLOT(updateOne()));
            b->setDefaultAction(act);
            widget = b;
        }
        table->setCellWidget(row, 4, widget);

        row++;
    }
    for (int i = 0; i < table->columnCount() - 1; i++)
        table->resizeColumnToContents(i);
    table->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}


void ModulesPage::onCFUComplete()
// ----------------------------------------------------------------------------
//   Leave time for progress bar to show 100%
// ----------------------------------------------------------------------------
{
    QTimer::singleShot(200, this, SLOT(endCheckForUpdate()));
}


void ModulesPage::endCheckForUpdate()
// ----------------------------------------------------------------------------
//   Hide progress bar, refresh module list
// ----------------------------------------------------------------------------
{
    findUpdatesInProgress = false;
    sw->setCurrentIndex(0);
    updateTable();
}


void ModulesPage::updateOne()
// ----------------------------------------------------------------------------
//   Start update for single module
// ----------------------------------------------------------------------------
{
    QAction *act = (QAction *)sender();
    int row = act->data().toInt();
    ModuleManager::ModuleInfoPrivate m = modules[row];
    act->setEnabled(false);
    UpdateModule *up = new UpdateModule(*mmgr, m.id);
    connect(up, SIGNAL(complete(bool)), this, SLOT(onUpdateOneComplete()));
    up->start();
}


void ModulesPage::onUpdateOneComplete()
// ----------------------------------------------------------------------------
//   Delete module updater object, refresh module list
// ----------------------------------------------------------------------------
{
    UpdateModule *up = (UpdateModule *)sender();
    up->deleteLater();
    updateTable();
}

}
