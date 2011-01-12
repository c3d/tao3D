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
#include "tao/module_api.h"
#include "application.h"
#include "options.h"
#include "tao_utf8.h"
#include "parser.h"
#include "runtime.h"
#include "repository.h"
#include <QSettings>

#include <unistd.h>    // For chdir()
#include <sys/param.h> // For MAXPATHLEN


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
        {
            delete instance;
            instance = NULL;
        }
    }
    return instance;
}


XL::Tree_p ModuleManager::import(XL::Context_p context, XL::Tree_p self,
                                 XL::Tree_p what)
// ----------------------------------------------------------------------------
//   The import primitive
// ----------------------------------------------------------------------------
{
    // import "filename"
    XL::Text *file = what->AsText();
    if (file)
        return XL::xl_import(context->stack, file->value);

    // Other import syntax: explicit module import
    ModuleManager *mmgr = moduleManager();
    if (mmgr)
        return mmgr->importModule(context, self, what);

    return XL::xl_false;
}


XL::Tree_p ModuleManager::importModule(XL::Context_p context, XL::Tree_p self,
                                       XL::Tree_p what)
// ----------------------------------------------------------------------------
//   The primitive to import a module, for example:   import ModuleName "1.10"
// ----------------------------------------------------------------------------
{
    XL::Tree *err = NULL;
    XL::Name *name = NULL;
    XL::Text *ver = NULL;
    text m_n, m_v;

    XL::Prefix *prefix = what->AsPrefix();
    if (prefix)
    {
        name = prefix->left->AsName();
        if (name)
            m_n = name->value;
        ver = prefix->right->AsText();
        if (ver)
            m_v = ver->value;
    }

    if (m_n != "" && m_v != "")
    {
        bool found = false, name_found = false, version_found = false;
        bool enabled_found = false;
        text inst_v;
        foreach (ModuleInfoPrivate m, modules)
        {
            if (m_n == m.importName)
            {
                name_found = true;
                inst_v = m.ver;
                if (Repository::versionMatches(+m.ver, +m_v))
                {
                    version_found = true;
                    if (!m.enabled)
                        continue;
                    enabled_found = true;
                    if (m.hasNative && !m.native)
                        continue;
                    found = true;
                    QString xlPath = QDir(+m.path).filePath("module.xl");

                    IFTRACE(modules)
                            debug() << "  Importing module " << m_n
                                    << " version " << inst_v << " (requested "
                                    << m_v <<  "): " << +xlPath << "\n";

                    // module_description <indent block> evaluates as nil
                    Name *n = new Name("module_description");
                    Block *b = new Block(new Name("x"), XL::Block::indent,
                                         XL::Block::unindent);
                    Prefix *from = new Prefix(n, b);
                    Name *to = new Name("nil");
                    context->Define(from, to);

                    XL::xl_import(XL::MAIN->context/*context*/, +xlPath);
                }
            }
        }

        if (!found)
        {
            if (name_found)
            {
                if (version_found)
                {
                    if (enabled_found)
                    {
                        err = XL::Ooops("Module $1 load error", name);
                    }
                    else
                    {
                        err = XL::Ooops("Module $1 is disabled", name);
                    }
                }
                else
                {
                    err = XL::Ooops("Installed module $1 version $2 does not "
                                    "match requested version $3", name,
                                    new XL::Text(inst_v, "", ""),
                                    new XL::Text(m_v, "", ""));
                }
            }
            else
            {
                err = XL::Ooops("Module $1 not found", name);
            }
        }
    }
    else
        err = XL::Ooops("Invalid module import: $1", self);

    if (err)
        return context->stack->Evaluate(err);

    return XL::xl_true;
}


