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

#define OPTVAR(name, type, value)
#define OPTION(name, descr, code)
#define TRACE(name) ret.append(#name);

#include "options.tbl"

#undef OPTVAR
#undef OPTION
#undef TRACE

    return ret;
}


void DebugPage::toggleTrace(bool on)
// ----------------------------------------------------------------------------
//   Toggle debug trace when state of a checkbox is changed
// ----------------------------------------------------------------------------
{
    QCheckBox *b = (QCheckBox *)sender();
    QString toggled = b->text();

#define OPTVAR(name, type, value)
#define OPTION(name, descr, code)
#define TRACE(name) if (#name == toggled) XL::MAIN->options.traces.name = on;

#include "options.tbl"

#undef OPTVAR
#undef OPTION
#undef TRACE
}


bool DebugPage::isTraceEnabled(QString tname)
// ----------------------------------------------------------------------------
//   Return true if trace is currently enabled
// ----------------------------------------------------------------------------
{
#define OPTVAR(name, type, value)
#define OPTION(name, descr, code)
#define TRACE(name) if (#name == tname) return XL::MAIN->options.traces.name;

#include "options.tbl"

#undef OPTVAR
#undef OPTION
#undef TRACE

    return false;
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
    table->setHorizontalHeaderItem(0, new QTableWidgetItem("Enabled"));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem("Name"));
    table->setHorizontalHeaderItem(3, new QTableWidgetItem("Version"));
    table->setHorizontalHeaderItem(4, new QTableWidgetItem("Status"));
    table->horizontalHeader()->setStretchLastSection(true);
    table->setRowCount(modules.count());
    table->verticalHeader()->hide();
    updateTable();
    connect(table, SIGNAL(cellClicked(int,int)),
            this, SLOT(onCellClicked(int,int)));
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


void ModulesPage::onCellClicked(int row, int col)
// ----------------------------------------------------------------------------
//   Update module enabled/disabled state when checkbox is clicked
// ----------------------------------------------------------------------------
{
    if (col)
        return;
    Q_ASSERT(row <= modules.count());
    QTableWidget *table = (QTableWidget *)sender();
    bool enabled = false;
    if (table->item(row, col)->checkState() == Qt::Checked)
        enabled = true;
    ModuleManager::ModuleInfo m = modules[row];
    mmgr->setEnabled(m.id, enabled);
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
    int row = 0;
    foreach (ModuleManager::ModuleInfo m, modules)
    {
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        Qt::CheckState state = m.enabled ? Qt::Checked : Qt::Unchecked;
        item->setCheckState(state);
        table->setItem(row, 0, item);

        if (m.icon == "")
            m.icon = ":/images/modules.png";
        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setIcon(QIcon(m.icon));
        item->setFlags(Qt::ItemIsEnabled);
        table->setItem(row, 1, item);

        item = new QTableWidgetItem(m.name);
        item->setFlags(Qt::NoItemFlags);
        table->setItem(row, 2, item);

        item = new QTableWidgetItem(m.ver);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(Qt::NoItemFlags);
        table->setItem(row, 3, item);

        if (m.updateAvailable)
        {
            QString msg = QString("Version %1 available").arg(m.latest);
#if 1
            item = new QTableWidgetItem(msg);
            item->setFlags(Qt::NoItemFlags);
            QColor green = QColor(170, 255, 170);
            item->setBackground(QBrush(green));
            table->setItem(row, 4, item);
#else
            // TODO: Use tool buttons to update / enable / disable /
            // show module details
            QWidget *widget = new QWidget;
            QHBoxLayout *layout = new QHBoxLayout;
            layout->addWidget(new QLabel(msg));
            QToolButton *b = new QToolButton;
            b->setText("Update");
            layout->addWidget(b);
            widget->setLayout(layout);
            table->setCellWidget(row, 4, widget);
#endif
        }

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
    QTimer::singleShot(200, this, SLOT(refresh()));
}


void ModulesPage::refresh()
// ----------------------------------------------------------------------------
//   Hide progress bar, reload table
// ----------------------------------------------------------------------------
{
    findUpdatesInProgress = false;
    sw->setCurrentIndex(0);
    updateTable();
}

}
