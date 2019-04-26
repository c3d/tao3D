// *****************************************************************************
// module_manager.cpp                                              Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
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
#include "tao/module_api.h"
#include "application.h"
#include "options.h"
#include "tao_utf8.h"
#include "parser.h"
#include "runtime.h"
#include "repository.h"
#include "widget.h"
#include "runtime.h"
#include "error_message_dialog.h"
#include <QSettings>
#include <QHash>
#include <QMessageBox>

#include <unistd.h>    // For chdir()
#include <sys/param.h> // For MAXPATHLEN
#include <math.h>
#ifdef Q_OS_WIN
#include <windows.h> // For GetEnvironmenVariable(), etc.
#endif


RECORDER(modules,               64, "Tao3D module manager");
RECORDER(modules_warning,       16, "Warning about Tao3D modules");

namespace Tao {

ModuleManager * ModuleManager::instance = NULL;
ModuleManager::Cleanup ModuleManager::cleanup;


ModuleManager * ModuleManager::moduleManager()
// ----------------------------------------------------------------------------
//   Module manager factory
// ----------------------------------------------------------------------------
{
    if (!instance)
        instance = new ModuleManager;
    return instance;
}


Tree_p ModuleManager::import(Scope *scope, Tree *self)
// ----------------------------------------------------------------------------
//   The import primitive
// ----------------------------------------------------------------------------
{
    // Check list of source files before import
    XL::source_files before = XL::MAIN->files;

    // Import throuhg the module manager if we can, otherwise XL runtime
    Tree *result = self;
    if (ModuleManager *modules = moduleManager())
        result = modules->importModule(scope, self);
    else
        result = XL::xl_import(scope, self);

    // Check if new files were imported, if so monitor them
    XL::source_files after = XL::MAIN->files;
    if (after.size() > before.size())
        for (auto &source : after)
            if (before.count(source.first) == 0)
                Widget::Tao()->fileMonitor().addPath(+source.first);

    return XL::xl_false;
}


Tree * ModuleManager::importModule(Scope *scope, Tree *self)
// ----------------------------------------------------------------------------
//   The primitive to import a module, for example:   import ModuleName "1.10"
// ----------------------------------------------------------------------------
{
    Prefix *prefix = self->AsPrefix();
    if (!prefix)
        return self;

    // Check what we import
    Tree        *what    = prefix->right;
    Name        *name    = NULL;
    XL::Version  version = XL::Version(1);
    Tree        *result  = self;

    prefix  = what->AsPrefix();
    if (prefix)
    {
        version = parseVersion(prefix->right);
        name = prefix->left->AsName();
    }
    else
    {
        name = what->AsName();
    }

    Context context(scope);
    if (name)
    {
        text        moduleName   = name->value;
        bool        found        = false;
        bool        nameFound    = false;
        bool        versionFound = false;
        bool        enabledFound = false;
        XL::Version moduleVersion;
        foreach (ModuleInfoPrivate m, modules)
        {
            if (moduleName == m.importName)
            {
                XL::Version moduleVersion = m.version;
                nameFound = true;
                if (m.version.IsCompatibleWith(version))
                {
                    versionFound = true;
                    if (!m.enabled)
                        continue;
                    enabledFound = true;
                    if (m.hasNative)
                    {
                        if (!m.native && !m.inError)
                        {
                            bool ok = loadNative(context, m);
                            if (!ok)
                                break;
                        }
                        if (!m.native)
                            continue;
                    }
                    found = true;
                    QString xlPath = m.xlPath();

                    record(modules,
                           "Module %s version %f (requested %v) path %s",
                           moduleName, moduleVersion, version, xlPath);
                    if (m.loaded)
                    {
                        // Use the cached (already imported) version
                        result = m.loaded;
                    }
                    else
                    {
                        // Enter symbols only the first time
                        enter_symbols_fn enterSymbols = (enter_symbols_fn)
                            m.native->resolve("enter_symbols");

                        if (enterSymbols)
                        {
                            record(modules, "Calling enter_symbols %p for %s",
                                   enterSymbols, moduleName);
                            enterSymbols(context);
                        }

                        result = XL::xl_import(scope, self);
                        m.loaded = result;
                    }
                    break;
                }
            }
        }

        if (!found)
        {
            if (nameFound)
            {
                if (versionFound)
                {
                    if (enabledFound)
                    {
                        XL::Ooops("Module $1 load error", name);
                        return XL::xl_false;
                    }
                    else
                    {
                        XL::Ooops("Module $1 is disabled", name);
                        return XL::xl_false;
                    }
                }
                else
                {
                    XL::Ooops("Installed module $1 version $2 does not "
                              "match requested version $3", name)
                        .Arg(text(moduleVersion)).Arg(text(version));
                    return XL::xl_false;
                }
            }
            else
            {
                XL::Ooops("Module $1 not found", name);
                return XL::xl_false;
            }
        }
    }
    else
    {
        XL::Ooops("Invalid module import $1", self);
    }

    return XL::xl_true;
}


bool ModuleManager::init()
// ----------------------------------------------------------------------------
//   Initialize module manager, load/check/update user's module configuration
// ----------------------------------------------------------------------------
{
    record(modules, "Initializing manager %p", this);

    if (!enabled())
        return false;
    if (!initPaths())
        return false;
    if (!loadConfig())
        return false;
    if (!checkNew())
        return false;
    if (!cleanConfig())
        return false;

    IFTRACE(modules)
    {
        record(modules, "Updated module list before load");
        foreach (const ModuleInfoPrivate &m, modules)
            record(modules,
                   "Module ID %u name %s %+s",
                   m.id, m.name, m.enabled ? "enabled" : "disabled");
    }

    return true;
}


bool ModuleManager::initPaths()
// ----------------------------------------------------------------------------
//   Setup per-user and system-wide module paths
// ----------------------------------------------------------------------------
{
#define MODULES "/modules"
    userDir = Application::defaultTaoPreferencesFolderPath() + MODULES;
    userDir = QDir::toNativeSeparators(userDir);
    systemDir = Application::defaultTaoApplicationFolderPath() + MODULES;
    systemDir = QDir::toNativeSeparators(systemDir);
#undef MODULES

    record(modules, "User %s System %s", userDir, systemDir);
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

    record(modules, "Reading user module configuration");

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

        record(modules, "New module id %s name %s %+s",
               id, name,
               enabled ? "enabled" : "disabled");
        settings.endGroup();
    }
    settings.endGroup();
    record(modules, "%u modules conifgured", ids.size());
    return true;
}