bool ModuleManager::init()
// ----------------------------------------------------------------------------
//   Initialize module manager, load/check/update user's module configuration
// ----------------------------------------------------------------------------
{
    if (!enabled())
        return false;

    IFTRACE(modules)
        debug() << "Initializing\n";

    if (!initPaths())
        return false;

    if (!loadConfig())
        return false;

    if (!checkNew())
        return false;

    IFTRACE(modules)
    {
        debug() << "Updated module list before load\n";
        foreach (ModuleInfoPrivate m, modules)
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
//   Read list of configured modules from user settings
// ----------------------------------------------------------------------------
{
    // The module settings are stored as child groups under the Modules main
    // group.
    //
    // Modules/<ID1>/Name = "Some module name"
    // Modules/<ID1>/Enabled = true
    // Modules/<ID2>/Name = "Other module name"
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
        bool    enabled = settings.value("Enabled").toBool();

        ModuleInfoPrivate m(+id, "", enabled);
        m.name = +name;
        modules[id] = m;

        IFTRACE(modules)
            debugPrintShort(m);

        settings.endGroup();
    }
    settings.endGroup();
    IFTRACE(modules)
        debug() << ids.size() << " module(s) configured\n";
    return true;
}


bool ModuleManager::saveConfig()
// ----------------------------------------------------------------------------
//   Save all modules into user's configuration
// ----------------------------------------------------------------------------
{
    bool ok = true;
    foreach (ModuleInfoPrivate m, modules)
        ok &= addToConfig(m);
    return ok;
}


bool ModuleManager::removeFromConfig(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Remove m from the list of configured modules
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Removing module " << m.toText() << " from configuration\n";

    modules.remove(+m.id);

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.remove(+m.id);
    settings.endGroup();

    IFTRACE(modules)
        debug() << "Module removed\n";

    return true;
}


bool ModuleManager::addToConfig(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Add m to the list of configured modules
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "Adding module " << m.toText() << " to configuration\n";

    modules[+m.id] = m;

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(+m.id);
    settings.setValue("Name", +m.name);
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
    ModuleInfoPrivate * m_p = moduleById(+id);
    if (!m_p)
        return;

    IFTRACE(modules)
        debug() << (enabled ? "En" : "Dis") << "abling module "
                << m_p->toText() << "\n";

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(id);
    settings.setValue("Enabled", enabled);
    settings.endGroup();
    settings.endGroup();

    bool prev = m_p->enabled;
    m_p->enabled = enabled;

    if (prev == enabled)
        return;
    if (!m_p->context)
        return;
    if (enabled && !m_p->loaded)
        load(m_p->context, *m_p);
    else
    if (!enabled && m_p->loaded)
        unload(m_p->context, *m_p);
}


bool ModuleManager::loadAll(Context *context)
// ----------------------------------------------------------------------------
//   Load all enabled modules for current user
// ----------------------------------------------------------------------------
{
    QList<ModuleInfoPrivate> toload;
    foreach (ModuleInfoPrivate m, modules)
    {
        modules[+m.id].context = context;
        if (m.enabled && !m.loaded)
            toload.append(m);
    }
    bool ok = load(context, toload);
    IFTRACE(modules)
    {
        debug() << "All modules\n";
        foreach (ModuleInfoPrivate m, modules)
            debugPrint(m);
    }
    return ok;
}

static bool HACK_NoUnload = true;    // See FIXME below

bool ModuleManager::unloadAll(Context *context)
// ----------------------------------------------------------------------------
//   Unload all currently loaded modules
// ----------------------------------------------------------------------------
{
    HACK_NoUnload = false;
    bool ok = true;
    foreach (ModuleInfoPrivate m, modules)
    {
        modules[+m.id].context = NULL;
        if (m.loaded)
            ok &= unload(context, m);
    }
    return ok;
}


QList<ModuleManager::ModuleInfoPrivate> ModuleManager::allModules()
// ----------------------------------------------------------------------------
//   Return a list of all configured modules
// ----------------------------------------------------------------------------
{
    QList<ModuleInfoPrivate> all;
    foreach (const ModuleInfoPrivate m, modules)
        all.append(m);
    return all;
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

    QList<ModuleInfoPrivate> mods = newModules(path);
    foreach (ModuleInfoPrivate m, mods)
    {
        if (askEnable(m, tr("New module")))
            m.enabled = true;
        addToConfig(m);
        IFTRACE(modules)
            debug() << "Module " << (m.enabled ? "en" : "dis") << "abled\n";
    }
    return ok;
}


QList<ModuleManager::ModuleInfoPrivate> ModuleManager::newModules(QString path)
// ----------------------------------------------------------------------------
//   Return the modules under path that are not in the user's configuration
// ----------------------------------------------------------------------------
//   For already configured modules, update path and all properties
{
    QList<ModuleManager::ModuleInfoPrivate> mods;

    IFTRACE(modules)
        debug() << "Checking for modules in " << +path << "\n";

    int known = 0, disabled = 0;
    QDir dir(path);
    if (dir.isReadable())
    {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString d, subdirs)
        {
            QString moduleDir = QDir(dir.filePath(d)).absolutePath();
            IFTRACE(modules)
                debug() << "Checking " << +moduleDir << "\n";
            ModuleInfoPrivate m = readModule(moduleDir);
            if (m.id != "")
            {
                if (!modules.contains(+m.id))
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
                    ModuleInfoPrivate & existing = modules[+m.id];
                    if (existing.path != "")
                    {
                        IFTRACE(modules)
                        {
                            debug() << "WARNING: Duplicate module "
                                       "will be ignored:\n";
                            debugPrintShort(m);
                        }
                        warnDuplicateModule(m);
                    }
                    else
                    {
                        existing.path = m.path;
                        known ++;
                        existing.copyPublicProperties(m);
                        if (!m.enabled)
                            disabled ++;
                    }
                }
            }
        }
    }

    IFTRACE(modules)
        debug() << known << " known modules and "
                << mods.size() << " new module(s) found\n";

    return mods;
}


