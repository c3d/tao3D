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
#include "renderer.h"
#include <QDir>
#include <QDebug>
#include <QtGlobal>
#include <iostream>

TAO_BEGIN

GitRepo::Status
GitRepo::SaveDocument(QString docName, XL::Tree *tree, QString msg)
// ------------------------------------------------------------------------
//   Save Tao document 'tree' in Git repository for 'docName'
// ------------------------------------------------------------------------
{
    if (!tree)
        return notSavedSaveError;
    QString repoPath = docName + ".git";
    qDebug() << Q_FUNC_INFO << "Saving document to Git repository" << repoPath;
    if (!Open(repoPath))
        return notSavedSaveError;
    GitBlobMaker toGit(*this);
    XL::Renderer render(toGit);
    render.SelectStyleSheet("git.stylesheet");
    render.Render(tree);
    Status status = CreateCommit(CreateTopDir(toGit.ReadSha1()), msg);
    qDebug() << Q_FUNC_INFO << "done";
    return status;
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

GitRepo::Status GitRepo::CreateCommit(QString sha1, QString commitMessage)
// ------------------------------------------------------------------------
//   Create a commit for tree ID sha1 and return SHA1 of commit.
// ------------------------------------------------------------------------
{
    qDebug() << Q_FUNC_INFO << "Create commit (if needed) for tree ID:"
             << sha1;

    bool hasMaster = false;
    if (QFile(curPath + "/.git/refs/heads/master").exists())
    {
        GitProcess git(*this);
        git.start(QStringList() << "show" << "--pretty=format:%H%T"
                  << "master");
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
            return notSavedNoChange;
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
    return savedNewVersionCreated;
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

QString GitRepo::CreateTopDir(QString docSha1)
// ------------------------------------------------------------------------
//   Create a Git tree containing the specified document and return its ID
// ------------------------------------------------------------------------
{
    QString ret =  MakeTree(QString("100644 blob %1\tdoc\n").arg(docSha1));
    qDebug() << Q_FUNC_INFO << "Git tree for top dir created, ID:" << ret;
    return ret;
}

QString GitRepo::MakeTree(QString treeSpec)
// ------------------------------------------------------------------------
//   Create a Git tree-object from ls-tree formatted text and return its ID
// ------------------------------------------------------------------------
{
    GitProcess git(*this);
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

GitBlobMakerStreamBuf::GitBlobMakerStreamBuf(GitRepo &repo, size_t bufSize)
        : repo(repo), git(NULL)
{
    char *ptr = new char[bufSize];
    setp(ptr, ptr + bufSize);
}

GitBlobMakerStreamBuf::~GitBlobMakerStreamBuf()
{
    sync();
    delete[] pbase();
    if (git)
        delete git;
}

void GitBlobMakerStreamBuf::PipeToProcess(std::string str)
// ------------------------------------------------------------------------
//   Pipe a string to the standard input of the Git subprocess
// ------------------------------------------------------------------------
{
    if (!git)
    {
        qDebug() << Q_FUNC_INFO << "Starting Git subprocess";
        git = new GitProcess(repo);
        git->start(QStringList() << "hash-object" << "-w" << "--stdin");
        if (!git->waitForStarted())
            Q_ASSERT(!"Problem starting Git subprocess");
    }
    git->write(str.c_str());
}

int GitBlobMakerStreamBuf::overflow(int c)
// ------------------------------------------------------------------------
//   Overriding std::streambuf::overflow()
// ------------------------------------------------------------------------
{
    sync();
    if (c != EOF)
    {
        if (pbase() == epptr())
            PipeToProcess("" + char(c));
        else
            sputc(c);
    }
    return 0;
}

int GitBlobMakerStreamBuf::sync()
// ------------------------------------------------------------------------
//   Overriding std::streambuf::sync()
// ------------------------------------------------------------------------
{
    if (pbase() != pptr())
    {
        PipeToProcess(std::string(pbase(), int(pptr() - pbase())));
        setp(pbase(), epptr());
    }
    return 0;
}

QString GitBlobMaker::ReadSha1()
// ------------------------------------------------------------------------
//   Flush pending output to Git subprocess and read blob's SHA1
// ------------------------------------------------------------------------
{
    flush();
    Q_ASSERT(sb.git);
    sb.sync();
    sb.git->closeWriteChannel();
    sb.git->waitForReadyRead(-1);
    QString ret = QString(sb.git->readLine());
    ret.resize(40);
    sb.git->waitForFinished();
    qDebug() << Q_FUNC_INFO << "Git blob for document created, ID:" << ret;
    return ret;
}

TAO_END
