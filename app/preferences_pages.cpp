// *****************************************************************************
// preferences_pages.cpp                                           Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "preferences_pages.h"
#include "main.h"
#include "application.h"
#include "module_info_dialog.h"
#include "tao_gl.h"
#include "opengl_state.h"
#include "texture_cache.h"
#ifndef CFG_NO_WEBUI
#include "window.h"
#endif
#include <QtGui>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QStackedWidget>
#include <QProgressBar>
#include <QAction>
#include <QToolButton>


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
    languageCombo = new QComboBox;
    grid->addWidget(languageCombo, 1, 2);
    QStringList languages = installedLanguages();
    languageCombo->addItem(tr("(System Language)"));
    foreach (QString lang, languages)
    {
        QString langName;
        if (lang == "en")
            langName = "English";
        else
        if (lang == "fr")
            langName = QString::fromUtf8("Fran\303\247ais");

        if (langName != "")
            languageCombo->addItem(langName,
                           lang);
    }
    QString saved = QSettings().value("uiLanguage", "C").toString();
    if (saved != "C")
    {
        int index = languageCombo->findData(saved);
        if (index != -1)
            languageCombo->setCurrentIndex(index);
    }
    connect(languageCombo, SIGNAL(currentIndexChanged(int)),
            this,  SLOT(setLanguage(int)));

    // Menu for automatic check for update
    QCheckBox *cfu = new QCheckBox(tr("Check for update at the launch of the application"));
    cfu->setChecked(checkForUpdateOnStartup());
    connect(cfu, SIGNAL(toggled(bool)),
            this, SLOT(setCheckForUpdateOnStartup(bool)));
    grid->addWidget(cfu, 2, 1);

    // Option to select how to resolve tao: URIs
    grid->addWidget(new QLabel(tr("Connect to tao:// addresses using:")), 3, 1);
    uriCombo = new QComboBox;
    grid->addWidget(uriCombo, 3, 2);
    uriCombo->addItem(tr("git:// on port 9418 (default)"), QVariant("git"));
    uriCombo->addItem(tr("http://"), QVariant("http"));
    saved = taoUriScheme();
    int index = uriCombo->findData(saved);
    if (index != -1)
        uriCombo->setCurrentIndex(index);
    connect(uriCombo, SIGNAL(currentIndexChanged(int)),
            this,     SLOT(setTaoUriScheme(int)));
    QLabel *fw = new QLabel("<span style=\"font-size:small; \">"
                            "&nbsp;&nbsp;&nbsp;&nbsp;" +
                            tr("If you are behind a corporate firewall, or "
                               "have problems with tao:// links, select "
                               "http:// in the above list.") +
                            "</span>");
    grid->addWidget(fw, 4, 1, 1, 2);

#ifndef CFG_NO_WEBUI
    // WebUI authentication
    QCheckBox *secure = new QCheckBox(tr("Reject unauthenticated connections "
                                         "to document editor"));
    secure->setChecked(webUISecurityTokenEnabled());
    connect(secure, SIGNAL(toggled(bool)),
            this, SLOT(setWebUISecurityTokenEnabled(bool)));
    // Changing the setting will restart the WebUI server (if running)
    connect(this, SIGNAL(webUISecurityChanged()),
            &TaoApp->window()->webui, SLOT(securitySettingChanged()));
    grid->addWidget(secure, 5, 1);
#endif

#ifndef CFG_NOGIT
    // Activating all the Git stuff
    QCheckBox *useGit = new QCheckBox(tr("Activate version control features"));
    useGit->setChecked(gitEnabled());
    connect(useGit, SIGNAL(toggled(bool)),
            this, SLOT(setGitEnabled(bool)));
    connect(this, SIGNAL(gitEnabledChanged(bool)),
            TaoApp->window(), SLOT(showShareMenu(bool)));
    grid->addWidget(useGit, 6, 1);
#endif

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
    QString lang = languageCombo->itemData(index).toString();
    QSettings().setValue("uiLanguage", lang);
}


void GeneralPage::setCheckForUpdateOnStartup(bool on)
// ----------------------------------------------------------------------------
//   Save setting about check for update
// ----------------------------------------------------------------------------
{
   QSettings settings;
   settings.setValue("CheckForUpdate", QVariant(on));
}


bool GeneralPage::checkForUpdateOnStartup()
// ----------------------------------------------------------------------------
//   Read setting about check for update
// ----------------------------------------------------------------------------
{
    bool dflt = checkForUpdateOnStartupDefault();
    return QSettings().value("CheckForUpdate", QVariant(dflt)).toBool();
}


