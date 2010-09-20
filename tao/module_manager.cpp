// ****************************************************************************
//  module_manager.cpp                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of the ModuleManager class.
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


#include "module_manager.h"
#include "application.h"
#include "options.h"
#include "tao_utf8.h"
#include "parser.h"
#include "runtime.h"
#include "repository.h"
#include <QSettings>
#include <QMessageBox>

TAO_BEGIN

ModuleManager * ModuleManager::instance = NULL;
ModuleManager::Cleanup ModuleManager::cleanup;


ModuleManager * ModuleManager::moduleManager()
// ----------------------------------------------------------------------------
//   Module manager factory
// ----------------------------------------------------------------------------
{
    if (!instance)
    {
        instance = new ModuleManager;
        if (!instance->init())
            instance = NULL;
    }
    return instance;
}


bool ModuleManager::init()
// ----------------------------------------------------------------------------
//   Initialize module manager, load/check/update user's module configuration
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Initializing\n";

    if (!initPaths())
        return false;

    if (!loadConfig())
        return false;

    if (!checkConfig())
        return false;

    if (!checkNew())
        return false;

    IFTRACE(modules)
    {
        debug() << "Updated list of all installed modules\n";
        foreach (ModuleInfo m, modules)
            debugPrint(m);
    }

    return true;
}


bool ModuleManager::initPaths()
// ----------------------------------------------------------------------------
//   Setup per-user and system-wide module paths
// ----------------------------------------------------------------------------
{
#   define MODULES "/modules"
    u = Application::defaultTaoPreferencesFolderPath() + MODULES;
    s = Application::defaultTaoApplicationFolderPath() + MODULES;
#   undef MODULES

    IFTRACE(modules)
    {
        debug() << "User path:   " << +u << "\n";
        debug() << "System path: " << +s << "\n";
    }
    return true;
}


bool ModuleManager::loadConfig()
// ----------------------------------------------------------------------------
//   Read list of configured modules paths from user settings
// ----------------------------------------------------------------------------
{
    // The module settings are stored as child groups under the Modules main
    // group.
    //
    // Modules/<ID1>/Name = "Some module name"
    // Modules/<ID1>/Path = "/some/path"
    // Modules/<ID1>/Enabled = true
    // Modules/<ID2>/Name = "Other module name"
    // Modules/<ID2>/Path = "/other/path"
    // Modules/<ID2>/Enabled = false

    IFTRACE(modules)
        debug() << "Reading user's module configuration\n";

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    QStringList ids = settings.childGroups();
    foreach(QString id, ids)
    {
        settings.beginGroup(id);

        QString name    = settings.value("Name").toString();
        QString path    = settings.value("Path").toString();
        bool    enabled = settings.value("Enabled").toBool();

        ModuleInfo m(id, path, name, enabled);
        modules.append(m);
        modulesById[id]     = &modules.last();

        IFTRACE(modules)
            debugPrint(m);

        settings.endGroup();
    }
    settings.endGroup();
    IFTRACE(modules)
        debug() << ids.size() << " module(s) configured\n";
    return true;
}


bool ModuleManager::checkConfig()
// ----------------------------------------------------------------------------
//   Check current list of modules against filesystem
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Validating current module configuration\n";

    unsigned total = 0, invalid = 0, removed = 0, disabled = 0;
    foreach (ModuleInfo m, modules)
    {
        total++;
        if (!isValid(m))
        {
            invalid++;
            IFTRACE(modules)
                debug() << "Module '" << +m.id << "' not found on disk "
                           "or invalid\n";
            if (askRemove(m, tr("Module is not found or invalid")))
            {
                if (removeFromConfig(m))
                    removed++;
            }
        }
        else
        {
            if (!m.enabled)
                disabled++;
        }
    }

    IFTRACE(modules)
    {
        QString msg;
        msg = QString("Modules checked - %1/%2(%3)/%4 "
                      "total/invalid(removed)/disabled\n")
              .arg(total).arg(invalid).arg(removed).arg(disabled);
        debug() << +msg;
    }
    return true;
}


bool ModuleManager::removeFromConfig(const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Remove m from the list of configured modules
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Removing module '" << +m.id << "' from configuration\n";

    modules.removeOne(m);
    modulesById.remove(m.id);

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.remove(m.id);
    settings.endGroup();

    IFTRACE(modules)
        debug() << "Module removed\n";

    return true;
}


