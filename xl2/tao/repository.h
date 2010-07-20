#ifndef REPOSITORY_H
#define REPOSITORY_H
// ****************************************************************************
//  repository.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//     Abstract interface describing an SCM repository for the document
//     Derived class will provide implementations for git, hg, bzr
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
//  (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
//  Although the class is supposed to be abstract, the terminology is
//  largely inspired by Git. We had to pick one.

#include "tao.h"
#include "tree.h"
#include "tao_tree.h"
#include "process.h"
#include "main.h"
#include "ansi_textedit.h"
#include <QString>
#include <QProcess>
#include <QtGlobal>
#include <QMap>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QQueue>
#include <iostream>

namespace Tao {

class Repository : public QObject
// ----------------------------------------------------------------------------
//   A repository storing the history of documents
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------
    //   The kind of SCM tool available to the application
    // ------------------------------------------------------------------------
    enum Kind
    {
        Unknown,
        None,
        Git
    };

    // ------------------------------------------------------------------------
    //   The behavior in case of merge conflict
    // ------------------------------------------------------------------------
    enum ConflictResolution
    {
        CR_Unknown,
        CR_Ours,     // Keep local version
        CR_Theirs,   // Take remote version
        CR_Manual,   // Do nothing, user will manually resolve conflict
    };

    // ------------------------------------------------------------------------
    //   Repository states
    // ------------------------------------------------------------------------
    enum State
    {
        RS_Clean,     // Work area reflects last commit
        RS_NotClean,  // Work area has been modified since last commit
    };


    // ------------------------------------------------------------------------
    //   Commit information
    // ------------------------------------------------------------------------
    struct Commit
    {
        Commit() {}
        Commit(QString id, QString msg = ""): id(id), msg(msg) {}
        Commit(const Commit &o): id(o.id), msg(o.msg) {}
        Commit& operator = (const Commit &o)
        {
            id = o.id;
            msg = o.msg;
            return *this;
        }
        bool operator == (const Commit &o) const
        {
            return id == o.id && msg == o.msg;
        }
        bool operator != (const Commit &o) const
        {
            return ! operator ==(o);
        }
        QString toString()
        {
            QString ret = id;
            if (!msg.isEmpty())
                ret += " " + msg;
            return ret;
        }

        QString id;
        QString msg;
    };