bool GeneralPage::checkForUpdateOnStartupDefault()
// ----------------------------------------------------------------------------
//   Default value for "check for update on startup"
// ----------------------------------------------------------------------------
{
#ifdef CFG_WITH_UPDATES
    return true;
#else
    return false;
#endif
}


#ifndef CFG_NO_WEBUI
void GeneralPage::setWebUISecurityTokenEnabled(bool on)
// ----------------------------------------------------------------------------
//   Save setting about security token for WebUI
// ----------------------------------------------------------------------------
{
   QSettings settings;
   settings.setValue("WebUISecurity", QVariant(on));
   emit webUISecurityChanged();
}


bool GeneralPage::webUISecurityTokenEnabled()
// ----------------------------------------------------------------------------
//   Read setting about security token for WebUI
// ----------------------------------------------------------------------------
{
    bool dflt = webUISecurityTokenEnabledDefault();
    return QSettings().value("WebUISecurity", QVariant(dflt)).toBool();
}


bool GeneralPage::webUISecurityTokenEnabledDefault()
// ----------------------------------------------------------------------------
//   Default value for "security token for WebUI"
// ----------------------------------------------------------------------------
{
    return true;
}
#endif // CFG_NO_WEBUI


#ifndef CFG_NOGIT
void GeneralPage::setGitEnabled(bool on)
// ----------------------------------------------------------------------------
//   Save setting about git
// ----------------------------------------------------------------------------
{
   QSettings settings;
   settings.setValue("GitEnabled", QVariant(on));
   emit gitEnabledChanged(on);
}


bool GeneralPage::gitEnabled()
// ----------------------------------------------------------------------------
//   Read setting about security token for WebUI
// ----------------------------------------------------------------------------
{
    bool dflt = false;
    return QSettings().value("GitEnabled", QVariant(dflt)).toBool();
}
#else
bool GeneralPage::gitEnabled()
// ----------------------------------------------------------------------------
//   If git features are not built, just disable
// ----------------------------------------------------------------------------
{
    return false;
}
#endif // CFG_NOGIT


void GeneralPage::setTaoUriScheme(int index)
// ----------------------------------------------------------------------------
//   Save (or clear) the scheme that tao: should translate to
// ----------------------------------------------------------------------------
{
    QString as = uriCombo->itemData(index).toString();
    if (as == taoUriSchemeDefault())
    {
        QSettings().remove("taoUriScheme");
        return;
    }
    QSettings().setValue("taoUriScheme", as);
}


QString GeneralPage::taoUriScheme()
// ----------------------------------------------------------------------------
//   Read setting for tao: scheme translation
// ----------------------------------------------------------------------------
{
    QString dflt = taoUriSchemeDefault();
    return QSettings().value("taoUriScheme", QVariant(dflt)).toString();
}


