#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H
// ****************************************************************************
//  module_manager.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    Locate, load, enable, disable Tao modules.
//    The architecture is largely inspired from Mozilla (Firefox) extensions
//      https://developer.mozilla.org/en/Extensions
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
    module.xl    A XL file, loaded by the module manager
    lib/         [Optional] Native code as shared libraries

  2.1 Structure of main.xl

main.xl must contain at least the following.

module_description
    id "2D3DD293-C8CC-4AFC-B15A-EBB243081EFC"  // Any unique identifier
    name "My great Tao module"
    description "Undoubtedly, the nicest of all Tao modules."
    creator "XYZ company"
    website "http://greatmodule.xyz.com/"

    target_application
        id "XXXX TODO XXXX" // Tao's application ID
        min_version "0.3"
        max_version "0.3.*"

module_init
    // Some XL code that will be evaluated on init

In this code:
 - min_version is the earliest version of Tao the module will work with.
 - max_version is the newest version of Tao the module is known to work with.
 0.3.* means that the module works with Tao 0.3 and any subsequent 0.3.x
 release.

Note that the module version does not appear in module.xl, because it is read
from the git repository.

3. Where are modules installed?

Module files may be installed anywhere, but are typically stored either under
the current user's application data folder, or in a system-wide location.
These locations depend on the operating system. We will call the per-user
folder "U" and the system-wide folder "S".

4. How are modules loaded?

When Tao starts up, it checks folders U/modules and S/modules, looking for
unknown modules. A module is unknown if it is not registered in the user's
settings (we use QSettings for that). For each unknown module, the user is
prompted with a description of the module and has to choose whether to enable
the module or not. If user accepts, the module is initialized and the module
ID is stored into the user's settings, along with the "enabled" flag.
Otherwise, the module ID is stored with the "disabled" flag.
The module path is also saved.

Then, all other modules marked as "enabled" in the user settings are loaded
and initialized.

At any time, the user may register a new module.

5. How are modules installed?

Several options:
  - Manually, by downloading module files under U/modules or S/modules then
  restarting Tao. Download typically goes through "git clone".
  - By clicking on a specially crafted tao: link (which will "git clone" the
  module under U or S).
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
tag. Comparison is performed assuming the usual versioning scheme, where:
    1.0 < 1.1 < 1.1.2 < 1.2 < 1.9 < 1.10 < 2.0
(lexicographic comparison left to right of integers separated by dots).

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

Any XL code goes into module.xl and possibly other .xl files.

  8.2. Native code

New XL primitives can be added by using the Tao module API, and generating
a shared library that will be loaded by module_init.

 */

#include "tree.h"
#include "repository.h"
#include "tao_utf8.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QSet>
#include <iostream>


namespace Tao {

using namespace XL;

struct ModuleManager : public QObject
// ----------------------------------------------------------------------------
//   A singleton class to deal with modules
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    static ModuleManager * moduleManager();

    struct ModuleInfo
    // ------------------------------------------------------------------------
    //   Information about a module
    // ------------------------------------------------------------------------
    {
        ModuleInfo() {}
        ModuleInfo(QString id, QString path, bool enabled = false)
            : id(id), path(path), enabled(enabled), loaded(false),
              updateAvailable(false)
            {}

        QString id;
        QString path;

        // Properties
        QString name;
        QString desc;
        QString icon;
        QString ver;

        // Runtime attributes
        QString latest;
        bool    enabled;
        bool    loaded;
        bool    updateAvailable;

        bool operator==(const ModuleInfo &o) const
        {
            return (id == o.id);
        }

        text toText() const
        {
            return +id + " (" + +path + ")";
        }

        void copyProperties(const ModuleInfo &o)
        {
            name = o.name;  desc = o.desc;  icon = o.icon;  ver = o.ver;
        }
    };

    bool                loadAll();
    QList<ModuleInfo>   allModules()    { return modules; }
    void                setEnabled(QString id, bool enabled);

    virtual bool        askRemove(const ModuleInfo &m, QString reason = "");
    virtual bool        askEnable(const ModuleInfo &m, QString reason = "");
    virtual void        warnDuplicateModule(const ModuleInfo &m);

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

    struct FindAttribute : Action
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
    bool                checkConfig();
    bool                removeFromConfig(const ModuleInfo &m);
    bool                addToConfig(const ModuleInfo &m);

    bool                checkNew();
    bool                checkNew(QString parentDir);
    QList<ModuleInfo>   newModules(QString parentDir);
    ModuleInfo          readModule(QString moduleDir);
    QString             gitVersion(QString moduleDir);

    Tree *              parse(QString xlPath);
    QString             moduleAttr(Tree * tree, QString attribute);

    bool                load(const QList<ModuleInfo> &mods);
    bool                load(const ModuleInfo &m);

    std::ostream &      debug();
    void                debugPrint(const ModuleInfo &m);

private:
    QString                     u, s;
    QList<ModuleInfo>           modules;
    QMap<QString, ModuleInfo *> modulesById;


    static ModuleManager * instance;
    static Cleanup         cleanup;

friend class CheckForUpdate;
friend class CheckAllForUpdate;
friend class UpdateModule;

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
        if (mm.modulesById.contains(id))
            m = *(mm.modulesById[id]);
    }

    bool           start();

signals:
    void           complete(ModuleManager::ModuleInfo m, bool updateAvailable);

private slots:
    void           processRemoteTags(QStringList tags);

private:
    std::ostream & debug()  { return mm.debug(); }

private:
    ModuleManager &           mm;
    ModuleManager::ModuleInfo m;
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
    void           processResult(ModuleManager::ModuleInfo m, bool need);

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
        if (mm.modulesById.contains(id))
            m = *(mm.modulesById[id]);
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
    ModuleManager::ModuleInfo m;
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
