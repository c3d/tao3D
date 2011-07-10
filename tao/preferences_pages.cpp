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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtGui>
#include "preferences_pages.h"
#include "main.h"
#include "application.h"
#include "module_info_dialog.h"


namespace Tao {

// ============================================================================
//
//   The general page is used to show and modify general settings
//
// ============================================================================

GeneralPage::GeneralPage(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create the page and show general settings
// ----------------------------------------------------------------------------
     : QWidget(parent)
{
    QGroupBox *group = new QGroupBox(tr("General"));
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("User interface language:")), 1, 1);
    combo = new QComboBox;
    grid->addWidget(combo, 1, 2);
    QStringList languages = installedLanguages();
    combo->addItem(tr("(System Language)"));
    foreach (QString lang, languages)
    {
        QString langName;
        if (lang == "en")
            langName = "English";
        else
        if (lang == "fr")
            langName = QString::fromUtf8("Fran\303\247ais");

        if (langName != "")
            combo->addItem(langName,
                           lang);
    }
    QString saved = QSettings().value("uiLanguage", "C").toString();
    if (saved != "C")
    {
        int index = combo->findData(saved);
        if (index != -1)
            combo->setCurrentIndex(index);
    }
    connect(combo, SIGNAL(currentIndexChanged(int)),
            this,  SLOT(setLanguage(int)));

//    grid->addWidget(new QLabel(tr("Disable stereoscopy (3D)")), 2, 1);
//    noStereo = new QCheckBox;
//    grid->addWidget(noStereo, 2, 2);
//    bool disable = QSettings().value("DisableStereoscopy", false).toBool();
//    noStereo->setChecked(disable);
//    connect(noStereo, SIGNAL(toggled(bool)),
//            this,     SLOT(disableStereoBuffers(bool)));

    group->setLayout(grid);

    message = new QLabel;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(group);
    mainLayout->addSpacing(12);
    mainLayout->addStretch(1);
    mainLayout->addWidget(message);
    setLayout(mainLayout);
}


QStringList GeneralPage::installedLanguages()
// ----------------------------------------------------------------------------
//   Return the language for which we have a .qm file
// ----------------------------------------------------------------------------
{
    QStringList ret("en");
    QDir dir = QDir(QCoreApplication::applicationDirPath());
    QStringList files = dir.entryList(QStringList("tao_*.qm"), QDir::Files);
    foreach (QString file, files)
    {
        int from = file.indexOf("_") + 1;
        int to = file.lastIndexOf(".");
        QString lang = file.mid(from, to-from);
        ret << lang;
    }
    return ret;
}


void GeneralPage::setLanguage(int index)
// ----------------------------------------------------------------------------
//   Save (or clear) the preferred language
// ----------------------------------------------------------------------------
{
    message->setText(tr("The language change will take effect after a restart "
                        "of the application."));
    if (index == 0)
    {
        QSettings().remove("uiLanguage");
        return;
    }
    QString lang = combo->itemData(index).toString();
    QSettings().setValue("uiLanguage", lang);
}


// ============================================================================
//
//   The debug page is used to show and modify the state of debug traces
//
// ============================================================================

DebugPage::DebugPage(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create the page and show the current state of debug traces
// ----------------------------------------------------------------------------
     : QWidget(parent)
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
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonsWidget);
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

#ifndef CFG_NOMODPREF

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
    if (!mmgr)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout;
        QLabel *msg = new QLabel(tr("Modules are disabled"));
        mainLayout->addWidget(msg);
        setLayout(mainLayout);
        return;
    }
    modules = mmgr->allModules();

    QGroupBox *gb = new QGroupBox(tr("Installed modules"));
    QVBoxLayout *vbLayout = new QVBoxLayout;

    table = new QTableWidget;
    table->setColumnCount(6);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(3, new QTableWidgetItem("Name"));
    table->setHorizontalHeaderItem(4, new QTableWidgetItem("Version"));
    table->setHorizontalHeaderItem(5, new QTableWidgetItem("Status"));
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
    table->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    table->setIconSize(QSize(32, 32));
    connect(table, SIGNAL(cellClicked(int,int)),
            this, SLOT(onCellClicked(int,int)));
    updateTable();
    vbLayout->addWidget(table);
    search = new QLineEdit;
    search->setPlaceholderText(tr("Search"));
    connect(search, SIGNAL(textChanged(QString)), this, SLOT(doSearch()));
    vbLayout->addWidget(search);
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
    sw->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    cfuLayout->addWidget(sw);
    cfuWidget->setLayout(cfuLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gb);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(cfuWidget);
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

    mmgr->setEnabled(+m.id, !m.enabled || m.inError);
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
    qSort(modules);
    table->setRowCount(modules.count());
    int row = 0;
    foreach (ModuleManager::ModuleInfoPrivate m, modules)
    {
        Qt::ItemFlags enFlag = m.enabled ? Qt::ItemIsEnabled : Qt::NoItemFlags;

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        // info.png from:
        // http://www.iconfinder.com/icondetails/12825/32/info_icon
        // Author: Austin The Heller
        // License: Free for commercial use
        // Icon resized to 24x24 and pasted into a 32x32 area
        item->setIcon(QIcon(":/images/info.png"));
        item->setFlags(enFlag);
        table->setItem(row, 0, item);

        QString txt = (m.enabled && !m.inError) ? tr("Disable") : tr("Enable");
        QToolButton *b = new QToolButton;
        QAction *act = new QAction(txt, this);
        act->setData(QVariant(row));
        connect(act, SIGNAL(triggered()), this, SLOT(toggleModule()));
        b->setDefaultAction(act);
        table->setCellWidget(row, 1, b);

        if (m.icon == "")
            m.icon = ":/images/modules.png";
        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setIcon(QIcon(+m.icon));
        item->setFlags(enFlag);
        table->setItem(row, 2, item);

        item = new QTableWidgetItem(+m.name);
        item->setFlags(enFlag);
        table->setItem(row, 3, item);

        QString vstr = QString::number(m.ver);
        if (!vstr.contains("."))
            vstr.append(".0");
        item = new QTableWidgetItem(vstr);
        item->setTextAlignment(Qt::AlignCenter);
        item->setFlags(enFlag);
        table->setItem(row, 4, item);

        QWidget *widget = NULL;
        if (m.updateAvailable)
        {
            QString msg = QString("Update to %1").arg(+m.latest);
            QToolButton *b = new QToolButton;
            QAction *act = new QAction(msg, this);
            act->setData(QVariant(row));
            connect(act, SIGNAL(triggered()), this, SLOT(updateOne()));
            b->setDefaultAction(act);
            widget = b;
        }
        table->setCellWidget(row, 5, widget);

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
    UpdateModule *up = new UpdateModule(*mmgr, +m.id);
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


void ModulesPage::onCellClicked(int row, int col)
// ----------------------------------------------------------------------------
//   Show info box when info icon is clicked
// ----------------------------------------------------------------------------
{
    if (col)
        return;
    ModuleManager::ModuleInfoPrivate m = modules[row];
    ModuleInfoDialog dialog(m, this);
    dialog.resize(500, 400);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.exec();
}


void ModulesPage::doSearch()
// ----------------------------------------------------------------------------
//   Restrict module list to those that match the current search string
// ----------------------------------------------------------------------------
{
    QString searchString = search->text();
    for (int row = 0; row < table->rowCount(); row++)
    {
        ModuleManager::ModuleInfoPrivate m = modules[row];
        bool hide = !searchString.isEmpty() && !m.contains(searchString);
        table->setRowHidden(row, hide);
    }
}

#endif // !CFG_NOMODPREF

}