QString GeneralPage::taoUriSchemeDefault()
// ----------------------------------------------------------------------------
//   Default value for the Uri scheme that tao: should translate to
// ----------------------------------------------------------------------------
{
    return QString("git");
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
    save = new QPushButton(tr("Save for next run"));
    save->setEnabled(false);
    connect(save, SIGNAL(clicked()), this, SLOT(saveClicked()));
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

    save->setEnabled(true);
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


void DebugPage::saveClicked()
// ----------------------------------------------------------------------------
//   Save trace settings
// ----------------------------------------------------------------------------
{
    TaoApp->saveDebugTraceSettings();
    save->setEnabled(false);
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
    table->setColumnCount(7);
    table->setHorizontalHeaderItem(0, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(1, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(2, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Name")));
    table->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Version")));
    table->setHorizontalHeaderItem(6, new QTableWidgetItem(""));
    table->setHorizontalHeaderItem(5, new QTableWidgetItem(tr("Status")));
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
#ifdef CFG_NOMODULEUPDATE
    QFrame *findUpdates = new QFrame;
#else
    QPushButton *findUpdates = new QPushButton(tr("Check for updates"));
    connect(findUpdates, SIGNAL(clicked()), this, SLOT(findUpdates()));
    if (!modules.count())
        findUpdates->setEnabled(false);
#endif
    cfuLayout->addWidget(findUpdates);
    cfuLayout->addSpacing(12);
    sw = new QStackedWidget;
    sw->insertWidget(0, (lb = new QLabel));
    sw->insertWidget(1, (pb = new QProgressBar));
    sw->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    cfuLayout->addWidget(sw);
    cfuWidget->setLayout(cfuLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(gb);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(cfuWidget);
    setLayout(mainLayout);

    connect(mmgr, SIGNAL(modulesChanged()), this, SLOT(updateTable()));
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
    connect(cafu, SIGNAL(complete(bool)), this, SLOT(onCFUComplete(bool)));
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

        item = new QTableWidgetItem;
        item->setTextAlignment(Qt::AlignCenter);
        item->setIcon(QIcon(":/images/general.png"));
        Qt::ItemFlags pFlag = Qt::NoItemFlags;
        if (m.enabled && m.show_preferences)
            pFlag = Qt::ItemIsEnabled;
        item->setFlags(pFlag);
        table->setItem(row, 1, item);

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

        QString txt = (m.enabled && !m.inError) ? tr("Enabled") : tr("Disabled");
        QToolButton *b = new QToolButton;
        QAction *act = new QAction(txt, this);
        act->setData(QVariant(row));
        connect(act, SIGNAL(triggered()), this, SLOT(toggleModule()));
        b->setDefaultAction(act);
        table->setCellWidget(row, 5, b);

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
        table->setCellWidget(row, 6, widget);

        row++;
    }
    for (int i = 0; i < table->columnCount() - 1; i++)
        table->resizeColumnToContents(i);
    table->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}


void ModulesPage::onCFUComplete(bool updatesAvailable)
// ----------------------------------------------------------------------------
//   Leave time for progress bar to show 100%
// ----------------------------------------------------------------------------
{
    if (updatesAvailable)
        lb->setText(tr("Updates are available."));
    else
        lb->setText(tr("All modules are up-to-date."));
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
    connect(up, SIGNAL(complete(bool)), this, SLOT(onUpdateOneComplete(bool)));
    up->start();
}


void ModulesPage::onUpdateOneComplete(bool success)
// ----------------------------------------------------------------------------
//   Delete module updater object, refresh module list
// ----------------------------------------------------------------------------
{
    UpdateModule *up = (UpdateModule *)sender();
    up->deleteLater();
    updateTable();
    if (success)
        lb->setText(tr("Updates are ready to install on restart."));
}


void ModulesPage::onCellClicked(int row, int col)
// ----------------------------------------------------------------------------
//   Show info box when info icon is clicked
// ----------------------------------------------------------------------------
{
    ModuleManager::ModuleInfoPrivate m = modules[row];
    if (col == 0)
    {
        // Info icon clicked
        ModuleInfoDialog dialog(m, this);
        dialog.resize(500, 400);
        dialog.setWindowModality(Qt::WindowModal);
        dialog.exec();
    }
    else if (col == 1)
    {
        // Configuration icon clicked
        if (m.show_preferences)
            m.show_preferences();
    }
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


// ============================================================================
//
//   The performances page shows GL info and allows tweaking some parameters
//
// ============================================================================

// QSettings group name to read/write performance-related preferences
#define PERFORMANCES_GROUP "Performances"

// Cache
bool    PerformancesPage::dirty = true;  // = cache not initialized
// NB: initial values are irrelevant since they are set on first access
bool    PerformancesPage::perPixelLighting_ = false;
bool    PerformancesPage::VSync_ = false;
bool    PerformancesPage::texture2DCompress_ = false;
bool    PerformancesPage::texture2DMipmap_ = false;
int     PerformancesPage::texture2DMinFilter_ = 0;
int     PerformancesPage::texture2DMagFilter_ = 0;
quint64 PerformancesPage::textureCacheMaxMem_ = 0ULL;
quint64 PerformancesPage::textureCacheMaxGLMem_ = 0ULL;


PerformancesPage::PerformancesPage(QWidget *parent)
     : QWidget(parent)
// ----------------------------------------------------------------------------
//   Create the page
// ----------------------------------------------------------------------------
{
    QGroupBox *info = new QGroupBox(trUtf8("OpenGL\302\256 information"));
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Vendor:")), 1, 1);
    grid->addWidget(new QLabel(+GL.Vendor()), 1, 2);
    grid->addWidget(new QLabel(tr("Renderer:")), 2, 1);
    grid->addWidget(new QLabel(+GL.Renderer()), 2, 2);
    grid->addWidget(new QLabel(tr("Version:")), 3, 1);
    grid->addWidget(new QLabel(+GL.Version()), 3, 2);
    info->setLayout(grid);

    QGroupBox *settings = new QGroupBox(trUtf8("OpenGL\302\256 settings"));
    QGridLayout *settingsLayout = new QGridLayout;
    QCheckBox *pps = new QCheckBox(tr("Enable per-pixel lighting"));
    pps->setChecked(perPixelLighting());
    connect(pps, SIGNAL(toggled(bool)),
            this, SLOT(setPerPixelLighting(bool)));
    settingsLayout->addWidget(pps, 1, 1, 1, 2);
    QCheckBox *vs = new QCheckBox(tr("Enable VSync"));
    vs->setChecked(VSync());
    connect(vs, SIGNAL(toggled(bool)),
            this, SLOT(setVSync(bool)));
    settingsLayout->addWidget(vs, 2, 1, 1, 2);
    QCheckBox *ct = new QCheckBox(tr("Compress 2D textures"));
    ct->setChecked(texture2DCompress());
    connect(ct, SIGNAL(toggled(bool)),
            this, SLOT(setTexture2DCompress(bool)));
    connect(ct, SIGNAL(toggled(bool)),
            TextureCache::instance().data(), SLOT(setCompression(bool)));
    settingsLayout->addWidget(ct, 3, 1, 1, 2);
    QCheckBox *mm = new QCheckBox(tr("Generate mipmaps for 2D textures"));
    mm->setEnabled(TaoApp->hasMipmap);
    mm->setChecked(texture2DMipmap());
    connect(mm, SIGNAL(toggled(bool)),
            this, SLOT(setTexture2DMipmap(bool)));
    connect(mm, SIGNAL(toggled(bool)),
            TextureCache::instance().data(), SLOT(setMipmap(bool)));
    settingsLayout->addWidget(mm, 4, 1, 1, 2);
    settingsLayout->addWidget(new QLabel(tr("2D texture magnification:")), 5, 1);
    magCombo = new QComboBox;
    magCombo->addItem(tr("Pixellated"), QVariant(GL_NEAREST));
    magCombo->addItem(tr("Smooth"), QVariant(GL_LINEAR));
    QVariant magSaved = QVariant(texture2DMagFilter());
    int magIndex = magCombo->findData(magSaved);
    if (magIndex != -1)
        magCombo->setCurrentIndex(magIndex);
    connect(magCombo, SIGNAL(currentIndexChanged(int)),
            this,  SLOT(texture2DMagFilterChanged(int)));
    settingsLayout->addWidget(magCombo, 5, 2);
    settingsLayout->addWidget(new QLabel(tr("2D texture reduction:")), 6, 1);
    minCombo = new QComboBox;
    minCombo->addItem(tr("Pixellated"), QVariant(GL_NEAREST));
    minCombo->addItem(tr("Smooth"), QVariant(GL_LINEAR));
    minCombo->addItem(tr("Resized, pixellated"),
                      QVariant(GL_NEAREST_MIPMAP_NEAREST));
    minCombo->addItem(tr("Resized, smoothed"),
                      QVariant(GL_NEAREST_MIPMAP_LINEAR));
    minCombo->addItem(tr("Interpolated, pixellated"),
                      QVariant(GL_LINEAR_MIPMAP_NEAREST));
    minCombo->addItem(tr("Interpolated, smoothed"),
                      QVariant(GL_LINEAR_MIPMAP_LINEAR));
    QVariant minSaved = QVariant(texture2DMinFilter());
    int minIndex = minCombo->findData(minSaved);
    if (minIndex != -1)
        minCombo->setCurrentIndex(minIndex);
    connect(minCombo, SIGNAL(currentIndexChanged(int)),
            this,  SLOT(texture2DMinFilterChanged(int)));
    settingsLayout->addWidget(minCombo, 6, 2);
    settingsLayout->addWidget(new QLabel(tr("Texture cache max. memory:")), 7, 1);
    cacheMemCombo = new QComboBox;
    cacheMemCombo->addItem(tr("0 (default)"), QVariant(0));
    cacheMemCombo->addItem(tr("Unlimited"), QVariant(CACHE_UNLIMITED));
    QVariant cmemSaved = QVariant(TextureCache::instance()->maxMem());
    int cmemIndex = cacheMemCombo->findData(cmemSaved);
    if (cmemIndex != -1)
        cacheMemCombo->setCurrentIndex(cmemIndex);
    connect(cacheMemCombo, SIGNAL(currentIndexChanged(int)),
            this,  SLOT(textureCacheMaxMemChanged(int)));
    settingsLayout->addWidget(cacheMemCombo, 7, 2);
    settingsLayout->addWidget(new QLabel(tr("Texture cache max. GL memory:")), 8, 1);
    cacheGLMemCombo = new QComboBox;
    cacheGLMemCombo->addItem(tr("64 MiB"), QVariant(64*CACHE_MB));
    cacheGLMemCombo->addItem(tr("Unlimited (default)"),
                             QVariant(CACHE_UNLIMITED));
    QVariant cgmemSaved = QVariant(TextureCache::instance()->maxGLMem());
    int cgmemIndex = cacheGLMemCombo->findData(cgmemSaved);
    if (cgmemIndex != -1)
        cacheGLMemCombo->setCurrentIndex(cgmemIndex);
    connect(cacheGLMemCombo, SIGNAL(currentIndexChanged(int)),
            this,  SLOT(textureCacheMaxGLMemChanged(int)));
    settingsLayout->addWidget(cacheGLMemCombo, 8, 2);

    settings->setLayout(settingsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(info);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(settings);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}


void PerformancesPage::setPerPixelLighting(bool on)
// ----------------------------------------------------------------------------
//   Save setting, update application
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("PerPixelLighting", QVariant(on));

    perPixelLighting_ = TaoApp->useShaderLighting = on;
}


void PerformancesPage::setVSync(bool on)
// ----------------------------------------------------------------------------
//   Save setting, update application
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("VSync", QVariant(on));
    TaoApp->enableVSync(on);
    VSync_ = on;
}


void PerformancesPage::setTexture2DCompress(bool on)
// ----------------------------------------------------------------------------
//   Save setting
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("Texture2DCompress", QVariant(on));
    texture2DCompress_ = on;
}


void PerformancesPage::setTexture2DMipmap(bool on)
// ----------------------------------------------------------------------------
//   Save setting
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("Texture2DMipmap", QVariant(on));
    texture2DMipmap_ = on;
}


void PerformancesPage::setTexture2DMinFilter(int value)
// ----------------------------------------------------------------------------
//   Save setting, update application
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("Texture2DMinFilter", QVariant(value));
    TextureCache::instance()->setMinFilter(value);
    texture2DMinFilter_ = value;
}


void PerformancesPage::texture2DMinFilterChanged(int index)
// ----------------------------------------------------------------------------
//   Set minifying filter from combo box index
// ----------------------------------------------------------------------------
{
    int min = minCombo->itemData(index).toInt();
    setTexture2DMinFilter(min);
}


void PerformancesPage::setTexture2DMagFilter(int value)
// ----------------------------------------------------------------------------
//   Save setting, update application
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("Texture2DMagFilter", QVariant(value));
    TextureCache::instance()->setMagFilter(value);
    texture2DMagFilter_ = value;
}