bool ModuleManager::saveConfig()
// ----------------------------------------------------------------------------
//   Save all modules into user's configuration
// ----------------------------------------------------------------------------
{
    record(settings, "Saving user's module configuration, %u modules",
           modules.count());
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
    record(modules,
           "Removing module %s id %s from configuration", m.name, m.id);
    modules.remove(+m.id);

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.remove(+m.id);
    settings.endGroup();
    return true;
}


bool ModuleManager::addToConfig(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Add m to the list of configured modules
// ----------------------------------------------------------------------------
{
    record(modules,
           "Adding module %s id %s to configuration", m.name, m.id);
    modules[+m.id] = m;

    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(+m.id);
    settings.setValue("Name", +m.name);
    settings.setValue("Enabled", m.enabled);
    settings.endGroup();
    settings.endGroup();

    return true;
}


bool ModuleManager::cleanConfig()
// ----------------------------------------------------------------------------
//   Check current module list, remove invalid modules, save config to disk
// ----------------------------------------------------------------------------
{
    foreach (ModuleInfoPrivate m, modules)
    {
        if (m.path == "" || m.id == "0")
        {
            removeFromConfig(m);
            continue;
        }
        ModuleInfoPrivate &minfo = moduleById(m.id);
        minfo.hasNative = QFile(minfo.libPath()).exists();
    }
    return true;
}


void ModuleManager::setEnabled(QString id, bool enabled)
// ----------------------------------------------------------------------------
//   Enable or disable a module
// ----------------------------------------------------------------------------
{
    record(modules, "%+sabling module id %s", enabled ? "En" : "Dis", id);
    XL_ASSERT(modules.contains(id));

    ModuleInfoPrivate &minfo = moduleById(id);
    QSettings settings;
    settings.beginGroup(USER_MODULES_SETTING_GROUP);
    settings.beginGroup(id);
    settings.setValue("Enabled", enabled);
    settings.endGroup();
    settings.endGroup();

    bool prev = minfo.enabled;
    minfo.enabled = enabled;

    if (prev == enabled && !minfo.inError)
        return;
    if (!minfo.scope)
        return;
    if (enabled && !minfo.loaded)
    {
        Context context(minfo.scope);
        load(context, minfo);
    }
}


bool ModuleManager::enabled(QString importName)
// ----------------------------------------------------------------------------
//   Return true if module can be used (exists and is enabled)
// ----------------------------------------------------------------------------
{
    foreach (ModuleInfoPrivate m, modules)
        if (m.enabled && m.importName == +importName)
            return true;
    return false;
}


bool ModuleManager::loadAll(Context &context)
// ----------------------------------------------------------------------------
//   Load all enabled modules for current user
// ----------------------------------------------------------------------------
{
    QList<ModuleInfoPrivate> toload;
    foreach (const ModuleInfoPrivate &m, modules)
    {
        modules[+m.id].scope = context.CreateScope();
        if (m.enabled && !m.loaded && m.path != "")
            toload.append(m);
        context.PopScope();
    }
    bool ok = load(context, toload);
    record(modules, "All %u modules loaded", modules.size());
    foreach (const ModuleInfoPrivate &m, modules)
        record(modules, "Loaded %s id %s %+s",
               m.name, m.id, m.enabled ? "enabled" : "disabled");
    return ok;
}


bool ModuleManager::loadAutoLoadModules(Context &context)
// ----------------------------------------------------------------------------
//   Load and init native code for all modules that are marked auto-load
// ----------------------------------------------------------------------------
{
    bool ok = true;
    Scope *scope = context.CurrentScope();
    foreach (ModuleInfoPrivate m, modules)
    {
        modules[+m.id].scope = scope;
        if (m.enabled && !m.loaded && m.path != "" &&
            (m.importName == "" || m.autoLoad))
            ok &= loadNative(context, m);
    }
    return ok;
}


QStringList ModuleManager::anonymousXL()
// ----------------------------------------------------------------------------
//   Return list of all XL files for modules that have no import_name
// ----------------------------------------------------------------------------
{
    QStringList files;
    foreach (ModuleInfoPrivate m, modules)
    {
        if (m.enabled && m.path != "" && m.importName == "")
            files << m.xlPath();
    }
    return files;
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
    if (!checkNew(userDir))
        return false;
    if (!checkNew(systemDir))
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
        record(modules, "Checking module %s id %s %+s",
               m.name, m.id, m.enabled ? "enabled" : "disabled");
    }
    return ok;
}


