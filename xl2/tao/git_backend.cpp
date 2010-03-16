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
#include <QtGlobal>
#include <iostream>

TAO_BEGIN

GitRepo::Status
GitRepo::SaveDocument(XL::Tree *tree, QString msg)
// ------------------------------------------------------------------------
//   Save Tao document 'tree' in Git repository for 'docName'
// ------------------------------------------------------------------------
{
    if (!Open())
        return notSavedSaveError;
    if (!tree)
        return notSavedNullTree;
    GitBlobMaker toGit(*this);
    XL::Renderer render(toGit);
    render.SelectStyleSheet("git.stylesheet");
    render.Render(tree);
    return CreateCommit(CreateTopDir(toGit.ReadSha1()), msg);
}

bool GitRepo::Init()
// ------------------------------------------------------------------------
//   Initialize a Git repository
// ------------------------------------------------------------------------
{
    if (!QDir().mkpath(curPath))
        return false;
    GitProcess git(*this);
    git.start(QStringList() << "init" /*<< "--bare"*/);
    if (!git.waitForStarted()  ||
        !git.waitForFinished() ||
         git.exitStatus()      != QProcess::NormalExit)
        return false;
    isOpen = true;
    return true;
}

bool GitRepo::Open()
// ------------------------------------------------------------------------
//   Make sure repoitory is available (initialize it if needed)
// ------------------------------------------------------------------------
{
    if (isOpen)
        return true;
    if (QDir(curPath).isReadable())
        return true;
    return Init();
}

GitRepo::Status GitRepo::CreateCommit(QString sha1, QString commitMessage)
// ------------------------------------------------------------------------
//   Create a commit for tree ID sha1 and return SHA1 of commit.
// ------------------------------------------------------------------------
{
    bool hasMaster = false;
    if (QFile(curPath + "/.git/refs/heads/master").exists())
    {
        GitProcess git(*this);
        git.start(QStringList() << "show" << "--pretty=format:%H%T"
                  << "master");
        git.closeWriteChannel();
        git.waitForReadyRead(-1);
        QString masterCommitSha1 = QString(git.readLine(41)); // REVISIT unused
        QString masterTreeSha1   = QString(git.readLine(41));
        git.waitForFinished();

        if (masterTreeSha1 == sha1)
            return notSavedNoChange;

        GitProcess git1(*this);
        git1.start(QStringList() << "read-tree" << "HEAD");
        git1.closeWriteChannel();
        git1.waitForFinished();
        hasMaster = true;
    }

    GitProcess git2(*this);
    git2.start(QStringList() << "read-tree" << "-i" << "-m" << sha1);
    git2.closeWriteChannel();
    git2.waitForFinished();

    GitProcess git3(*this);
    git3.start(QStringList() << "write-tree");
    git3.closeWriteChannel();
    git3.waitForReadyRead(-1);
    QString msha1 = QString(git3.readLine());
    msha1.resize(40);
    git3.waitForFinished();

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

    UpdateRef("refs/heads/master", ret);
    return savedNewVersionCreated;
}

bool GitRepo::UpdateRef(QString ref, QString sha1)
// ------------------------------------------------------------------------
//   Update a branch ref to point to a specific commit
// ------------------------------------------------------------------------
{
    GitProcess git(*this);
    git.start(QStringList() << "update-ref" << ref << sha1);
    git.closeWriteChannel();
    git.waitForFinished();
    return true;
}

void GitRepo::CheckoutDocument(const QString & docName)
// ------------------------------------------------------------------------
//   Checkout 'docname'.git/main as 'docName'
// ------------------------------------------------------------------------
{
    if (!Open())
        return;
    GitProcess git(*this);
    git.start(QStringList() << "checkout" << "--" << "main");
    git.waitForFinished();
    QFile docFile(curPath + "/main");
    docFile.copy(docName);
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
    return MakeTree(QString("100644 blob %1\tmain\n").arg(docSha1));
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
    return ret;
}

TAO_END
