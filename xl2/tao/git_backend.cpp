// ****************************************************************************
//  git_backend.cpp                                                Tao project
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

#include "git_backend.h"
#include <QDir>
#include <QDebug>
#include <QtGlobal>

TAO_BEGIN

bool GitRepo::SaveDocument(QString docName, XL::Tree *tree, QString msg)
// ------------------------------------------------------------------------
//   Save Tao document 'tree' in Git repository for 'docName'
// ------------------------------------------------------------------------
{
    if (!tree)
        return false;
    QString repoPath = docName + ".git";
    bool status;
    qDebug() << Q_FUNC_INFO << "Saving document to Git repository" << repoPath;
    if (!Open(repoPath))
        return false;
    GitCreateObjects toGit(*this);
    tree->Do(toGit);
    CreateCommit(toGit.sha1, msg);
    qDebug() << Q_FUNC_INFO << "done";
    return true;
}

bool GitRepo::Init(QString path)
// ------------------------------------------------------------------------
//   Initialize a Git repository
// ------------------------------------------------------------------------
{
    qDebug() << Q_FUNC_INFO << "Initializing GIT repository" << path;
    if (!QDir().mkpath(path))
        return false;
    curPath = path;
    GitProcess git(*this);
    git.start(QStringList() << "init" /*<< "--bare"*/);
    if (!git.waitForStarted()  ||
        !git.waitForFinished() ||
         git.exitStatus()      != QProcess::NormalExit)
        return false;
    qDebug() << Q_FUNC_INFO << "GIT repository initialized";
    return true;
}

bool GitRepo::Open(QString path)
// ------------------------------------------------------------------------
//   Make sure repoitory is available (initialize it if needed)
// ------------------------------------------------------------------------
{
    if (curPath == path)
        return true;
    if (QDir(path).isReadable())
    {
        curPath = path;
        return true;
    }
    return Init(path);
}

QString GitRepo::CreateCommit(QString sha1, QString commitMessage)
// ------------------------------------------------------------------------
//   Create a commit for tree ID sha1 and return SHA1 of commit.
// ------------------------------------------------------------------------
{
    qDebug() << Q_FUNC_INFO << "Tree ID:" << sha1;

    bool hasMaster = false;
    if (QFile(curPath + "/.git/refs/heads/master").exists())
    {
        GitProcess git(*this);
        git.start(QStringList() << "show" << "--pretty=format:%H%T" << "master");
        git.closeWriteChannel();
        git.waitForReadyRead(-1);
        QString masterCommitSha1 = QString(git.readLine(41));
        QString masterTreeSha1   = QString(git.readLine(41));
        qDebug() << "master tree:" << masterTreeSha1
                 << "master commit:" << masterCommitSha1;
        git.waitForFinished();

        if (masterTreeSha1 == sha1)
        {
            qDebug() << Q_FUNC_INFO << "No need to create commit (no change)";
            return masterCommitSha1;  // FIXME should return commit ID not tree ID
        }

        qDebug() << "Loading HEAD into index";
        GitProcess git1(*this);
        git1.start(QStringList() << "read-tree" << "HEAD");
        git1.closeWriteChannel();
        git1.waitForFinished();
        hasMaster = true;
    }

    qDebug() << "Merging tree into index";
    GitProcess git2(*this);
    git2.start(QStringList() << "read-tree" << "-i" << "-m" << sha1);
    git2.closeWriteChannel();
    git2.waitForFinished();

    qDebug() << "Writing tree from index";
    GitProcess git3(*this);
    git3.start(QStringList() << "write-tree");
    git3.closeWriteChannel();
    git3.waitForReadyRead(-1);
    QString msha1 = QString(git3.readLine());
    msha1.resize(40);
    git3.waitForFinished();
    qDebug() << "Merged tree ID:" << msha1 << "- committing...";

    QStringList args;
    args << "commit-tree" << msha1;
    if (hasMaster)
        args << "-p" << "master";
    GitProcess git4(*this);
    git4.SetGitEnvironmentForCommit();
    git4.start(args);
    git4.write(commitMessage.toAscii());
    git4.closeWriteChannel();
    git4.waitForReadyRead(-1);
    QString ret = QString(git4.readLine());
    ret.resize(40);
    git4.waitForFinished();
    qDebug() << Q_FUNC_INFO << "New commit ID:" << ret;

    UpdateRef("refs/heads/master", ret);
    return ret;
}

bool GitRepo::UpdateRef(QString ref, QString sha1)
// ------------------------------------------------------------------------
//   Update a branch ref to point to a specific commit
// ------------------------------------------------------------------------
{
    qDebug() << Q_FUNC_INFO << ref << "->" << sha1;
    GitProcess git(*this);
    git.start(QStringList() << "update-ref" << ref << sha1);
    git.closeWriteChannel();
    git.waitForFinished();
    return true;
}

GitProcess::GitProcess(GitRepo &repo): QProcess()
{
    setWorkingDirectory(repo.curPath);
}

void GitProcess::SetGitEnvironmentForCommit()
// ------------------------------------------------------------------------
//   Set Git environment variables that have an influence on commit
// ------------------------------------------------------------------------
{
    // TODO
}

void GitProcess::start(const QStringList &arguments)
// ------------------------------------------------------------------------
//   Run git with the supplied arguments
// ------------------------------------------------------------------------
{
    QProcess::start("git", arguments);
}

XL::Tree *GitCreateObjects::DoInteger(XL::Integer *what)
{
    char value[50];
    size_t size = snprintf(value, sizeof(value), "integer %lld", what->value);
    QByteArray data(value, size);
    sha1 = MakeBlob(data);
    return what;
}