QList<ModuleManager::ModuleInfoPrivate> ModuleManager::newModules(QString path)
// ----------------------------------------------------------------------------
//   Return the modules under path that are not in the user's configuration
// ----------------------------------------------------------------------------
//   For already configured modules, update path and all properties
{
    record(modules, "Checking for modules in %s", path);
    QList<ModuleManager::ModuleInfoPrivate> mods;
    int known = 0, disabled = 0;
    QDir dir(path);
    if (dir.isReadable())
    {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString d, subdirs)
        {
            QString moduleDir = QDir(dir.filePath(d)).absolutePath();
            record(modules, "Checking directory %s", +moduleDir);
            ModuleInfoPrivate m = readModule(moduleDir);
            if (m.id != "")
            {
                emit checking(+m.name);

                if (!modules.contains(+m.id))
                {
                    record(modules, "Found new module %s id %s", m.name, m.id);
                    mods.append(m);
                }
                else
                {
                    ModuleInfoPrivate & existing = modules[+m.id];
                    if (existing.path != "")
                    {
                        record(modules_warning,
                               "Duplicate module %s id %s will be ignored",
                               m.name, m.id);
                        warnDuplicateModule(m, existing);
                    }
                    else
                    {
                        existing.path = m.path;
                        known ++;
                        existing.copyPublicProperties(m);
                        existing.qchFiles = m.qchFiles;
                        if (!m.enabled)
                            disabled ++;
                    }
                }
            }
        }
    }

    record(modules, "%u known modules and %u new modules found",
           known, mods.size());

    return mods;
}


void ModuleManager::refreshModuleProperties(QString moduleDir)
// ----------------------------------------------------------------------------
//   Read module directory. If module is known, its refresh entry
// ----------------------------------------------------------------------------
{
    record(modules, "Refreshing module propertis for %s", moduleDir);
    ModuleInfoPrivate m = readModule(moduleDir);
    if (m.id != "" && modules.contains(+m.id))
    {
        ModuleInfoPrivate & existing = modules[+m.id];
        existing.copyPublicProperties(m);
    }
}


ModuleManager::ModuleInfoPrivate ModuleManager::readModule(QString moduleDir)
// ----------------------------------------------------------------------------
//   Read module directory and return module info. Set m.id == "" on error.
// ----------------------------------------------------------------------------
{
    ModuleInfoPrivate m("", +moduleDir);
    QString cause;
    QString xlPath = m.xlPath();
    if (QFileInfo(xlPath).isReadable())
    {
        if (XL::Tree * tree = parse(xlPath))
        {
            // If any mandatory attribute is missing, module is ignored
            QString id =  moduleAttr(tree, "id");
            QString name = moduleAttr(tree, "name");
            XL::Version version;
            FindAttribute findAttribute("module_description", "version");
            if (Tree *versionTree = tree->Do(findAttribute))
                version = parseVersion(versionTree);
            if (id != "" && name != "")
            {
                m = ModuleInfoPrivate(+id, +moduleDir);
                m.name = +name;
                m.version = version;
                m.desc = +moduleAttr(tree, "description");
                QString iconPath = QDir(moduleDir).filePath("icon.png");
                if (QFile(iconPath).exists())
                    m.icon = +iconPath;
                m.author = +moduleAttr(tree, "author");
                m.website = +moduleAttr(tree, "website");
                m.importName = +moduleAttr(tree, "import_name");
                m.autoLoad = (moduleAttr(tree, "auto_load") != "");
                m.onLoadError = +moduleAttr(tree, "on_load_error");
#ifdef Q_OS_WIN
                m.windowsLoadPath = +moduleAttr(tree, "windows_load_path");
                m.expandSpecialPathTokens();
#endif
            }
            else
            {
                cause = tr("Missing ID, name or version");
            }
        }
        else
        {
            cause = tr("Could not parse %1").arg(xlPath);
        }
    }
    if (m.id != "")
    {
        // We have a valid module. Try to get its version from Git.
        XL::Version gitVer = gitVersion(moduleDir);
        if (gitVer)
            m.version = gitVer;

        // Check if there is a pending update
        if (applyPendingUpdate(m))
        {
            gitVer = gitVersion(moduleDir);
            if (gitVer)
                m.version = gitVer;
        }

        // Look for online documentation file
        QString qchPath = moduleDir + "/doc/" + TaoApp->lang + "/qch";
        QDir qchDir(qchPath);
        QStringList files = qchDir.entryList(QStringList("*.qch"),
                                             QDir::Files);
        if (files.isEmpty())
        {
            // Look for english by default
            qchPath = moduleDir + "/doc/en/qch";
            QDir qchDir(qchPath);
            files = qchDir.entryList(QStringList("*.qch"),
                                                 QDir::Files);
        }
        foreach(QString f, files)
            m.qchFiles << qchPath + "/" + f;
    }
    else
    {
        warnInvalidModule(moduleDir, cause);
    }
    return m;
}