    static struct Commit HeadCommit;

public:
    Repository(const QString &path): path(path), task("work"),
                                     pullInterval(XL::MAIN->options
                                                  .pull_interval),
                                     state(RS_Clean), whatsNew("") {}
    virtual ~Repository();

public:
    virtual bool        write(text fileName, Tree *tree);
    virtual XL::Tree *  read(text fileName);
    virtual bool        setTask(text name);
    virtual bool        selectWorkBranch();
    virtual bool        selectUndoBranch();
    virtual bool        isUndoBranch(text branch);
    virtual text        undoBranch(text name);
    virtual text        taskBranch(text name);
    virtual bool        mergeUndoBranchIntoTaskBranch(text name);
    virtual bool        mergeTaskBranchIntoUndoBranch(text name);
    virtual bool        idle();
    virtual void        markChanged(text reason);
    virtual void        abort(Process *proc);

public:
    virtual QString     userVisibleName()               = 0;
    virtual bool        valid()                         = 0;
    virtual bool        initialize()                    = 0;
    virtual text        branch()                        = 0;
    virtual QStringList branches()                      = 0;
    virtual bool        addBranch(QString name, bool force = false) = 0;
    virtual bool        delBranch(QString name, bool force = false) = 0;
    virtual bool        renBranch(QString oldName, QString newName, bool force = false) = 0;
    virtual bool        isRemoteBranch(text branch)     = 0;
    virtual bool        checkout(text name)             = 0;
    virtual bool        branch(text name)               = 0;
    virtual bool        add(text name)                  = 0;
    virtual bool        change(text name)               = 0;
    virtual bool        remove(text name)               = 0;
    virtual bool        rename(text from, text to)      = 0;
    virtual bool        commit(text msg = "",bool all=false) = 0;
    virtual bool        revert(text id)                 = 0;
    virtual bool        cherryPick(text id)             = 0;
    virtual bool        merge(text branch, ConflictResolution how = CR_Manual) = 0;
    virtual bool        reset(text commit = "")         = 0;
    virtual bool        pull()                          = 0;
    virtual bool        push(QString pushUrl)           = 0;
    virtual bool        fetch(QString url)              = 0;
    virtual QStringList remotes()                       = 0;
    virtual QString     remoteFetchUrl(QString name)    = 0;
    virtual QString     remotePushUrl(QString name)    = 0;
    virtual bool        addRemote(QString name, QString pullUrl) = 0;
    virtual bool        setRemote(QString name, QString newPullUrl) = 0;
    virtual bool        delRemote(QString name)         = 0;
    virtual bool        renRemote(QString oldName, QString newName) = 0;
    virtual QList<Commit> history(QString branch = "", int max = 100) = 0;
    virtual Process *   asyncClone(QString cloneUrl, QString newFolder,
                              AnsiTextEdit *out = NULL, void *id = NULL) = 0;
    virtual text        version()                       = 0;
    virtual bool        isClean()                       = 0;
    virtual QString     url()                           = 0;
    virtual bool        gc()                            = 0;

public:
    static bool         versionGreaterOrEqual(QString ver, QString ref);

signals:
    void                commitSuccess(QString commitId, QString msg);
    void                asyncCloneComplete(void *id, QString projPath);
    void                asyncPullComplete();
    void                deleted();

protected:
    virtual QString     command()                       = 0;
    virtual text        styleSheet();
    virtual text        fullName(text fileName);
    Process *           dispatch(Process *cmd, AnsiTextEdit *err = NULL,
                                 AnsiTextEdit *out = NULL, void *id = NULL);
    void                waitForAsyncProcessCompletion();

protected slots:
    virtual void        asyncProcessFinished(int exitCode);
    virtual void        asyncProcessError(QProcess::ProcessError error);


protected:
    struct ProcQueueConsumer
    {
        ProcQueueConsumer(Repository &repo): repo(repo) {}
        ~ProcQueueConsumer();

        Repository &repo;
    };

public:
    QString            path;
    text               task;
    text               errors;
    QString            pullFrom;
    int                pullInterval;        // ms
    ConflictResolution conflictResolution;
    State              state;
    text               whatsNew;
    QString            lastPublishTo;
    QString            lastFetchUrl;
    text               cachedBranch;

protected:
    QQueue<Process *> pQueue;
};

#define TAO_UNDO_SUFFIX "_tao_undo"



// ============================================================================
//
//   Repository factory
//
// ============================================================================

typedef QSharedPointer<Repository> repository_ptr;

class RepositoryFactory
// ----------------------------------------------------------------------------
//   Create and cache repository instances
// ----------------------------------------------------------------------------
{
    friend class Repository;

public:
    enum Mode
    // ------------------------------------------------------------------------
    //   How to open a local repository
    // ------------------------------------------------------------------------
    {
        OpenExisting, // Fail if path is not a valid repository
        Create,       // Don't fail if path is not a valid repository
        Clone,        // Return a Repository instance for a given path that
                      // must NOT contain a valid repository -- caller will
                      // later invoke Repository::clone()
    };

public:
    RepositoryFactory() {}
    ~RepositoryFactory() {}

public:
    static bool             available();
    static repository_ptr   repository(QString path, Mode mode = OpenExisting);

protected:
    static Repository *     newRepository(QString path, Mode mode);
    static void             removeFromCache(QString path);

protected:
    static QMap<QString, QWeakPointer <Repository > > cache;
    static Repository::Kind                           availableScm;
};

}

// Make struct Commit useable as a QVariant
Q_DECLARE_METATYPE(Tao::Repository::Commit);

#endif // REPOSITORY_H