void PerformancesPage::texture2DMagFilterChanged(int index)
// ----------------------------------------------------------------------------
//   Set magnification filter from combo box index
// ----------------------------------------------------------------------------
{
    int mag = magCombo->itemData(index).toInt();
    setTexture2DMagFilter(mag);
}


void PerformancesPage::setTextureCacheMaxMem(quint64 bytes)
// ----------------------------------------------------------------------------
//   Save setting, update texture cache value
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("TextureCacheMaxMem", QVariant(bytes));
    TextureCache::instance()->setMaxMemSize(bytes);
    textureCacheMaxMem_ = bytes;
}


void PerformancesPage::textureCacheMaxMemChanged(int index)
// ----------------------------------------------------------------------------
//   Set texture cache max memory from combo box index
// ----------------------------------------------------------------------------
{
    quint64 bytes = cacheMemCombo->itemData(index).toULongLong();
    setTextureCacheMaxMem(bytes);
}


void PerformancesPage::setTextureCacheMaxGLMem(quint64 bytes)
// ----------------------------------------------------------------------------
//   Save setting, update texture cache value
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    settings.setValue("TextureCacheMaxGLMem", QVariant(bytes));
    TextureCache::instance()->setMaxGLSize(bytes);
    textureCacheMaxGLMem_ = bytes;
}


