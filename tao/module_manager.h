#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H
// ****************************************************************************
//  module_manager.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    Locate, load, enable, disable Tao modules.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

/*

// TODO:
//   - localization
//   - do we need some kind of dependency information? (load order)

1. What is a Tao module?

A Tao module is made up of some XLR code, and optionally some native code in
one or several shared libraries. It provides additional functions to the main
application, such as:
  - graphical elements (themes)
  - utility functions
  - whole new features with menus, buttons etc.

2. What is the structure of a module?

A module is a directory with the following structure:

  <module_name>/
    .git/        [Optional] Git folder, used to manage upgrades
    <module_name>.xl  A XL file, loaded by the module manager
    icon.png     [Optional] Module icon
    lib/         [Optional] Native code as a shared library

  2.1 Structure of <module_name>.xl

<module_name>.xl may contain the following.

//-----
module_description
    id "2D3DD293-C8CC-4AFC-B15A-EBB243081EFC"  // Any unique identifier
    name "My great Tao module"
    description "Undoubtedly, the nicest of all Tao modules."
    author "XYZ company"
    website "http://greatmodule.xyz.com/"
    import_name "GreatModule"

module_init
    // Some XL code that will be evaluated on init
//-----

id  [Mandatory]
A unique identifier for the module. It is recommended to use uuidgen to get a
new identifier. The module ID is used to exclude duplicates when Tao scans the
user and application directories, looking for modules.

name  [Mandatory]
The short name of the module. Used in the user interface (module preferences,
for instance).

description  [Optional]
A plain text module description.

version  [Mandatory, conditional]
The version attribute is used only if version cannot be read from the module's
Git repository (such as, if the module is not a Git repository, or it contains
no tag), in which case it is mandatory.

author [Optional]
The person or company that created the module.

website  [Optional]
The web site where one can find information about the module.

import_name [Optional]
The name to use if the module is to be explicitely imported. That is, if
import_name is "MyModule", the module can be imported with:
  import MyModule 1.0

3. Where are modules installed?

Module files may be installed anywhere, but are typically stored either under
the current user's application data folder, or in a system-wide location.
These locations depend on the operating system. We will call the per-user
folder "U" and the system-wide folder "S".

4. How are modules loaded?

  4.1 Common module initialization

When Tao starts up, it checks folders U/modules and S/modules, looking for
unknown modules. A module is unknown if it is not registered in the user's
settings (we use QSettings for that). For each unknown module, the user is
prompted with a description of the module and has to choose whether to enable
the module or not. If user accepts, the module is initialized and the module
ID is stored into the user's settings, along with the "enabled" flag.
Otherwise, the module ID is stored with the "disabled" flag.
Note: currently user is not prompted, all modules are enabled by default.

Then, all other modules marked as "enabled" in the user settings are loaded
and initialized (see details below).

  4.2 Explicit import

A Tao document may explicitely import a module, for instance to benefit from
new XL constructs or primitives. Explicit import is achived by the following
line:

  import ModuleName 1.1

When Tao encounters the import statement, it looks up ModuleName in the list
of currently loaded modules, checks the version compatibility, and then makes
the module definitions available to the Tao document.
Version matching is documented in the online doc.

Without explicit import, no definition from <module_name>.xl are reachable from
the document.

5. How are modules installed?

Several options:
  - Manually, by downloading module files under U/modules or S/modules then
  restarting Tao. Download typically goes through "git clone".
  - By clicking on a specially crafted tao: link (which will "git clone" the
  module under U or S) (TBD).
  - Through the Tao interface (TBD).

6. How are modules upgraded?

Modules installed as Git repositories are upgradeable either manually, or
automatically. Tao will use the information available to Git to determine
if newer versions exists.

  6.1. Comparing versions and setting "update available" flag

When checking for update, Tao will set the "update available" flag on a
module in the following circumstances:

(1) The local module has to be a valid Git repository;
(2) It must have at least one tag (annotated or not, it doesn't matter),
and the current checked out version must match a local tag (i.e., no
development version);
(3) The remote name "origin" must be a valid repository and must have at
least one tag (annotated or not);
(4) The highest local tag must be strictly lower than the highest remote
tag. Comparison is performed by ModuleManager::parseVersion().

  6.2. Maintaining a module repository

(1) Only use one branch: master
(2) To publish a new version, make one or several commits on master, then
create a new tag (with a higher version number). Any Tao user who wants
to update will get the version that corresponds to the new tag.

7. How are modules configured?

Module specific settings are handled by the module itself. Modules will
typically add entries to the Tools menu (Tools > Module_name), and also add
entries into the Tao command chooser.

General module settings (listing the modules, enabling/disabling a module,
checking for update...) is achieved through the Preferences dialog.

8. How are modules implemented?

  8.1. XL code

Any XL code goes into <module_name>.xl and possibly other .xl files.

  8.2. Native code

New XL primitives can be added by using the Tao module API, and generating
a shared library that will be loaded by Tao before <module_name>.xl is loaded.
The base name of the library must be the name of the main module directory,
i.e., <module_name>.dll on Windows,lib<module_name>.dylib on MacOSX,
lib<module_name>.so on Linux.
To make building a Tao module easy, Tao has a special Qt project include
file, modules.pri, as well as a C/C++ API. There are also examples under the
'modules' directory.

  8.2.1 Source files for a typical Tao module

    mymodule.pro   Qt project file. Uses Tao's modules.pri.
    mymodule.xl    Module information (id, description...) and XL code.
    mymodule.tbl   Declarations of new XL primitives. Processed by the tbl_gen
                   script to produce mymodule_wrap.cpp, which defines functions
                   enter_symbols and delete_symbols.
    mymodule.cpp,
    *.cpp          Implementation of new XL primitives and any support code.
                   May use tao/module_api.h and tao/module_info.h.
    traces.tbl     Declaration of debug trace levels for the module.

  8.2.2 Native module API

    tao/module_api.h

This file defines the binary interface used by Tao to initialize/uninitialize
the module (through module_init/module_exit), as well as to add/delete
XL primitives (through enter_symbols/delete_symbols). See comments in the file
for details.

    tao/module_info.h

This header file contains the definition of the ModuleInfo structure, which
is used by Tao to communicate some module information to the module itself
(identifier, version, installation path...). Use of this header is optional.

  8.2.3 Order of calls

Native and XL functions of a module are called by Tao in the following order:

  1. module_init    (native)  Once, after library is loaded
  2. enter_symbols  (native)  When module is explicitely imported
  3. module_init    (XL)      Not in current implementation
  ...
  4. module_exit    (XL)      Not in current implementation
  5. delete_symbols (native)  Not in current implementation
  6. module_exit    (native)  Not in current implementation

 */