ModuleManager::ModuleInfoPrivate ModuleManager::readModule(QString moduleDir)
// ----------------------------------------------------------------------------
//   Read module directory and return module info. Set m.id == "" on error.
// ----------------------------------------------------------------------------
{
    ModuleInfoPrivate m;
    QString cause;
    QString xlPath = QDir(moduleDir).filePath("module.xl");
    if (QFileInfo(xlPath).isReadable())
    {
        if (XL::Tree * tree = parse(xlPath))
        {
            // If any mandatory attribute is missing, module is ignored
            QString id =  moduleAttr(tree, "id");
            QString name = moduleAttr(tree, "name");
            QString ver = gitVersion(moduleDir);
            if (ver == "")
                ver = moduleAttr(tree, "version");

            if (id != "" && name != "" && ver != "")
            {
                m = ModuleInfoPrivate(+id, +moduleDir);
                m.name = +name;
                m.ver = +ver;
                m.desc = +moduleAttr(tree, "description");
                QString iconPath = QDir(moduleDir).filePath("icon.png");
                if (QFile(iconPath).exists())
                    m.icon = +iconPath;
                m.author = +moduleAttr(tree, "author");
                m.website = +moduleAttr(tree, "website");
                m.importName = +moduleAttr(tree, "import_name");
            }
            else
            {
                cause = tr("Missing ID, name or version");
            }
        }
        else
        {
            cause = tr("Could not parse module.xl");
        }
    }
    if (m.id == "")
        warnInvalidModule(moduleDir, cause);
    return m;
}


