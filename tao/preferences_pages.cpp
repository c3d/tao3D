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
#include "tao_gl.h"
#include "texture_cache.h"


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

    // Menu for automatic check for update
    QCheckBox *cfu = new QCheckBox(tr("Check for update at the launch of the application"));
    cfu->setChecked(checkForUpdateOnStartup());
    connect(cfu, SIGNAL(toggled(bool)),
            this, SLOT(setCheckForUpdateOnStartup(bool)));
    grid->addWidget(cfu, 2, 1);

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


void GeneralPage::setCheckForUpdateOnStartup(bool on)
// ----------------------------------------------------------------------------
//   Save setting about check for update
// ----------------------------------------------------------------------------
{
   QSettings settings;
   if (on == checkForUpdateOnStartupDefault())
       settings.remove("CheckForUpdate");
   else
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
#ifdef CFG_WITH_CFU
    return true;
#else
    return false;
#endif
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
    table->setHorizontalHeaderItem(3, new QTableWidgetItem("Name"));
    table->setHorizontalHeaderItem(4, new QTableWidgetItem("Version"));
    table->setHorizontalHeaderItem(6, new QTableWidgetItem(""));
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

PerformancesPage::PerformancesPage(QWidget *parent)
     : QWidget(parent)
// ----------------------------------------------------------------------------
//   Create the page
// ----------------------------------------------------------------------------
{
    QGroupBox *info = new QGroupBox(trUtf8("OpenGL\302\256 information"));
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Vendor:")), 1, 1);
    grid->addWidget(new QLabel(+TaoApp->GLVendor), 1, 2);
    grid->addWidget(new QLabel(tr("Renderer:")), 2, 1);
    grid->addWidget(new QLabel(+TaoApp->GLRenderer), 2, 2);
    grid->addWidget(new QLabel(tr("Version:")), 3, 1);
    grid->addWidget(new QLabel(+TaoApp->GLVersionAvailable), 3, 2);
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
    if (on == perPixelLightingDefault())
        settings.remove("PerPixelLighting");
    else
        settings.setValue("PerPixelLighting", QVariant(on));

    TaoApp->useShaderLighting = on;
}


void PerformancesPage::setVSync(bool on)
// ----------------------------------------------------------------------------
//   Save setting, update application
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    if (on == VSyncDefault())
        settings.remove("VSync");
    else
        settings.setValue("VSync", QVariant(on));
    TaoApp->enableVSync(on);
}


void PerformancesPage::setTexture2DCompress(bool on)
// ----------------------------------------------------------------------------
//   Save setting
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    if (on == texture2DCompressDefault())
        settings.remove("Texture2DCompress");
    else
        settings.setValue("Texture2DCompress", QVariant(on));
}


void PerformancesPage::setTexture2DMipmap(bool on)
// ----------------------------------------------------------------------------
//   Save setting
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    if (on == texture2DMipmapDefault())
        settings.remove("Texture2DMipmap");
    else
        settings.setValue("Texture2DMipmap", QVariant(on));
}


void PerformancesPage::setTexture2DMinFilter(int value)
// ----------------------------------------------------------------------------
//   Save setting, update application
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    if (value == texture2DMinFilterDefault())
        settings.remove("Texture2DMinFilter");
    else
        settings.setValue("Texture2DMinFilter", QVariant(value));
    TextureCache::instance()->setMinFilter(value);
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
    if (value == texture2DMagFilterDefault())
        settings.remove("Texture2DMagFilter");
    else
        settings.setValue("Texture2DMagFilter", QVariant(value));
    TextureCache::instance()->setMagFilter(value);
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
    if (bytes == textureCacheMaxMemDefault())
        settings.remove("TextureCacheMaxMem");
    else
        settings.setValue("TextureCacheMaxMem", QVariant(bytes));
    TextureCache::instance()->setMaxMemSize(bytes);
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
    if (bytes == textureCacheMaxGLMemDefault())
        settings.remove("TextureCacheMaxGLMem");
    else
        settings.setValue("TextureCacheMaxGLMem", QVariant(bytes));
    TextureCache::instance()->setMaxGLSize(bytes);
}


void PerformancesPage::textureCacheMaxGLMemChanged(int index)
// ----------------------------------------------------------------------------
//   Set texture cache max GL memory from combo box index
// ----------------------------------------------------------------------------
{
    quint64 bytes = cacheGLMemCombo->itemData(index).toULongLong();
    setTextureCacheMaxGLMem(bytes);
}


bool PerformancesPage::perPixelLighting()
// ----------------------------------------------------------------------------
//   Read setting
// ----------------------------------------------------------------------------
{
    bool dflt = perPixelLightingDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    bool pps = settings.value("PerPixelLighting", QVariant(dflt)).toBool();
    return pps;
}


bool PerformancesPage::VSync()
// ----------------------------------------------------------------------------
//   Read setting
// ----------------------------------------------------------------------------
{
    bool dflt = VSyncDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    bool enabled = settings.value("VSync", QVariant(dflt)).toBool();
    return enabled;
}


bool PerformancesPage::texture2DCompress()
// ----------------------------------------------------------------------------
//   Use compressed internal format when a texture is loaded from file?
// ----------------------------------------------------------------------------
{
    bool dflt = texture2DCompressDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    bool ret = settings.value("Texture2DCompress", QVariant(dflt)).toBool();
    return ret;
}


bool PerformancesPage::texture2DMipmap()
// ----------------------------------------------------------------------------
//   Create a mipmap when a texture is loaded from file?
// ----------------------------------------------------------------------------
{
    bool dflt = texture2DMipmapDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    bool ret = settings.value("Texture2DMipmap", QVariant(dflt)).toBool();
    return ret;
}


int PerformancesPage::texture2DMinFilter()
// ----------------------------------------------------------------------------
//   Read setting for 2D texture minifying
// ----------------------------------------------------------------------------
{
    int dflt = texture2DMinFilterDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    int ret = settings.value("Texture2DMinFilter", QVariant(dflt)).toInt();
    return ret;
}


int PerformancesPage::texture2DMagFilter()
// ----------------------------------------------------------------------------
//   Read setting for 2D texture magnifying
// ----------------------------------------------------------------------------
{
    int dflt = texture2DMagFilterDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    int ret = settings.value("Texture2DMagFilter", QVariant(dflt)).toInt();
    return ret;
}


quint64 PerformancesPage::textureCacheMaxMem()
// ----------------------------------------------------------------------------
//   Read setting for texture cache memory size
// ----------------------------------------------------------------------------
{
    quint64 dflt = textureCacheMaxMemDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    quint64 ret = settings.value("TextureCacheMaxMem",
                                QVariant(dflt)).toULongLong();
    return ret;
}


quint64 PerformancesPage::textureCacheMaxGLMem()
// ----------------------------------------------------------------------------
//   Read setting for texture cache GL memory size
// ----------------------------------------------------------------------------
{
    quint64 dflt = textureCacheMaxGLMemDefault();
    QSettings settings;
    settings.beginGroup(PERFORMANCES_GROUP);
    quint64 ret = settings.value("TextureCacheMaxGLMem",
                                QVariant(dflt)).toULongLong();
    return ret;
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