#include "tao.h"
#include "tao_tree.h"
#include "action.h"
#include "repository.h"
#include "tao_utf8.h"
#include "tao/module_api.h"
#include "tao/module_info.h"
#include "module_api_p.h"
#include "tree.h"
#include "context.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QSet>
#include <QLibrary>
#include <QDir>
#include <iostream>


namespace Tao {

struct ModuleManager : public QObject
// ----------------------------------------------------------------------------
//   A singleton class to deal with modules
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static ModuleManager * moduleManager();
    static XL::Tree_p      import(XL::Context_p context,
                                  XL::Tree_p self,
                                  XL::Tree_p what,
                                  bool execute);

    struct ModuleInfoPrivate : ModuleInfo
    // ------------------------------------------------------------------------
    //   Information about a module
    // ------------------------------------------------------------------------
    {
        ModuleInfoPrivate() : ModuleInfo(), enabled(enabled), loaded(false),
              updateAvailable(false), hasNative(false),
              native(NULL), context(NULL), inError(false)
            {}
        ModuleInfoPrivate(text id, text path = "", bool enabled = false)
            : ModuleInfo(id, path), enabled(enabled), loaded(false),
              updateAvailable(false), hasNative(false),
              native(NULL), context(NULL), inError(false)
            {}

        // Configuration attributes
        bool    enabled;

        // Runtime attributes
        text    latest;
        // loaded is set to true when xl file is imported and
        //           set to false when xl file is unloaded
        bool    loaded;
        bool    updateAvailable;
        // hasNative is true when the specified path for native library exists. It is
        // different from native only when specified file is not a compliant library.
        bool    hasNative;
        // native is the pointer to a compliant library (enter_symbols is present)
        QLibrary * native;
        XL::Context_p context;
        bool    inError;

        bool operator==(const ModuleInfoPrivate &o) const
        {
            return (id == o.id);
        }

        text toText() const
        {
            return id + " (" + path + ")";
        }

        void copyPublicProperties(const ModuleInfo &o)
        {
            *(ModuleInfo*)this = o;
        }

        QString dirname() const
        {
            return QDir(+path).dirName();
        }

        QString xlPath() const
        {
            QString ret = QDir(+path).filePath(dirname() + ".xl");
            if (!QFileInfo(ret).isReadable())
                ret = QDir(+path).filePath("module.xl"); // Backward compat.
            return ret;
        }

        QString libPath() const
        {
#           if   defined(CONFIG_MINGW)
            QString fmt("%1/%2/%3.dll");
#           elif defined(CONFIG_MACOSX)
            QString fmt("%1/%2/lib%3.dylib");
#           elif defined(CONFIG_LINUX)
            QString fmt("%1/%2/lib%3.so");
#           else
#           error Unknown OS - please define library name
#           endif
            QString ret = fmt.arg(+path).arg("lib").arg(dirname());
            if (!QFileInfo(ret).isReadable())
                ret = fmt.arg(+path).arg("lib").arg("module"); // Backwd compat.
            return ret;
        }

        bool operator<(const ModuleInfoPrivate o) const
        {
            return (name.compare(o.name) < 0);
        }
    };