QString ModuleManager::gitVersion(QString moduleDir)
// ----------------------------------------------------------------------------
//   Try to find the version of the module using Git
// ----------------------------------------------------------------------------
{
    RepositoryFactory::Mode mode = RepositoryFactory::OpenExistingHere;
    repository_ptr repo = RepositoryFactory::repository(moduleDir, mode);
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


bool ModuleManager::load(Context *ctx, const QList<ModuleInfoPrivate> &mods)
// ----------------------------------------------------------------------------
//   Load modules, in sequence
// ----------------------------------------------------------------------------
{
    bool ok = true;
    foreach(ModuleInfoPrivate m, mods)
        ok &= load(ctx, m);
    return ok;
}


bool ModuleManager::load(Context *context, const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Load one module
// ----------------------------------------------------------------------------
{
    bool ok = true;

    IFTRACE(modules)
        debug() << "Loading module " << m.toText() << "\n";

    emit loading(QString::fromStdString(m.name));

    ok = loadNative(context, m);
    if (ok)
        ok = loadXL(context, m);

    return ok;
}


bool ModuleManager::loadXL(Context */*context*/, const ModuleInfoPrivate &/*m*/)
// ----------------------------------------------------------------------------
//   Load the XL code of a module
// ----------------------------------------------------------------------------
{
    // Do not import module definitions at startup time but only on explicit
    // import (see importModule)
    // REVISIT: here we probably want to execute only a specific part of
    // module.xl
    return true;

#if 0
    IFTRACE(modules)
        debug() << "  Loading XL source (module.xl)\n";

    QString xlPath = QDir(+m.path).filePath("module.xl");

    // module_description <indent block> evaluates as nil
    // REVISIT: bind a native function and use it to parse the block?
    Name *n = new Name("module_description");
    Block *b = new Block(new Name("x"), XL::Block::indent,
                         XL::Block::unindent);
    Prefix *from = new Prefix(n, b);
    Name *to = new Name("nil");
    context->Define(from, to);

    bool ok = XL::xl_import(context, +xlPath) != NULL;

    ModuleInfoPrivate *m_p = moduleById(m.id);
    if (ok && m_p)
        m_p->loaded = true;

    XL::source_files::iterator it = XL::MAIN->files.find(+xlPath);
    XL::source_files::iterator end = XL::MAIN->files.end();
    if (it != end)
    {
        XL::SourceFile &sf = (*it).second;
        Context *moduleContext = sf.context;

        if (context->Bound(new XL::Name("module_init")))
        {
            IFTRACE(modules)
                debug() << "    Calling XL module_init\n";
            XL::XLCall("module_init")(moduleContext);
        }
    }

    return ok;
#endif
}


struct SetCwd
// ----------------------------------------------------------------------------
//   Temporarily change current directory
// ----------------------------------------------------------------------------
{
    SetCwd(QString path)
    {
        IFTRACE(modules)
        {
            ModuleManager::debug() << "    Changing current directory to: "
                                   << +path << "\n";
        }
        getcwd(wd, sizeof(wd));
        chdir(path.toStdString().c_str());
    }
    ~SetCwd()
    {
        IFTRACE(modules)
            ModuleManager::debug() << "    Restoring current directory: "
                                   << wd << "\n";
        chdir(wd);
    }

    char wd[MAXPATHLEN];
};



bool ModuleManager::loadNative(Context * /*context*/, const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Load the native code of a module (shared libraries under lib/)
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "  Looking for native library\n";

    ModuleInfoPrivate * m_p = moduleById(m.id);
    Q_ASSERT(m_p);
    bool ok;
    QString libdir(+m.path + "/lib");
#if   defined(CONFIG_MINGW)
    QString path(libdir + "/module.dll");
#elif defined(CONFIG_MACOSX)
    QString path(libdir + "/libmodule.dylib");
#elif defined(CONFIG_LINUX)
    QString path(libdir + "/libmodule.so");
#else
#error Unknown OS - please define library name
#endif

    m_p->hasNative = QFile(path).exists();
    if (m_p->hasNative)
    {
        // Change current directory, just the time to load any module dependency
        SetCwd cd(libdir);
        QLibrary * lib = new QLibrary(path, this);
        if (lib->load())
        {
            path = lib->fileName();
            IFTRACE(modules)
                    debug() << "    Loaded: " << +path << "\n";
            ok = isCompatible(lib);
            if (ok)
            {
                enter_symbols_fn es =
                        (enter_symbols_fn) lib->resolve("enter_symbols");
                ok = (es != NULL);
                if (ok)
                {
                    module_init_fn mi =
                            (module_init_fn) lib->resolve("module_init");
                    if ((mi != NULL))
                    {
                        IFTRACE(modules)
                                debug() << "    Calling module_init\n";
                        mi(&api, &m);
                    }

                    IFTRACE(modules)
                            debug() << "    Calling enter_symbols\n";
                    if (m_p)
                        m_p->native = lib;
                    es(XL::MAIN->context);
                }
            }
        }
        else
        {
            IFTRACE(modules)
                debug() << "    Load error: " << +lib->errorString() << "\n";
            delete lib;
        }
    }
    else
    {
        IFTRACE(modules)
            debug() << "    Module has no native library\n";
    }
    return (m_p->hasNative && m_p->native);
}


bool ModuleManager::isCompatible(QLibrary * lib)
// ----------------------------------------------------------------------------
//   Return true if library was built with a compatible version of the Tao API
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "    Checking API compatibility\n";

    unsigned   current   = TAO_MODULE_API_CURRENT;
    unsigned   age       = TAO_MODULE_API_AGE;
    unsigned * m_current = (unsigned *) lib->resolve("module_api_current");
    if (!m_current)
    {
        IFTRACE(modules)
            debug() << "      Error: library has no version information\n";
        return false;
    }

    IFTRACE(modules)
        debug() << "      Module [current " << *m_current << "]  Tao [current "
                << current << " age " << age << "]\n";
    bool ok = ((current - age) <= *m_current && *m_current <= current);
    if (!ok)
    {
        IFTRACE(modules)
            debug() << "      Version mismatch!\n";
        warnBinaryModuleIncompatible(lib);
    }
    return ok;
}


bool ModuleManager::unload(Context *context, const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Unload one module
// ----------------------------------------------------------------------------
{
    // FIXME: unload is normally disabled because there is no way to remove
    // primitives from the XL runtime
    if (HACK_NoUnload)
        return false;

    bool ok = true;

    IFTRACE(modules)
        debug() << "Unloading module: " << m.toText() << "\n";

    ok = unloadXL(context, m);
    if (ok)
        ok = unloadNative(context, m);
    return ok;
}


bool ModuleManager::unloadNative(Context *context, const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Unload the native code of a module
// ----------------------------------------------------------------------------
{
    IFTRACE(modules)
        debug() << "  Unloading native library\n";

    bool ok = true;

    ModuleInfoPrivate * m_p = moduleById(m.id);
    if (!m_p || !m_p->hasNative)
        return ok;

    QLibrary * lib = m_p->native;
    if (!lib)
        return ok;

    Tao::delete_symbols_fn ds;
    ds = (Tao::delete_symbols_fn)lib->resolve("delete_symbols");
    if (ds)
    {
        IFTRACE(modules)
            debug() << "    Calling delete_symbols\n";
        ok &= ds(context);
    }
    Tao::module_exit_fn me;
    me = (Tao::module_exit_fn)lib->resolve("module_exit");
    if (me)
    {
        IFTRACE(modules)
            debug() << "    Calling module_exit\n";
        ok &= me();
    }

    IFTRACE(modules)
        debug() << "    Unloading library\n";
    m_p->native->unload();
    m_p->native->deleteLater();
    m_p->native = NULL;
    return ok;
}


bool ModuleManager::unloadXL(Context *context, const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Unload the XL code of a module
// ----------------------------------------------------------------------------
{
    bool ok = true;
    IFTRACE(modules)
        debug() << "  Unloading XL code (module.xl)\n";

    ModuleInfoPrivate * m_p = moduleById(m.id);
    if (!m_p)
        return ok;

    QString xlPath = +m_p->path + "module.xl";
    XL::source_files::iterator it = XL::MAIN->files.find(+xlPath);
    XL::source_files::iterator end = XL::MAIN->files.end();
    if (it != end)
    {
        XL::SourceFile &sf = (*it).second;
        Context *moduleContext = sf.context;

        if (context->Bound(new XL::Name("module_exit")))
        {
            IFTRACE(modules)
                debug() << "    Calling XL module_exit\n";
            XL::XLCall("module_exit")(moduleContext);
        }
    }

    m_p->loaded = false;
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


void ModuleManager::debugPrint(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Convenience method to display a ModuleInfoPrivate object
// ----------------------------------------------------------------------------
{
    debug() << "  ID:         " <<  m.id << "\n";
    debug() << "  Path:       " <<  m.path << "\n";
    debug() << "  Name:       " <<  m.name << "\n";
    debug() << "  Import:     " <<  m.importName << "\n";
    debug() << "  Author:     " <<  m.author << "\n";
    debug() << "  Website:    " <<  m.website << "\n";
    debug() << "  Icon:       " <<  m.icon << "\n";
    debug() << "  Version:    " <<  m.ver << "\n";
    debug() << "  Latest:     " <<  m.latest << "\n";
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


void ModuleManager::debugPrintShort(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Display minimal information about a module
// ----------------------------------------------------------------------------
{
    debug() << "  ID:         " <<  m.id << "\n";
    debug() << "  Name:       " <<  m.name << "\n";
    debug() << "  Enabled:    " <<  m.enabled << "\n";
    debug() << "  ------------------------------------------------\n";
}


ModuleManager::ModuleInfoPrivate * ModuleManager::moduleById(text id)
// ----------------------------------------------------------------------------
//   Lookup a module info structure in the list of known modules
// ----------------------------------------------------------------------------
{
    if (modules.contains(+id))
        return &modules[+id];
    return NULL;
}

// ============================================================================
//
//   Virtual methods are the module manager's interface with the user
//
// ============================================================================

bool ModuleManager::askRemove(const ModuleInfoPrivate &m, QString reason)
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


bool ModuleManager::askEnable(const ModuleInfoPrivate &m, QString reason)
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


void ModuleManager::warnInvalidModule(QString moduleDir, QString cause)
// ----------------------------------------------------------------------------
//   Tell user of invalid module (will be ignored)
// ----------------------------------------------------------------------------
{
    std::cerr << +tr("WARNING: Skipping invalid module %1\n")
                 .arg(moduleDir);
    if (cause != "")
        std::cerr << +tr("WARNING:   %1\n").arg(cause);
}


void ModuleManager::warnDuplicateModule(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Tell user of conflicting new module (will be ignored)
// ----------------------------------------------------------------------------
{
    (void)m;
}


void ModuleManager::warnBinaryModuleIncompatible(QLibrary *lib)
// ----------------------------------------------------------------------------
//   Tell user about incompatible binary module (will be ignored)
// ----------------------------------------------------------------------------
{
    std::cerr << +QObject::tr("WARNING: Skipping incompatible binary module ")
              << +lib->fileName() << "\n";
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
    repo = RepositoryFactory::repository(+m.path);
    if (repo && repo->valid())
    {
        QStringList tags = repo->tags();
        if (!tags.isEmpty())
        {
            if (tags.contains(+m.ver))
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
        QString current = +m.ver;
        QString latest = tags[0];
        foreach (QString tag, tags)
            if (Repository::versionGreaterOrEqual(tag, latest))
                latest = tag;

        mm.modules[+m.id].latest = +latest;

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

    mm.modules[+m.id].updateAvailable = hasUpdate;
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

    QList<ModuleManager::ModuleInfoPrivate> modules = mm.allModules();
    num = modules.count();
    if (!num)
        return false;

    emit minimum(0);
    emit maximum(num);

    foreach (ModuleManager::ModuleInfoPrivate m, modules)
        pending << +m.id;

    foreach (ModuleManager::ModuleInfoPrivate m, modules)
    {
        CheckForUpdate *cfu = new CheckForUpdate(mm, +m.id);
        connect(cfu, SIGNAL(complete(ModuleManager::ModuleInfoPrivate,bool)),
                this,
                SLOT(processResult(ModuleManager::ModuleInfoPrivate,bool)));
        if (!cfu->start())
            ok = false;
    }
    return ok;
}


void CheckAllForUpdate::processResult(ModuleManager::ModuleInfoPrivate m,
                                      bool updateAvailable)
// ----------------------------------------------------------------------------
//   Remove module from pending list and emit result on last module
// ----------------------------------------------------------------------------
{
    if (updateAvailable)
        this->updateAvailable = true;
    pending.remove(+m.id);
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
        repo = RepositoryFactory::repository(+m.path);
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
        repo->checkout(m.latest);
        QString ver = mm.gitVersion(+m.path);
        ok = (ver == +m.latest);
        if (ok)
        {
            ModuleManager::ModuleInfoPrivate *p = mm.moduleById(m.id);
            p->ver = +ver;
            p->updateAvailable = false;
        }
    }
    emit complete(ok);
}

}