void PerformancesPage::textureCacheMaxGLMemChanged(int index)
// ----------------------------------------------------------------------------
//   Set texture cache max GL memory from combo box index
// ----------------------------------------------------------------------------
{
    quint64 bytes = cacheGLMemCombo->itemData(index).toULongLong();
    setTextureCacheMaxGLMem(bytes);
}


void PerformancesPage::readAllSettings()
// ----------------------------------------------------------------------------
//   Read all values from user's settings and cache them
// ----------------------------------------------------------------------------
{
    QSettings s;
    s.beginGroup(PERFORMANCES_GROUP);

#define B(name, dflt)  s.value(name, QVariant(dflt)).toBool()
#define I(name, dflt)  s.value(name, QVariant(dflt)).toInt()
#define Q(name, dflt)  s.value(name, QVariant(dflt)).toULongLong()

    perPixelLighting_   = B("PerPixelLighting",   perPixelLightingDefault());
    VSync_              = B("VSync",              VSyncDefault());
    texture2DCompress_  = B("Texture2DCompress",  texture2DCompressDefault());
    texture2DMipmap_    = B("Texture2DMipmap",    texture2DMipmapDefault());
    texture2DMinFilter_ = I("Texture2DMinFilter", texture2DMinFilterDefault());
    texture2DMagFilter_ = I("Texture2DMagFilter", texture2DMagFilterDefault());
    textureCacheMaxMem_ = Q("TextureCacheMaxMem", textureCacheMaxMemDefault());
    textureCacheMaxGLMem_ =
                          Q("TextureCacheMaxGLMem",
                            textureCacheMaxGLMemDefault());

#undef B
#undef I
#undef Q

    dirty = false;
}