    bool                loadAll(Context *context);
    bool                unloadAll(Context *context);
    QList<ModuleInfoPrivate>   allModules();
    void                setEnabled(QString id, bool enabled);
    bool                enabled() { return XL::MAIN->options.enable_modules; }
    bool                saveConfig();

    virtual bool        askRemove(const ModuleInfoPrivate &m,
                                  QString reason = "");
    virtual bool        askEnable(const ModuleInfoPrivate &m,
                                  QString reason = "");
    virtual void        warnInvalidModule(QString moduleDir, QString cause);
    virtual void        warnDuplicateModule(const ModuleInfoPrivate &m);
    virtual void        warnLibraryLoadError(QString name, QString errorString);
    virtual void        warnBinaryModuleIncompatible(QLibrary *lib);
    static double       parseVersion(Tree *versionId);
    static double       parseVersion(text versionId);

signals:
    void                loading(QString name);

private:
    ModuleManager()  {}
    ~ModuleManager() {}

    struct Cleanup
    // ------------------------------------------------------------------------
    //   Automatically delete the module manager instance
    // ------------------------------------------------------------------------
    {
        ~Cleanup()
        {
            if (ModuleManager::instance)
                delete ModuleManager::instance;
        }
    };


    struct FindAttribute : XL::Action
    // ------------------------------------------------------------------------
    //   Find value of an attribute in a named section
    // ------------------------------------------------------------------------
    {
        // This action looks for the following pattern in the tree:
        //
        // ...
        // sectionName
        //     ...
        //     attrname value
        //     ...
        //
        // In addition, sectionName and attrName have to be of type Name, and
        // must be prefixes: sectionName is prefix of the block, and attrName
        // is prefix of value.
        // If pattern is found at indentation level 0, a pointer to value is
        // returned. Otherwise, NULL is returned.
        FindAttribute (text sectionName, text attrName):
                sectionName(sectionName), attrName(attrName),
                sectionFound(false) {}

        Tree *DoBlock(Block *what);
        Tree *DoInfix(Infix *what);
        Tree *DoPrefix(Prefix *what);
        Tree *Do(Tree *what);

        text sectionName, attrName;
        bool sectionFound;
    };

    bool                init();
    bool                initPaths();

    bool                loadConfig();
    bool                removeFromConfig(const ModuleInfoPrivate &m);
    bool                addToConfig(const ModuleInfoPrivate &m);
    bool                cleanConfig();

    bool                checkNew();
    bool                checkNew(QString parentDir);
    QList<ModuleInfoPrivate>   newModules(QString parentDir);
    ModuleInfoPrivate          readModule(QString moduleDir);
    QString             gitVersion(QString moduleDir);

    Tree *              parse(QString xlPath);
    QString             moduleAttr(Tree * tree, QString attribute);

    bool                load(Context *, const QList<ModuleInfoPrivate> &mods);
    bool                load(Context *, const ModuleInfoPrivate &m);
    bool                loadXL(Context *, const ModuleInfoPrivate &m);
    bool                loadNative(Context *, const ModuleInfoPrivate &m);
    bool                isCompatible(QLibrary * lib);

    bool                unload(Context *, const ModuleInfoPrivate &m);
    bool                unloadXL(Context *, const ModuleInfoPrivate &m);
    bool                unloadNative(Context *, const ModuleInfoPrivate &m);