bool ModuleManager::hasPendingUpdate(QString moduleDir)
// ----------------------------------------------------------------------------
//   Check if latest local tag is newer than current version
// ----------------------------------------------------------------------------
{
    bool hasUpdate = false;
    XL::Version current = gitVersion(moduleDir);
    XL::Version latestLocal = latestTag(moduleDir);
    if (current && latestLocal)
        hasUpdate = (latestLocal > current);
    record(modules, "%+s pending update", hasUpdate ? "Has" : "No");
    return hasUpdate;
}


XL::Version ModuleManager::latestTag(QString moduleDir)
// ----------------------------------------------------------------------------
//   Return latest local tag or ""
// ----------------------------------------------------------------------------
{
    RepositoryFactory::Mode mode = RepositoryFactory::OpenExistingHere;
    repository_ptr repo = RepositoryFactory::repository(moduleDir, mode);
    XL::Version latest;
    if (repo && repo->valid())
        latest = repo->highestVersionTag();
    return latest;
}


bool ModuleManager::applyPendingUpdate(const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Checkout latest local tag if current version is not latest
// ----------------------------------------------------------------------------
{
    bool hasUpdate = hasPendingUpdate(+m.path);
    if (hasUpdate)
    {
        XL::Version latest        = latestTag(+m.path);
        XL::Version current       = gitVersion(+m.path);
        record(modules,
               "Installing update %s -> %s", text(current), text(latest));
        emit updating(+m.name);
        RepositoryFactory::Mode mode = RepositoryFactory::OpenExistingHere;
        repository_ptr repo = RepositoryFactory::repository(+m.path, mode);
        repo->checkout(text(latest));
        return true;
    }
    return false;
}


XL::Version ModuleManager::gitVersion(QString moduleDir)
// ----------------------------------------------------------------------------
//   Try to find the version of the module using Git
// ----------------------------------------------------------------------------
{
    XL::Version version;
    RepositoryFactory::Mode mode = RepositoryFactory::OpenExistingHere;
    repository_ptr repo = RepositoryFactory::repository(moduleDir, mode);
    if (repo && repo->valid())
        version = XL::Version(repo->versionTag().c_str());
    return version;
}


XL::Tree * ModuleManager::parse(QString xlFile)
// ----------------------------------------------------------------------------
//   Parse the XL file of a module
// ----------------------------------------------------------------------------
{
    XL::Syntax     syntax (XL::MAIN->syntax);
    XL::Positions &positions = XL::MAIN->positions;
    XL::Errors     errors;
    XL::Parser     parser(+xlFile, syntax, positions, errors);
    return parser.Parse();
}


QString ModuleManager::moduleAttr(XL::Tree *tree, QString attribute)
// ----------------------------------------------------------------------------
//   Look for attribute value in module_description section of tree
// ----------------------------------------------------------------------------
{
    QString val;
    Tree * v = moduleAttrAsTree(tree, attribute);
    if (v)
    {
        Text_p t = toText(v);
        if (t)
            val = +(t->value);
    }
    return val;
}


Tree * ModuleManager::moduleAttrAsTree(XL::Tree *tree, QString attribute)
// ----------------------------------------------------------------------------
//   Look for attribute pointer in module_description section of tree
// ----------------------------------------------------------------------------
{
    // Look first in the localized description block, if present
    FindAttribute action("module_description", +attribute, +TaoApp->lang);
    Tree * t = tree->Do(action);
    if (!t)
    {
        FindAttribute action("module_description", +attribute);
        t = tree->Do(action);
    }
    return t;
}


Text * ModuleManager::toText(Tree *what)
// ----------------------------------------------------------------------------
//   Try to reduce 'what' to a single Text element (perform '&' concatenation)
// ----------------------------------------------------------------------------
{
    Text * txt = what->AsText();
    if (txt)
        return txt;
    Block * block = what->AsBlock();
    if (block)
        return toText(block->child);
    Infix * inf = what->AsInfix();
    if (inf && (inf->name == "&" || inf->name == "\n"))
    {
        Text * left = toText(inf->left);
        if (!left)
            return NULL;
        Text * right = toText(inf->right);
        if (!right)
            return NULL;
        return new Text(left->value + right->value);
    }
    return NULL;
}


bool ModuleManager::load(Context &context,
                         const QList<ModuleInfoPrivate> &mods)
// ----------------------------------------------------------------------------
//   Load modules, in sequence
// ----------------------------------------------------------------------------
{
    bool ok = true;
    foreach(ModuleInfoPrivate m, mods)
        ok &= load(context, m);
    return ok;
}


bool ModuleManager::load(Context &context, const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Load one module
// ----------------------------------------------------------------------------
{
    bool ok = true;
    record(modules, "Loading module %s id %s path %s", m.name, m.id, m.path);
    ok = loadNative(context, m);
    if (ok)
        ok = loadXL(context, m);

    return ok;
}


bool ModuleManager::loadXL(Context &/*context*/, const ModuleInfoPrivate &/*m*/)
// ----------------------------------------------------------------------------
//   Load the XL code of a module
// ----------------------------------------------------------------------------
{
    // Do not import module definitions at startup time but only on explicit
    // import (see importModule)
    // REVISIT: here we probably want to execute only a specific part of
    // the module's XL file
    return true;

#if 0
    IFTRACE(modules)
        debug() << "  Loading XL source\n";

    QString xlPath = m.xlPath();

    // module_description <indent block> evaluates as nil
    // REVISIT: bind a native function and use it to parse the block?
    Name *n = new Name("module_description");
    Block *b = new Block(new Name("x"), XL::Block::indent,
                         XL::Block::unindent);
    Prefix *from = new Prefix(n, b);
    Name *to = new Name("nil");
    context->Define(from, to);

    bool ok = XL::xl_import(context, +xlPath) != NULL;

    ModuleInfoPrivate *minfo = moduleById(m.id);
    if (ok && minfo)
        minfo->loaded = true;

    XL::source_files::iterator it = XL::MAIN->files.find(+xlPath);
    XL::source_files::iterator end = XL::MAIN->files.end();
    if (it != end)
    {
        XL::SourceFile &sf = (*it).second;
        Context &moduleContext = sf.context;

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
        record(modules, "Changing current directory to %s", path);
        if (getcwd(wd, sizeof(wd)))
            if (chdir(path.toStdString().c_str()) < 0)
                perror("Tao chdir");
    }
    ~SetCwd()
    {
        record(modules, "Restoring current directory to %s", wd);
        if (chdir(wd) < 0)
            perror("Tao chdir restore");
    }

    char wd[MAXPATHLEN];
};

#ifdef Q_OS_WIN

struct SetPath
// ----------------------------------------------------------------------------
//   Temporarily change %PATH%
// ----------------------------------------------------------------------------
{
#define VAR L"PATH"
#define BUFSIZE 4096

    SetPath(QString path) : exists(true), savedPath(NULL), path(path)
    {
        if (path.isEmpty())
            return;
        IFTRACE(modules)
        {
            ModuleManager::debug() << "    Setting %PATH% to: "
                                   << +path << "\n";
        }
        savedPath = (WCHAR *)malloc(BUFSIZE * sizeof(WCHAR));
        if (savedPath == NULL)
        {
            std::cerr << "SetPath: malloc: out of memory\n";
            return;
        }
        DWORD st = GetEnvironmentVariableW(VAR, savedPath, BUFSIZE);
        if (st == 0)
        {
            DWORD err = GetLastError();
            free(savedPath);
            savedPath = NULL;
            if (err != ERROR_ENVVAR_NOTFOUND)
            {
                std::cerr << "SetPath: GetEnvironmentVariable failed ("
                          << err << ")\n";
                return;
            }
            exists = false;
        }
        else if (BUFSIZE < st)
        {
            savedPath = (WCHAR *)realloc(savedPath, st*sizeof(WCHAR));
            if (savedPath == NULL)
            {
                std::cerr << "SetPath: realloc: out of memory\n";
                return;
            }
        }
        setVar((const WCHAR*)path.utf16());
    }
    ~SetPath()
    {
        if (path.isEmpty())
            return;
        IFTRACE(modules)
            ModuleManager::debug() << "    Restoring %PATH% to: "
                << +QString::fromUtf16((const ushort *)savedPath) << "\n";
        if (exists && savedPath)
            setVar(savedPath);
        else if (!exists)
            setVar(NULL);
        free(savedPath);
    }

protected:
    void setVar(const WCHAR *val)
    {
        if (!SetEnvironmentVariableW(VAR, val))
        {
            std::cerr << "SetPath: SetEnvironmentVariable failed ("
                      << GetLastError() << ")\n";
        }
    }

    bool     exists;
    WCHAR *  savedPath;
    QString  path;

#undef BUFSIZE
};


void ModuleManager::ModuleInfoPrivate::expandSpecialPathTokens()
// ----------------------------------------------------------------------------
//   Expand some special tokens in the string read from windows_load_path
// ----------------------------------------------------------------------------
{
#define BUFSIZE 32*1024
    QString path = +windowsLoadPath;
    path.replace("@loader_path", QDir::toNativeSeparators(libDir()));
    WCHAR * wpath = (WCHAR *)malloc(BUFSIZE * sizeof(WCHAR));
    if (wpath)
    {
        DWORD ret;
        ret = ExpandEnvironmentStringsW((WCHAR *)path.utf16(), wpath, BUFSIZE);
        if (ret == 0 || BUFSIZE < ret)
        {
            std::cerr << "expandSpecialPathTokens: ExpandEnvironmentStringsW "
                         "failed (" << GetLastError() << ")\n";
        }
        else
        {
            path = QString::fromUtf16((const ushort *)wpath);
        }
        free(wpath);
    }
    else
    {
        std::cerr << "expandSpecialPathTokens: malloc: out of memory\n";
    }
    windowsLoadPath = +path;
#undef BUFSIZE
}

#endif



bool ModuleManager::loadNative(Context &context XL_UNUSED,
                               const ModuleInfoPrivate &m)
// ----------------------------------------------------------------------------
//   Load the native code of a module (shared libraries under lib/)
// ----------------------------------------------------------------------------
{
    record(modules,
           "Looking for native library for module %s id %s", m.name, m.id);

    ModuleInfoPrivate &minfo = moduleById(m.id);
    bool ok;
    if (minfo.hasNative)
    {
        // Change current directory, just the time to load any module dependency
        QString path = m.libPath();
        QString libdir = m.libDir();
        SetCwd cd(libdir);
#ifdef Q_OS_WIN
        SetPath sp(+minfo->windowsLoadPath);
#endif
        QLibrary * lib = new QLibrary(path, this);
        if (lib->load())
        {
            path = lib->fileName();
            record(modules, "Loaded native module %s path %s", m.name, path);
            ok = isCompatible(lib, m.name);
            if (ok)
            {
                // enter_symbols is called later, when module is imported
                minfo.graphicStatePtr =
                        (GraphicState **) lib->resolve("graphic_state");
                if (minfo.graphicStatePtr)
                {
                    record(modules,
                           "Setting graphic states pointer at %p to %p",
                           minfo.graphicStatePtr, OpenGLState::Current());
                    *minfo.graphicStatePtr = OpenGLState::Current();
                }

                module_init_fn moduleInit = (module_init_fn)
                    lib->resolve("module_init");
                record(modules,
                       "Module %s id %s module_init is %p",
                       m.name, m.id, moduleInit);
                if (moduleInit)
                {
                    int rc = moduleInit(&api, &m);
                    if (rc)
                    {
                        record(modules_warning,
                               "Return code %d from module_init %s",
                               rc, minfo.onLoadError);
                        if (minfo.onLoadError != "")
                            warnLibraryLoadError(+minfo.name, "",
                                                 +minfo.onLoadError);
                        return false;
                    }
                }

                minfo.preferences = (module_preferences_fn)
                    lib->resolve("show_preferences");
                record(modules, "Show preference function for %s is %p",
                       m.name, minfo.preferences);

                QTranslator *translator = new QTranslator;
                QString tr_name = m.dirname() + "_" + TaoApp->lang;
                if (translator->load(tr_name, +m.path))
                {
                    Application::installTranslator(translator);
                    minfo.translator = translator;
                    record(modules, "Translations loaded from %s", tr_name);
                }
                else
                {
                    delete translator;
                }

                minfo.native = lib;
                emit modulesChanged();
            }
        }
        else
        {
            record(modules_warning, "Load error %s", lib->errorString());
            warnLibraryLoadError(+minfo.name, lib->errorString(),
                                 +minfo.onLoadError);
            delete lib;
            minfo.inError = true;
        }
    }
    else
    {
        record(modules, "Module %s has no native library", minfo.name);
    }
    return (minfo.hasNative && minfo.native);
}


bool ModuleManager::isCompatible(QLibrary * lib, text name)
// ----------------------------------------------------------------------------
//   Return true if library was built with a compatible version of the Tao API
// ----------------------------------------------------------------------------
{
    record(modules, "Checking API compatibility for %p", lib);
    XL::Version current        = TAO_MODULE_API_CURRENT;
    XL::Version compatible     = TAO_MODULE_API_COMPATIBLE;
    XL::Version *moduleVersion = (XL::Version *)
        lib->resolve("module_api_version");
    if (!moduleVersion)
    {
        record(modules_warning, "Library has no version information");
        return false;
    }
    record(modules, "Module version %s current %s compatible with %s",
           *moduleVersion, current, compatible);
    bool ok = moduleVersion->IsCompatibleWith(compatible)
        ||    moduleVersion->IsCompatibleWith(current);
    if (!ok)
    {
        record(modules_warning,
               "Module %s version %s is not compatible with %s (nor %s)",
               name, *moduleVersion, compatible, current);
        warnBinaryModuleIncompatible(lib, name);
    }
    return ok;
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
    record(modules_warning,
           "Skipping invalid module %s cause %s", moduleDir, cause);

    ErrorMessageDialog err;
    err.setWindowTitle(tr("Invalid module"));
    QString msg = tr("Skipping invalid module %1\n"
                     "%2")
        .arg(moduleDir)
        .arg(cause);
    err.showMessage(msg);
}


void ModuleManager::warnDuplicateModule(const ModuleInfoPrivate &m,
                                        const ModuleInfoPrivate &existing)
// ----------------------------------------------------------------------------
//   Tell user of conflicting new module (will be ignored)
// ----------------------------------------------------------------------------
{
    if (m.dirname()  != existing.dirname() ||
        m.importName != existing.importName)
    {
        record(modules_warning,
               "Module with id from %s duplicates one from %s",
               m.id, m.path, existing.path);

        ErrorMessageDialog err;
        err.setWindowTitle(tr("Duplicate module"));
        QString msg = tr("A duplicate module was found.\n\n"
                         "'%1' and '%2' share the same identifier "
                         "(%3), which looks suspicious.\n\n"
                         "Only the second module will be loaded. "
                         "You may want to change one UUID.")
                .arg(+m.path).arg(+existing.path).arg(+m.id);
        err.showMessage(msg);
    }
}


void ModuleManager::warnLibraryLoadError(QString name, QString errorString,
                                         QString moduleSuppliedText)
// ----------------------------------------------------------------------------
//   Tell user that library failed to load (module will be ignored)
// ----------------------------------------------------------------------------
{
    record(modules_warning,
           "Module %s cannot be initialized. Error: %s. Module info: %s",
           name, errorString, moduleSuppliedText);
    QMessageBox warn;
    warn.setWindowTitle(tr("Tao modules"));
    if (moduleSuppliedText.isEmpty())
    {
        QString msg = tr("Module %1 cannot be initialized.\n%2")
            .arg(name)
            .arg(errorString);
        warn.setText(msg);
    }
    else
    {
        warn.setText(tr("Module %1 cannot be initialized.").arg(name));
        warn.setInformativeText(moduleSuppliedText);
        warn.setDetailedText(errorString);
    }
    warn.exec();
}


void ModuleManager::warnBinaryModuleIncompatible(QLibrary *lib, text name)
// ----------------------------------------------------------------------------
//   Tell user about incompatible binary module (will be ignored)
// ----------------------------------------------------------------------------
{
    record(modules_warning,
           "Skipping incompatible binary module %s", lib->fileName());

    ErrorMessageDialog err;
    err.setWindowTitle(tr("Incompatible binary module"));
    QString msg = tr("Skipping module %1 found in library file %2 "
                     "because it's binary incompatible")
        .arg(+name)
        .arg(lib->fileName());
    err.showMessage(msg);
}


XL::Version ModuleManager::parseVersion(Tree *versionId)
// ----------------------------------------------------------------------------
//   Verify if we have a valid version number
// ----------------------------------------------------------------------------
//   Version numbers can have one of three forms:
//   - An integer value, e.g 1, which is the same as 1.0
//   - A real value, e.g.  1.0203,
//   - A text value, e.g. "1.0203"
{
    if (Integer *iver = versionId->AsInteger())
        return XL::Version(iver->value);
    if (Real *rver = versionId->AsReal())
        return XL::Version(unsigned(rver->value),
                           unsigned(1000 * rver->value) % 1000);
    if (Text *tver = versionId->AsText())
        return XL::Version(tver->value.c_str());
    XL::Ooops("Malformed version number $1", versionId);
    return XL::Version();
}


QStringList ModuleManager::qchFiles()
// ----------------------------------------------------------------------------
//   Return documentation files (*.qch) of all enabled modules
// ----------------------------------------------------------------------------
{
    QStringList files;
    foreach (ModuleInfoPrivate m, modules)
        if (m.enabled)
            files << m.qchFiles;
    return files;
}


void ModuleManager::unloadImported()
// ----------------------------------------------------------------------------
//   Unload all modules that were explicitely imported
// ----------------------------------------------------------------------------
//   REVISIT: here we just call module_exit for the native library. It is
//   enough to fix #1925. We do not unload the library, because XL still has
//   references to the code (primitives added by enter_symbols). We don't call
//   module_init on subsequent imports, either.
{
    record(modules, "Unloading imported modules");
    foreach (ModuleInfoPrivate m, modules)
    {
        if (m.loaded && m.hasNative && !(m.autoLoad || m.importName == ""))
        {
            QLibrary * lib = m.native;
            module_exit_fn moduleExit = (module_exit_fn)
                lib->resolve("module_exit");
            record(modules,
                   "Module %s id %s exit function %p",
                   m.name, m.id, moduleExit);
            if (!moduleExit)
                continue;
            int rc = moduleExit();
            record(modules,
                   "Module %s id %s exit function %p returned %d",
                   m.name, m.id, moduleExit, rc);
            if (rc)
                record(modules_warning,
                       "Module %s exit function return error code %d",
                       m.name, rc);
        }
    }
}


void ModuleManager::updateGraphicStatePointers(GraphicState *newState)
// ----------------------------------------------------------------------------
//   Update all modules that have a pointer to the current GraphicState
// ----------------------------------------------------------------------------
{
    record(modules,
           "GraphicState changed to %p, updating modules", newState);
    foreach (ModuleInfoPrivate m, modules)
    {
        if (m.graphicStatePtr)
        {
            record(modules,
                   "Module %s graphic state pointer %p from %p to %p",
                   m.name, m.graphicStatePtr, *m.graphicStatePtr, newState);
            *m.graphicStatePtr = newState;
        }
    }
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
    record(modules,
           "Start checking for updates for module %s (current version %s)",
           m.name, m.version);
    bool inProgress = false;
    repo = RepositoryFactory::repository(+m.path,
                                         RepositoryFactory::OpenExistingHere);
    if (repo && repo->valid())
    {
        QStringList tags = repo->tags();
        if (!tags.isEmpty())
        {
            foreach (QString t, tags)
            {
                XL::Version current(+t);
                if (m.version == current)
                {
                    proc = repo->asyncGetRemoteTags("origin");
                    connect(repo.data(),
                            SIGNAL(asyncGetRemoteTagsComplete(QStringList)),
                            this,
                            SLOT(processRemoteTags(QStringList)));
                    repo->dispatch(proc);
                    inProgress = true;
                    break;
                }
            }
            if (!inProgress)
                record(modules_warning,
                       "Module %s current version %s does not match any tag",
                       m.name, m.version);
        }
        else
        {
            record(modules, "Module %s has no local tags", m.name);
        }
    }
    else
    {
        record(modules, "Module %s has no git repository", m.name);
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
    record(modules, "Module %s processing tags: %s", m.name, +tags.join(", "));
    bool hasUpdate = false;
    if (!tags.isEmpty())
    {
        XL::Version latest = m.version;
        foreach (QString tag, tags)
        {
            XL::Version current(+tag);
            if (latest < current)
                latest = current;
        }

        hasUpdate = latest > m.version;
        record(modules,
               "Module %s processed tags, latest %s, current %s, %+s update",
               m.name, latest, m.version, hasUpdate ? "has" : "no");
        mm.modules[+m.id].latestVersion = latest;;
    }
    else
    {
        record(modules, "Module %s: No remote tags", m.name);
    }

    mm.modules[+m.id].hasUpdate = hasUpdate;
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
        hasUpdate = true;
    pending.remove(+m.id);
    emit progress(num - pending.count());
    if (pending.isEmpty())
    {
        emit complete(hasUpdate);
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
//   emit complete signal
// ----------------------------------------------------------------------------
{
    bool ok = (status ==  QProcess::NormalExit && exitCode == 0);
    if (ok)
    {
        // NB: Defer checkout on next restart, because module may be in use
        ModuleManager::ModuleInfoPrivate &p = mm.moduleById(m.id);
        p.hasUpdate = false;
    }
    emit complete(ok);
}



// ============================================================================
//
//    FindAttribute implementation
//
// ============================================================================

Tree *ModuleManager::FindAttribute::Do(Block *what)
// ----------------------------------------------------------------------------
//   Find module attribute in a block
// ----------------------------------------------------------------------------
{
    if (!sectionFound)
        return NULL;
    return what->child->Do(this);
}


Tree *ModuleManager::FindAttribute::Do(Infix *what)
// ----------------------------------------------------------------------------
//   Find module attributes in an infix
// ----------------------------------------------------------------------------
{
    if (what->name == "\n" || what->name == ";")
    {
        if (Tree * t = what->left->Do(this))
            return t;
        return what->right->Do(this);
    }
    return NULL;
}


Tree *ModuleManager::FindAttribute::Do(Prefix *what)
// ----------------------------------------------------------------------------
//  Find module attributes in a prefix
// ----------------------------------------------------------------------------
{
    XL::Name *name = what->left->AsName();
    if (!sectionFound)
    {
        if (name && name->value == sectionName)
        {
            if (lang == "")
            {
                if (what->right->AsBlock())
                {
                    sectionFound = true;
                    if (Tree * t = what->right->Do(this))
                        return t;
                    sectionFound = false;
                }
            }
            else
            {
                Infix * inf =  what->right->AsInfix();
                if (inf)
                {
                    Text * t = inf->left->AsText();
                    if (t && t->value == lang)
                    {
                        if (inf->right->AsBlock())
                        {
                            sectionFound = true;
                            if (Tree * t = inf->right->Do(this))
                                return t;
                            sectionFound = false;
                        }
                    }
                }
            }
        }
        return NULL;
    }
    if (name && name->value == attrName)
        return what->right;
    return NULL;
}

}