bool ModuleManager::addToConfig(const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Add m to the list of configured modules
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Adding module '" << +m.id << "' to configuration\n";

    modules.append(m);
    modulesById[m.id] = &modules.last();

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(m.id);
    settings.setValue("Name", m.name);
    settings.setValue("Path", m.path);
    settings.setValue("Enabled", m.enabled);
    settings.endGroup();
    settings.endGroup();

    IFTRACE(modules)
        debug() << "Module added\n";

    return true;
}


void ModuleManager::setEnabled(QString id, bool enabled)
// ----------------------------------------------------------------------------
//   Enable or disable a module
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << (enabled ? "En" : "Dis") << "abling module " << +id << "\n";

    if (modulesById.contains(id))
        modulesById[id]->enabled = enabled;

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(id);
    settings.setValue("Enabled", enabled);
    settings.endGroup();
    settings.endGroup();
}


bool ModuleManager::isValid(const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Check if module is valid wrt. what is installed on the filesystem
// ----------------------------------------------------------------------------
{
    return (readModule(m.path) == m);
}


bool ModuleManager::loadAll(Context *context)
// ----------------------------------------------------------------------------
//   Load all enabled modules for current user
// ----------------------------------------------------------------------------
{
    QList<ModuleInfo> enabled;
    foreach (ModuleInfo m, modules)
        if (m.enabled && !m.loaded)
            enabled.append(m);
    return load(context, enabled);
}


bool ModuleManager::checkNew()
// ----------------------------------------------------------------------------
//   Check for new modules in user and system directories
// ----------------------------------------------------------------------------
{
    if (!checkNew(u))
        return false;
    if (!checkNew(s))
        return false;
    return true;
}


bool ModuleManager::checkNew(QString path)
// ----------------------------------------------------------------------------
//   Check for new modules in path
// ----------------------------------------------------------------------------
{
    bool ok = true;

    QList<ModuleInfo> mods = newModules(path);
    foreach (ModuleInfo m, mods)
    {
        if (askEnable(m, tr("New module")))
            m.enabled = true;
        addToConfig(m);
        IFTRACE(modules)
            debug() << "Module " << (m.enabled ? "en" : "dis") << "abled\n";
    }
    return ok;
}


QList<ModuleManager::ModuleInfo> ModuleManager::newModules(QString path)
// ----------------------------------------------------------------------------
//   Return the modules under path that are not in the user's configuration
// ----------------------------------------------------------------------------
{
    QList<ModuleManager::ModuleInfo> mods;

    IFTRACE(modules)
        debug() << "Checking for new modules in " << +path << "\n";

    QDir dir(path);
    if (dir.isReadable())
    {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString d, subdirs)
        {
            QString moduleDir = QDir(dir.filePath(d)).absolutePath();
            ModuleInfo m = readModule(moduleDir);
            if (m.id != "" && !modules.contains(m))
            {
                IFTRACE(modules)
                {
                    debug() << "Found new module:\n";
                    debugPrint(m);
                }
                mods.append(m);
            }
        }
    }

    IFTRACE(modules)
        debug() << mods.size() << " new module(s) found\n";

    return mods;
}


ModuleManager::ModuleInfo ModuleManager::readModule(QString moduleDir)
// ----------------------------------------------------------------------------
//   Read module directory and return module info. Set m.id == "" on error.
// ----------------------------------------------------------------------------
{
    ModuleInfo m;

    QString xlPath = QDir(moduleDir).filePath("module.xl");
    if (QFileInfo(xlPath).isReadable())
    {
        if (XL::Tree * tree = parse(xlPath))
        {
            QString id =  moduleAttr(tree, "id");
            if (id != "")
            {
                QString name = moduleAttr(tree, "name");
                m = ModuleInfo(id, moduleDir, name, false);
                m.desc = moduleAttr(tree, "description");
                m.ver = gitVersion(moduleDir);
                QString iconPath = QDir(moduleDir).filePath("icon.png");
                if (QFile(iconPath).exists())
                    m.icon = iconPath;

                // Update module information
                if (modulesById.contains(m.id))
                {
                    ModuleInfo *p = modulesById[m.id];
                    p->name = m.name;
                    p->desc = m.desc;
                    p->ver = m.ver;
                    p->icon = m.icon;
                }
            }
        }
    }
    return m;
}


