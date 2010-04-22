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
#include "ansi_textedit.h"

#include <QString>
#include <QProcess>
#include <QtGlobal>
#include <iostream>

namespace Tao {

class GitRepository : public Repository
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
    virtual bool        commit(text message = "", bool all=false);
    virtual bool        asyncCommit(text message = "", bool all=false);
    virtual bool        asyncRevert(text id);
    virtual bool        asyncCherryPick(text id);
    virtual bool        merge(text branch);
    virtual bool        reset();
    virtual bool        pull();
    virtual bool        push(QString pushUrl);
    virtual QStringList remotes();
    virtual QString     remotePullUrl(QString name);
    virtual bool        addRemote(QString name, QString pullUrl);
    virtual bool        setRemote(QString name, QString newPullUrl);
    virtual bool        delRemote(QString name);
    virtual bool        renRemote(QString oldName, QString newName);
    virtual QList<Commit> history(int max = 20);
    virtual Process *   asyncClone(QString cloneUrl, QString path,
                                   AnsiTextEdit *out = NULL, void *id = NULL);

    static  bool        checkGit();

protected:
    virtual QString     command();
    virtual QString     userVisibleName();
    virtual text        styleSheet();

protected slots:
    virtual void        asyncProcessFinished(int exitCode);

private:
    bool                initialCommit();
    bool                parseCommitOutput(text output, QString &id,
                                          QString &msg);

    static QString      gitCommand;
};

}

#endif // GIT_BACKEND_H