    static
    std::ostream &      debug();
    void                debugPrint(const ModuleInfoPrivate &m);
    void                debugPrintShort(const ModuleInfoPrivate &m);

    ModuleInfoPrivate * moduleById(text id);

    XL::Tree_p          importModule(XL::Context_p context,
                                     XL::Tree_p self,
                                     XL::Tree_p what,
                                     bool execute);
private:
    QString                     u, s;
    QMap<QString, ModuleInfoPrivate>   modules;
    ModuleApiPrivate                   api;


    static ModuleManager * instance;
    static Cleanup         cleanup;

friend class CheckForUpdate;
friend class CheckAllForUpdate;
friend class UpdateModule;
friend class SetCwd;

#   define USER_MODULES_SETTING_GROUP "Modules"
};

// ============================================================================
//
//    Context to perform "check for updates" for a single module
//
// ============================================================================

class CheckForUpdate : public QObject
// ------------------------------------------------------------------------
//   Asynchronously check if an update is available for a single module
// ------------------------------------------------------------------------
{
    Q_OBJECT

public:
    CheckForUpdate(ModuleManager &mm, QString id) : mm(mm)
    {
        if (mm.modules.contains(id))
            m = mm.modules[id];
    }

    bool           start();

signals:
    void           complete(ModuleManager::ModuleInfoPrivate m, bool updateAvailable);

private slots:
    void           processRemoteTags(QStringList tags);

private:
    std::ostream & debug()  { return mm.debug(); }

private:
    ModuleManager &           mm;
    ModuleManager::ModuleInfoPrivate m;
    repository_ptr            repo;
    process_p                 proc;
};

// ============================================================================
//
//    Context to perform "check for updates" for all modules
//
// ============================================================================

class CheckAllForUpdate : public QObject
// ------------------------------------------------------------------------
//   Asynchronously check updates for all current modules in ModuleManager
// ------------------------------------------------------------------------
{
    Q_OBJECT

public:
    CheckAllForUpdate(ModuleManager &mm) : mm(mm), updateAvailable(false) {}

    bool           start();

signals:
    void           minimum(int min);
    void           maximum(int max);
    void           progress(int p);
    void           complete(bool need);

private slots:
    void           processResult(ModuleManager::ModuleInfoPrivate m, bool need);

private:
    std::ostream & debug()  { return mm.debug(); }

private:
    ModuleManager & mm;
    QSet<QString>   pending;
    bool            updateAvailable;
    int             num;
};

// ============================================================================
//
//    Context to update a module
//
// ============================================================================

class UpdateModule : public QObject
// ------------------------------------------------------------------------
//   Asynchronously update a module
// ------------------------------------------------------------------------
{
    Q_OBJECT

public:
    UpdateModule(ModuleManager &mm, QString id) : mm(mm)
    {
        if (mm.modules.contains(id))
            m = mm.modules[id];
    }

    bool           start();

signals:
    void           minimum(int min);
    void           maximum(int max);
    void           progress(int percent);
    void           complete(bool success);

private slots:
    void           onFinished(int exitCode, QProcess::ExitStatus status);

private:
    std::ostream & debug()  { return mm.debug(); }

private:
    ModuleManager &           mm;
    ModuleManager::ModuleInfoPrivate m;
    repository_ptr            repo;
    process_p                 proc;
};

// ============================================================================
//
//    FindAttribute implementation
//
// ============================================================================

inline Tree *ModuleManager::FindAttribute::DoBlock(Block *what)
{
    if (!sectionFound)
        return NULL;
    return what->child->Do(this);
}

inline Tree *ModuleManager::FindAttribute::DoInfix(Infix *what)
{
    if (what->name == "\n" || what->name == ";")
    {
        if (Tree * t = what->left->Do(this))
            return t;
        return what->right->Do(this);
    }
    return NULL;
}

inline Tree *ModuleManager::FindAttribute::DoPrefix(Prefix *what)
{
    XL::Name * name = what->left->AsName();
    if (!sectionFound)
    {
        if (name && name->value == sectionName)
        {
            if (what->right->AsBlock())
            {
                sectionFound = true;
                if (Tree * t = what->right->Do(this))
                    return t;
                sectionFound = false;
            }
        }
        return NULL;
    }
    if (name && name->value == attrName)
        return what->right;
    return NULL;
}

inline Tree *ModuleManager::FindAttribute::Do(Tree *what)
{
    (void) what;
    return NULL;
}

}

#endif // MODULE_MANAGER_H
