#ifndef GIT_BACKEND_H
#define GIT_BACKEND_H

// ****************************************************************************
//  git_backend.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Storing a Tao document in a Git repository.
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

#include "tao.h"
#include "tree.h"
#include <QString>
#include <QProcess>
#include <QtGlobal>

TAO_BEGIN

struct GitRepo
// ------------------------------------------------------------------------
//   A Git repository
// ------------------------------------------------------------------------
{
    GitRepo() {}
    virtual ~GitRepo() {}

    bool SaveDocument(QString docName, XL::Tree *tree, QString msg = "");

protected:
    bool     Open(QString path);
    bool     Init(QString path);
    QString  CreateCommit(QString sha1, QString commitMessage = "");
    bool     UpdateRef(QString ref, QString sha1);

public:
    QString  curPath;
};

struct GitProcess : QProcess
// ------------------------------------------------------------------------
//   Run a git command
// ------------------------------------------------------------------------
{
    GitProcess(GitRepo &repo);
    virtual ~GitProcess() {}

    void start(const QStringList & arguments);
    void SetGitEnvironmentForCommit();
};

struct GitCreateObjects : XL::Action
// ------------------------------------------------------------------------
//   Store whole XL tree in Git repository (as Git blobs and trees)
// ------------------------------------------------------------------------
{
    GitCreateObjects(GitRepo &repo): repo(repo) {}
    virtual ~GitCreateObjects() {}

    XL::Tree *DoInteger(XL::Integer *what);
    XL::Tree *DoReal(XL::Real *what);
    XL::Tree *DoText(XL::Text *what);
    XL::Tree *DoName(XL::Name *what);
    XL::Tree *DoBlock(XL::Block *what);
    XL::Tree *DoInfix(XL::Infix *what);
    XL::Tree *DoPrefix(XL::Prefix *what);
    XL::Tree *DoPostfix(XL::Postfix *what);
    XL::Tree *Do(XL::Tree *what) { Q_ASSERT(!"Unexpected node kind"); }

    QString    sha1;    // Git hash

protected:

    QString MakeBlob(const QByteArray &data);
    QString MakeTree(QString treeSpec);

    GitRepo &  repo;    // The Git repository
};

TAO_END

#endif // GIT_BACKEND_H
