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
#include <iostream>

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
    QString  CreateTopDir(QString docSha1);
    QString  MakeTree(QString treeSpec);

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

struct GitBlobMakerStreamBuf : std::streambuf
// ------------------------------------------------------------------------
//   Utility class to interface GitBlobMaker with a Git process
// ------------------------------------------------------------------------
{
    GitBlobMakerStreamBuf(GitRepo &repo, size_t bufSize = 1024);
    virtual ~GitBlobMakerStreamBuf();

    void PipeToProcess(std::string str);

    GitRepo    & repo;
    GitProcess * git;

    int overflow(int c);
    int sync();
};

struct GitBlobMaker : std::ostream
// ------------------------------------------------------------------------
//   Store data in a Git repository (as a Git blob) and return blob ID
// ------------------------------------------------------------------------
{
    GitBlobMaker(GitRepo &repo):
        std::ios(0), std::ostream(&sb), repo(repo), sb(repo) {}
    virtual ~GitBlobMaker() {}
    QString  ReadSha1();

protected:
    GitRepo &  repo;

private:
    GitBlobMakerStreamBuf sb;
};

TAO_END

#endif // GIT_BACKEND_H
