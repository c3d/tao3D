#ifndef PREFERENCES_PAGES_H
#define PREFERENCES_PAGES_H
// *****************************************************************************
// preferences_pages.h                                             Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2012, Soulisse Baptiste <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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
class QRadioButton;
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

    static bool         checkForUpdateOnStartup();
    static bool         webUISecurityTokenEnabled();
    static QString      taoUriScheme();

    static bool         gitEnabled();

#ifndef CFG_NO_WEBUI
signals:
    void                webUISecurityChanged();
#endif

#ifndef CFG_NOGIT
signals:
    void                gitEnabledChanged(bool on);
#endif

protected:
    static bool         checkForUpdateOnStartupDefault();
#ifndef CFG_NO_WEBUI
    static bool         webUISecurityTokenEnabledDefault();
#endif
    static QString      taoUriSchemeDefault();

private:
    QStringList         installedLanguages();

private slots:
    void                setLanguage(int index);
    void                setCheckForUpdateOnStartup(bool on);
#ifndef CFG_NO_WEBUI
    void                setWebUISecurityTokenEnabled(bool on);
#endif
#ifndef CFG_NOGIT
    void                setGitEnabled(bool on);
#endif
    void                setTaoUriScheme(int index);

private:
    QComboBox *         languageCombo;
    QLabel *            message;
    QComboBox *         uriCombo;
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

public:
    static bool    perPixelLighting();
    static bool    VSync();
    static bool    texture2DCompress();
    static bool    texture2DMipmap();
    static int     texture2DMinFilter();
    static int     texture2DMagFilter();
    static quint64 textureCacheMaxMem();
    static quint64 textureCacheMaxGLMem();

protected slots:
    void           setPerPixelLighting(bool on);
    void           setVSync(bool on);
    void           setTexture2DMipmap(bool on);
    void           setTexture2DCompress(bool on);
    void           setTexture2DMinFilter(int value);
    void           texture2DMinFilterChanged(int index);
    void           setTexture2DMagFilter(int value);
    void           texture2DMagFilterChanged(int index);
    void           setTextureCacheMaxMem(quint64 bytes);
    void           textureCacheMaxMemChanged(int index);
    void           setTextureCacheMaxGLMem(quint64 bytes);
    void           textureCacheMaxGLMemChanged(int index);

protected:
    static void    readAllSettings();

    static bool    perPixelLightingDefault();
    static bool    VSyncDefault();
    static bool    texture2DMipmapDefault();
    static bool    texture2DCompressDefault();
    static int     texture2DMinFilterDefault();
    static int     texture2DMagFilterDefault();
    static quint64 textureCacheMaxMemDefault();
    static quint64 textureCacheMaxGLMemDefault();

protected:
    QRadioButton * lightFixed;
    QRadioButton * lightVShader;
    QRadioButton * lightFShader;
    QComboBox    * magCombo, * minCombo, * cacheMemCombo, * cacheGLMemCombo;

protected:
    static bool    dirty;

                   // Cached values
    static bool    perPixelLighting_;
    static bool    VSync_;
    static bool    texture2DCompress_;
    static bool    texture2DMipmap_;
    static int     texture2DMinFilter_;
    static int     texture2DMagFilter_;
    static quint64 textureCacheMaxMem_;
    static quint64 textureCacheMaxGLMem_;
};

}

#endif
