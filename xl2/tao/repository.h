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
#include "process.h"
#include <QString>
#include <QProcess>
#include <QtGlobal>
#include <QMap>
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
    };

    // ------------------------------------------------------------------------
    //   Repository states
    // ------------------------------------------------------------------------
    enum State
    {
        RS_Clean,     // Work area reflects last commit
        RS_NotClean,  // Work area has been modified since last commit
    };

public:
    Repository(const QString &path): path(path), task("work"),
                                     state(RS_Clean) {}
    virtual ~Repository() {}

public:
    virtual bool        write(text fileName, XL::Tree *tree);
    virtual XL::Tree *  read(text fileName);
    virtual bool        setTask(text name);
    virtual bool        selectWorkBranch();
    virtual bool        selectUndoBranch();
    virtual bool        idle();

public:
    virtual QString     userVisibleName()               = 0;
    virtual bool        valid()                         = 0;
    virtual bool        initialize()                    = 0;
    virtual text        branch()                        = 0;
    virtual bool        checkout(text name)             = 0;
    virtual bool        branch(text name)               = 0;
    virtual bool        add(text name)                  = 0;
    virtual bool        change(text name)               = 0;
    virtual bool        rename(text from, text to)      = 0;
    virtual bool        commit(text msg,bool all=false) = 0;
    virtual bool        asyncCommit(text msg, bool all=false) = 0;
    virtual bool        merge(text branch)              = 0;
    virtual bool        reset()                         = 0;
    virtual bool        pull()                          = 0;
    virtual QStringList remotes()                       = 0;
    virtual QString     remotePullUrl(QString name)     = 0;
    virtual bool        addRemote(QString name, QString pullUrl) = 0;
    virtual bool        delRemote(QString name)         = 0;

public:
    static Repository * repository(const QString &path, bool create = false);
    static bool         available();

protected:
    virtual QString     command()                       = 0;
    virtual text        styleSheet();
    virtual text        fullName(text fileName);
    static Repository * newRepository(const QString &path,
                                      bool create = false);
    void                dispatch(Process *cmd);

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
    QString            pullUrl;
    ConflictResolution conflictResolution;
    State              state;

protected:
    QList<Process *> pQueue;

protected:
    static QMap<QString, Repository *> cache;
    static Kind                        availableScm;
};

#define TAO_UNDO_SUFFIX "_tao_undo"

}

#endif // REPOSITORY_H