XL::Tree *GitCreateObjects::DoReal(XL::Real *what)
{
    char value[50];
    size_t size = snprintf(value, sizeof(value), "real %g", what->value);
    QByteArray data(value, size);
    sha1 = MakeBlob(data);
    return what;
}

XL::Tree *GitCreateObjects::DoText(XL::Text *what)
{
    QByteArray data("text ");
    data.append(what->value.data(), what->value.length());
    sha1 = MakeBlob(data);
    return what;
}

XL::Tree *GitCreateObjects::DoName(XL::Name *what)
{
    QByteArray data("name ");
    data.append(what->value.data(), what->value.length());
    sha1 = MakeBlob(data);
    return what;
}

XL::Tree *GitCreateObjects::DoBlock(XL::Block *what)
{
    bool       childIsLeaf;
    QString    sha1Opening, sha1Closing, sha1Child;
    QString    treeSpec;
    QByteArray data;

    data.append(what->opening.data(), what->opening.size());
    sha1Opening = MakeBlob(data);
    data.clear();
    data.append(what->closing.data(), what->closing.size());
    sha1Closing = MakeBlob(data);
    childIsLeaf = what->child->Do(this);
    sha1Child   = sha1;

    treeSpec = QString("100644 blob %1\topening\n"
                       "100644 blob %2\tclosing\n"
                                "%3 %4\tchild\n")
                       .arg(sha1Opening)
                       .arg(sha1Closing)
                       .arg(childIsLeaf  ? "100644 blob"
                                         : "040000 tree")
                       .arg(sha1Child);
    sha1 = MakeTree(treeSpec);
    return NULL;
}

XL::Tree *GitCreateObjects::DoInfix(XL::Infix *what)
{
    bool       leftIsLeaf, rightIsLeaf;
    QString    sha1Infix, sha1Left, sha1Right;
    QString    treeSpec;
    QByteArray data;

    data.append(what->name.data(), what->name.size());
    sha1Infix   = MakeBlob(data);
    leftIsLeaf  = what->left->Do(this);
    sha1Left    = sha1;
    rightIsLeaf = what->right->Do(this);
    sha1Right   = sha1;

    treeSpec = QString("100644 blob %1\tinfix\n"
                                "%2 %3\tleft\n"
                                "%4 %5\tright\n")
                       .arg(sha1Infix)
                       .arg(leftIsLeaf  ? "100644 blob"
                                        : "040000 tree")
                       .arg(sha1Left)
                       .arg(rightIsLeaf ? "100644 blob"
                                        : "040000 tree")
                       .arg(sha1Right);
    sha1 = MakeTree(treeSpec);
    return NULL;
}

XL::Tree *GitCreateObjects::DoPrefix(XL::Prefix *what)
{
    bool       leftIsLeaf, rightIsLeaf;
    QString    sha1Prefix, sha1Left, sha1Right;
    QString    treeSpec;
    QByteArray data;

    data.append("", 0);
    sha1Prefix  = MakeBlob(data);
    leftIsLeaf  = what->left->Do(this);
    sha1Left    = sha1;
    rightIsLeaf = what->right->Do(this);
    sha1Right   = sha1;

    treeSpec = QString("100644 blob %1\tprefix\n"
                                "%2 %3\tleft\n"
                                "%4 %5\tright\n")
                       .arg(sha1Prefix)
                       .arg(leftIsLeaf  ? "100644 blob"
                                        : "040000 tree")
                       .arg(sha1Left)
                       .arg(rightIsLeaf ? "100644 blob"
                                        : "040000 tree")
                       .arg(sha1Right);
    sha1 = MakeTree(treeSpec);
    return NULL;
}

XL::Tree *GitCreateObjects::DoPostfix(XL::Postfix *what)
{
    bool       leftIsLeaf, rightIsLeaf;
    QString    sha1Postfix, sha1Left, sha1Right;
    QString    treeSpec;
    QByteArray data;

    data.append("", 0);
    sha1Postfix = MakeBlob(data);
    leftIsLeaf  = what->left->Do(this);
    sha1Left    = sha1;
    rightIsLeaf = what->right->Do(this);
    sha1Right   = sha1;

    treeSpec = QString("100644 blob %1\tpostfix\n"
                                "%2 %3\tleft\n"
                                "%4 %5\tright\n")
                       .arg(sha1Postfix)
                       .arg(leftIsLeaf  ? "100644 blob"
                                        : "040000 tree")
                       .arg(sha1Left)
                       .arg(rightIsLeaf ? "100644 blob"
                                        : "040000 tree")
                       .arg(sha1Right);
    sha1 = MakeTree(treeSpec);
    return NULL;
}

QString GitCreateObjects::MakeBlob(const QByteArray &data)
// ------------------------------------------------------------------------
//   Create a Git blob from supplied buffer and return its ID
// ------------------------------------------------------------------------
{
    GitProcess git(repo);
    git.start(QStringList() << "hash-object" << "-w" << "--stdin");
    if (!git.waitForStarted())
        return false;
    git.write(data);
    git.closeWriteChannel();
    git.waitForReadyRead(-1);
    QString ret = QString(git.readLine());
    ret.resize(40);
    git.waitForFinished();
    return ret;
}

QString GitCreateObjects::MakeTree(QString treeSpec)
// ------------------------------------------------------------------------
//   Create a Git tree-object from ls-tree formatted text and return its ID
// ------------------------------------------------------------------------
{
    GitProcess git(repo);
    git.start(QStringList() << "mktree");
    if (!git.waitForStarted())
        return false;
    git.write(treeSpec.toAscii());
    git.closeWriteChannel();
    git.waitForReadyRead(-1);
    QString ret = QString(git.readLine());
    ret.resize(40);
    git.waitForFinished();
    return ret;
}

TAO_END