#define RETURN_CACHED(x)  if (dirty) { readAllSettings(); } ; return x;

bool PerformancesPage::perPixelLighting()
// ----------------------------------------------------------------------------
//   Read setting
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(perPixelLighting_);
}


bool PerformancesPage::VSync()
// ----------------------------------------------------------------------------
//   Read setting
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(VSync_);
}


bool PerformancesPage::texture2DCompress()
// ----------------------------------------------------------------------------
//   Use compressed internal format when a texture is loaded from file?
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(texture2DCompress_);
}


bool PerformancesPage::texture2DMipmap()
// ----------------------------------------------------------------------------
//   Create a mipmap when a texture is loaded from file?
// ----------------------------------------------------------------------------
{
    if (!TaoApp->hasMipmap)
        return false;
    RETURN_CACHED(texture2DMipmap_);
}


int PerformancesPage::texture2DMinFilter()
// ----------------------------------------------------------------------------
//   Read setting for 2D texture minifying
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(texture2DMinFilter_);
}


int PerformancesPage::texture2DMagFilter()
// ----------------------------------------------------------------------------
//   Read setting for 2D texture magnifying
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(texture2DMagFilter_);
}


quint64 PerformancesPage::textureCacheMaxMem()
// ----------------------------------------------------------------------------
//   Read setting for texture cache memory size
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(textureCacheMaxMem_);
}


quint64 PerformancesPage::textureCacheMaxGLMem()
// ----------------------------------------------------------------------------
//   Read setting for texture cache GL memory size
// ----------------------------------------------------------------------------
{
    RETURN_CACHED(textureCacheMaxGLMem_);
}


bool PerformancesPage::perPixelLightingDefault()
// ----------------------------------------------------------------------------
//   Should per-pixel lighting be enabled by default?
// ----------------------------------------------------------------------------
{
    return false;
}


bool PerformancesPage::VSyncDefault()
// ----------------------------------------------------------------------------
//   Should VSync be enabled by default?
// ----------------------------------------------------------------------------
{
    return true;
}


bool PerformancesPage::texture2DCompressDefault()
// ----------------------------------------------------------------------------
//   Should texture be stored as GL_COMPRESSED_RGBA by default?
// ----------------------------------------------------------------------------
{
    return false;
}


bool PerformancesPage::texture2DMipmapDefault()
// ----------------------------------------------------------------------------
//   Should texture mipmaps be created by default?
// ----------------------------------------------------------------------------
{
    return true;
}


int PerformancesPage::texture2DMinFilterDefault()
// ----------------------------------------------------------------------------
//   Default minifying filter for 2D textures
// ----------------------------------------------------------------------------
{
    return GL_LINEAR_MIPMAP_LINEAR;
}


int PerformancesPage::texture2DMagFilterDefault()
// ----------------------------------------------------------------------------
//   Default magnifying filter for 2D textures
// ----------------------------------------------------------------------------
{
    return GL_LINEAR;
}


quint64 PerformancesPage::textureCacheMaxMemDefault()
// ----------------------------------------------------------------------------
//   Default value for the max memory size of the texture cache
// ----------------------------------------------------------------------------
{
    return 0;
}


quint64 PerformancesPage::textureCacheMaxGLMemDefault()
// ----------------------------------------------------------------------------
//   Default value for the max GL memory size of the texture cache
// ----------------------------------------------------------------------------
{
    return CACHE_UNLIMITED;
}

}