QString ModuleManager::gitVersion(QString moduleDir)
// ----------------------------------------------------------------------------
//   Try to find the version of the module using Git
// ----------------------------------------------------------------------------
{
    repository_ptr repo = RepositoryFactory::repository(moduleDir);
    if (repo && repo->valid())
        return +repo->version();
    return "";
}


XL::Tree * ModuleManager::parse(QString xlFile)
// ----------------------------------------------------------------------------
//   Parse the XL file of a module
// ----------------------------------------------------------------------------
{
    XL::Syntax          syntax (XL::MAIN->syntax);
    XL::Positions      &positions = XL::MAIN->positions;
    XL::Errors          errors;
    XL::Parser          parser((+xlFile).c_str(), syntax,positions,errors);

    return parser.Parse();
}


QString ModuleManager::moduleAttr(XL::Tree *tree, QString attribute)
// ----------------------------------------------------------------------------
//   Look for attribute value in module_description section of tree
// ----------------------------------------------------------------------------
{
    QString val;
    FindAttribute action("module_description", +attribute);
    Tree * v = tree->Do(action);
    if (v)
    {
        Text * t = v->AsText();
        if (t)
            val = +(t->value);
    }
    return val;
}


bool ModuleManager::load(Context *context, const QList<ModuleInfo> &mods)
// ----------------------------------------------------------------------------
//   Load modules, in sequence
// ----------------------------------------------------------------------------
{
    bool ok = true;
    foreach(ModuleInfo m, mods)
        ok &= load(context, m);
    return ok;
}


bool ModuleManager::load(Context *context, const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Load one module
// ----------------------------------------------------------------------------
{
    bool ok = true;

    IFTRACE(modules)
        debug() << "Loading module '" << +m.id << "' from " << +m.path << "\n";

    QString xlPath = QDir(m.path).filePath("module.xl");
    ok = xl_load(context, +xlPath) != NULL;

    if (ok && modulesById.contains(m.id))
        modulesById[m.id]->loaded = true;

    return ok;
}


std::ostream & ModuleManager::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Module Manager] ";
    return std::cerr;
}


void ModuleManager::debugPrint(const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Convenience method to display a ModuleInfo object
// ----------------------------------------------------------------------------
{
    debug() << "  ID:      " << +m.id << "\n";
    debug() << "  Path:    " << +m.path << "\n";
    debug() << "  Name:    " << +m.name << "\n";
    debug() << "  Version: " << +m.ver << "\n";
    debug() << "  Icon:    " << +m.icon << "\n";
    debug() << "  Enabled: " << +m.enabled << "\n";
    debug() << "  Loaded:  " << +m.loaded << "\n";
}

// ============================================================================
//
//   Virtual methods are the module manager's interface with the user
//
// ============================================================================

bool ModuleManager::askRemove(const ModuleInfo &m, QString reason)
// ----------------------------------------------------------------------------
//   Prompt user when we're about to remove a module from the configuration
// ----------------------------------------------------------------------------
{
    QString msg = tr("Do you want to remove the following module from the "
                     "Tao configuration?\n\n"
                     "Name: %1\n"
                     "Location: %2").arg(m.name).arg(m.path);
    if (reason != "")
        msg.append(tr("\n\nReason: %1").arg(reason));

    int ret = QMessageBox::question(NULL, tr("Tao modules"), msg,
                                    QMessageBox::Yes | QMessageBox::No);
    return (ret == QMessageBox::Yes);
}


bool ModuleManager::askEnable(const ModuleInfo &m, QString reason)
// ----------------------------------------------------------------------------
//   Prompt user to know if a module should be enabled or disabled
// ----------------------------------------------------------------------------
{
    QString msg = tr("Do you want to enable following module?\n\n"
                     "Name: %1\n"
                     "Location: %2").arg(m.name).arg(m.path);
    if (reason != "")
        msg.append(tr("\n\nReason: %1").arg(reason));

    int ret = QMessageBox::question(NULL, tr("Tao modules"), msg,
                                    QMessageBox::Yes | QMessageBox::No);
    return (ret == QMessageBox::Yes);
}

TAO_END
