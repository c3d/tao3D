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
#include <QSettings>
#include <QMessageBox>

namespace Tao {

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
        debug() << "Updated module list before load\n";
        foreach (ModuleInfo m, modules)
            debugPrintShort(m);
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
    u = QDir::toNativeSeparators(u);
    s = Application::defaultTaoApplicationFolderPath() + MODULES;
    s = QDir::toNativeSeparators(s);
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

        QString path    = settings.value("Path").toString();
        bool    enabled = settings.value("Enabled").toBool();

        ModuleInfo m(id, path, enabled);
        modules.append(m);
        modulesById[id]     = &modules.last();

        IFTRACE(modules)
            debugPrintShort(m);

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
        ModuleInfo d = readModule(m.path);
        if (d.id != m.id)
        {
            invalid++;
            IFTRACE(modules)
                debug() << "Module '" << m.toText() << "' not found on disk "
                           "or ID mismatch\n";
            if (askRemove(m, tr("Module is not found or invalid")))
            {
                if (removeFromConfig(m))
                    removed++;
            }
        }
        else
        {
            modulesById[m.id]->copyProperties(d);
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
        debug() << "Removing module " << m.toText() << " from configuration\n";

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
        debug() << "Adding module " << m.toText() << " to configuration\n";

    modules.append(m);
    modulesById[m.id] = &modules.last();

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(m.id);
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


bool ModuleManager::loadAll(Context *context)
// ----------------------------------------------------------------------------
//   Load all enabled modules for current user
// ----------------------------------------------------------------------------
{
    QList<ModuleInfo> enabled;
    foreach (ModuleInfo m, modules)
        if (m.enabled && !m.loaded)
            enabled.append(m);
    bool ok = load(context, enabled);
    IFTRACE(modules)
    {
        debug() << "All modules\n";
        foreach (ModuleInfo m, modules)
            debugPrint(m);
    }
    return ok;
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
            if (m.id != "")
            {
                if (!modules.contains(m))
                {
                    IFTRACE(modules)
                    {
                        debug() << "Found new module:\n";
                        debugPrint(m);
                    }
                    mods.append(m);
                }
                else
                {
                    ModuleInfo existing = *modulesById[m.id];
                    if (m.path != existing.path)
                    {
                        IFTRACE(modules)
                        {
                            debug() << "WARNING: Duplicate module "
                                       "will be ignored:\n";
                            debugPrintShort(m);
                        }
                        warnDuplicateModule(m);
                    }
                }
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
                m = ModuleInfo(id, moduleDir);
                m.name = name;
                m.desc = moduleAttr(tree, "description");
                m.ver = gitVersion(moduleDir);
                QString iconPath = QDir(moduleDir).filePath("icon.png");
                if (QFile(iconPath).exists())
                    m.icon = iconPath;
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
        debug() << "Loading module " << m.toText() << "\n";

    ok = loadNative(context, m);
    if (ok)
        ok = loadXL(context, m);

    return ok;
}


bool ModuleManager::loadXL(Context *context, const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Load the XL code of a module
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "  Loading XL code (module.xl)\n";

    QString xlPath = QDir(m.path).filePath("module.xl");
    bool ok = XL::xl_load(context, +xlPath) != NULL;

    if (ok && modulesById.contains(m.id))
        modulesById[m.id]->loaded = true;

    return ok;
}


bool ModuleManager::loadNative(Context *context, const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Load the native code of a module (shared libraries under lib/)
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "  Looking for native library, context " << context << "\n";

    ModuleInfo * m_p = NULL;
    if (modulesById.contains(m.id))
        m_p = modulesById[m.id];

    bool ok = false;
#ifdef CONFIG_MINGW
    QString path(m.path + "/lib/module");
#else
    QString path(m.path + "/lib/libmodule");
#endif
    QLibrary * lib = new QLibrary(path, this);
    if (lib->load())
    {
        path = lib->fileName();
        IFTRACE(modules)
            debug() << "    Loaded: " << +path << "\n";
        if (m_p)
            m_p->hasNative = true;
        typedef bool (*enter_symbols)(Context *);
        enter_symbols es = (enter_symbols) lib->resolve("enter_symbols");
        ok = (es != NULL);
        if (ok)
        {
            IFTRACE(modules)
                debug() << "    Calling enter_symbols\n";
            if (m_p)
                m_p->native = lib;
            es(XL::MAIN->context);
        }
        else
        {
            IFTRACE(modules)
                debug() << "    Could not revolve enter_symbols: "
                        << +lib->errorString() << "\n";
        }
    }
    else
    {
        IFTRACE(modules)
            debug() << "    Not found or could not load\n";
    }
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
    debug() << "  ID:         " << +m.id << "\n";
    debug() << "  Path:       " << +m.path << "\n";
    debug() << "  Name:       " << +m.name << "\n";
    debug() << "  Icon:       " << +m.icon << "\n";
    debug() << "  Version:    " << +m.ver << "\n";
    debug() << "  Latest:     " << +m.latest << "\n";
    debug() << "  Up to date: " << !m.updateAvailable << "\n";
    debug() << "  Enabled:    " <<  m.enabled << "\n";
    debug() << "  Loaded:     " <<  m.loaded << "\n";
    if (m.enabled)
    {
        debug() << "  Has native: " <<  m.hasNative << "\n";
        debug() << "  Lib loaded: " << (m.native != NULL) << "\n";
    }
    debug() << "  ------------------------------------------------\n";
}


void ModuleManager::debugPrintShort(const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Display minimal information about a module
// ----------------------------------------------------------------------------
{
    debug() << "  ID:         " << +m.id << "\n";
    debug() << "  Path:       " << +m.path << "\n";
    debug() << "  Enabled:    " <<  m.enabled << "\n";
    debug() << "  ------------------------------------------------\n";
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
#if 0
    QString msg = tr("Do you want to remove the following module from the "
                     "Tao configuration?\n\n"
                     "Name: %1\n"
                     "Location: %2").arg(m.name).arg(m.path);
    if (reason != "")
        msg.append(tr("\n\nReason: %1").arg(reason));

    int ret = QMessageBox::question(NULL, tr("Tao modules"), msg,
                                    QMessageBox::Yes | QMessageBox::No);
    return (ret == QMessageBox::Yes);
#endif
    (void)m; (void)reason;
    return true;
}


bool ModuleManager::askEnable(const ModuleInfo &m, QString reason)
// ----------------------------------------------------------------------------
//   Prompt user to know if a module should be enabled or disabled
// ----------------------------------------------------------------------------
{
#if 0
    QString msg = tr("Do you want to enable following module?\n\n"
                     "Name: %1\n"
                     "Location: %2").arg(m.name).arg(m.path);
    if (reason != "")
        msg.append(tr("\n\nReason: %1").arg(reason));

    int ret = QMessageBox::question(NULL, tr("Tao modules"), msg,
                                    QMessageBox::Yes | QMessageBox::No);
    return (ret == QMessageBox::Yes);
#endif
    (void)m; (void)reason;
    return true;
}


void ModuleManager::warnDuplicateModule(const ModuleInfo &m)
// ----------------------------------------------------------------------------
//   Tell user of conflicting new module (will be ignored)
// ----------------------------------------------------------------------------
{
    (void)m;
}

// ============================================================================
//
//   Checking if a module has new updates
//
// ============================================================================

bool CheckForUpdate::start()
// ----------------------------------------------------------------------------
//   Initiate "check for update" process for a module
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Start checking for updates, module "
                << m.toText() << "\n";

    bool inProgress = false;
    repo = RepositoryFactory::repository(m.path);
    if (repo && repo->valid())
    {
        QStringList tags = repo->tags();
        if (!tags.isEmpty())
        {
            if (tags.contains(m.ver))
            {
                proc = repo->asyncGetRemoteTags("origin");
                connect(repo.data(),
                        SIGNAL(asyncGetRemoteTagsComplete(QStringList)),
                        this,
                        SLOT(processRemoteTags(QStringList)));
                repo->dispatch(proc);
                inProgress = true;
            }
            else
            {
                IFTRACE(modules)
                    debug() << "N/A (current module version not tagged)\n";
            }
        }
        else
        {
            IFTRACE(modules)
                    debug() << "N/A (no local tags)\n";
        }
    }
    else
    {
        IFTRACE(modules)
            debug() << "N/A (not a Git repository)\n";
    }

    if (!inProgress)
        emit complete(m, false);

    return true;
}


void CheckForUpdate::processRemoteTags(QStringList tags)
// ----------------------------------------------------------------------------
//   Process the list of remote tags for module in currentModuleDir
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Module " << m.toText() << "\n";

    bool hasUpdate = false;
    if (!tags.isEmpty())
    {
        QString current = m.ver;
        QString latest = tags[0];
        foreach (QString tag, tags)
            if (Repository::versionGreaterOrEqual(tag, latest))
                latest = tag;

        mm.modulesById[m.id]->latest = latest;

        hasUpdate = (latest != current &&
                     Repository::versionGreaterOrEqual(latest, current));

        IFTRACE(modules)
        {
            debug() << "  Remote tags: " << +tags.join(" ")
                    << " latest " << +latest << "\n";
            debug() << "  Current " << +current << "\n";
            debug() << "  Needs update: " << (hasUpdate ? "yes" : "no") << "\n";
        }

    }
    else
    {
        IFTRACE(modules)
            debug() << "  No remote tags\n";
    }

    mm.modulesById[m.id]->updateAvailable = hasUpdate;
    emit complete(m, hasUpdate);
    deleteLater();
}

// ============================================================================
//
//   Checking if any module has an update
//
// ============================================================================

bool CheckAllForUpdate::start()
// ----------------------------------------------------------------------------
//   Start "check for updates" for all modules
// ----------------------------------------------------------------------------
{
    // Signal checkForUpdateComplete(bool need) will be emitted
    bool ok = true;

    QList<ModuleManager::ModuleInfo> &modules = mm.modules;
    num = modules.count();
    if (!num)
        return false;

    emit minimum(0);
    emit maximum(num);

    foreach (ModuleManager::ModuleInfo m, modules)
        pending << m.id;

    foreach (ModuleManager::ModuleInfo m, modules)
    {
        CheckForUpdate *cfu = new CheckForUpdate(mm, m.id);
        connect(cfu, SIGNAL(complete(ModuleManager::ModuleInfo,bool)),
                this,  SLOT(processResult(ModuleManager::ModuleInfo,bool)));
        if (!cfu->start())
            ok = false;
    }
    return ok;
}


void CheckAllForUpdate::processResult(ModuleManager::ModuleInfo m,
                                      bool updateAvailable)
// ----------------------------------------------------------------------------
//   Remove module from pending list and emit result on last module
// ----------------------------------------------------------------------------
{
    if (updateAvailable)
        this->updateAvailable = true;
    pending.remove(m.id);
    emit progress(num - pending.count());
    if (pending.isEmpty())
    {
        emit complete(this->updateAvailable);
        deleteLater();
    }
}

// ============================================================================
//
//   Updating one module to the latest version
//
// ============================================================================

bool UpdateModule::start()
// ----------------------------------------------------------------------------
//   Start update (download and install) of one module
// ----------------------------------------------------------------------------
{
    bool ok = false;

    if (m.path != "")
    {
        repo = RepositoryFactory::repository(m.path);
        if (repo && repo->valid())
        {
            proc = repo->asyncFetch("origin");
            connect(proc.data(), SIGNAL(finished(int,QProcess::ExitStatus)),
                    this,        SLOT(onFinished(int,QProcess::ExitStatus)));
            connect(proc.data(), SIGNAL(percentComplete(int)),
                    this,        SIGNAL(progress(int)));
            repo->dispatch(proc);
            ok = true;
        }
    }

    return ok;
}


void UpdateModule::onFinished(int exitCode, QProcess::ExitStatus status)
// ----------------------------------------------------------------------------
//   Checkout latest tag and emit complete signal
// ----------------------------------------------------------------------------
{
    bool ok = (status ==  QProcess::NormalExit && exitCode == 0);
    if (ok)
    {
        repo->checkout(+m.latest);
        QString ver = mm.gitVersion(m.path);
        ok = (ver == m.latest);
        if (ok)
        {
            ModuleManager::ModuleInfo *p = mm.modulesById[m.id];
            p->ver = ver;
            p->updateAvailable = false;
        }
    }
    emit complete(ok);
}

}
