#ifndef GIT_BACKEND_H
#define GIT_BACKEND_H
// ****************************************************************************
//  git_backend.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Storing Tao documents in a Git repository.
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
//  (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tree.h"
#include "repository.h"
#include "process.h"

#include <QString>
#include <QProcess>
#include <QtGlobal>
#include <iostream>

namespace Tao {

class GitRepository : public QObject, public Repository
// ----------------------------------------------------------------------------
//   A Git repository
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    GitRepository(const QString &path): Repository(path) { }
    virtual ~GitRepository() {}

public:
    // Redefine Repository virtual functions
    virtual bool        valid();
    virtual bool        initialize();
    virtual text        branch();
    virtual bool        checkout(text branch);
    virtual bool        branch(text name);
    virtual bool        add(text name);
    virtual bool        change(text name);
    virtual bool        rename(text from, text to);
    virtual bool        commit(text message, bool all=false);
    virtual bool        asyncCommit(text message, bool all=false);
    virtual bool        merge(text branch);
    virtual bool        reset();

    static  bool        checkGit();

protected:
    virtual QString     command();
    virtual QString     userVisibleName();
    virtual text        styleSheet();

protected:
    struct ProcQueueConsumer
    {
        ProcQueueConsumer(GitRepository &repo): repo(repo) {}
        ~ProcQueueConsumer();

        GitRepository &repo;
    };

private:
    bool                initialCommit();
    void                dispatch(Process *cmd);

    static QString      gitCommand;
    QList<Process *>    pQueue;

private slots:
    void                asyncProcessFinished(int exitCode);
    void                asyncProcessError(QProcess::ProcessError error);
};

}

#endif // GIT_BACKEND_H
