// ****************************************************************************
//  git_backend.cpp                                                Tao project
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "git_backend.h"
#include "renderer.h"
#include "options.h"
#include "tao_utf8.h"
#include <QDir>
#include <QString>
#include <QtGlobal>
#include <QApplication>
#include <QRegExp>
#include <QHostInfo>
#include <iostream>

TAO_BEGIN

// The 'git' command. May be updated by checkGit().
QString GitRepository::gitCommand("git");


GitRepository::GitRepository(const QString &path)
// ----------------------------------------------------------------------------
//   GitRepository constructor
// ----------------------------------------------------------------------------
    : Repository(path), currentBranchMtime(0)
{
    connect(&cdvTimer, SIGNAL(timeout()), this, SLOT(clearCachedDocVersion()));
    cdvTimer.start(5000);
}


bool GitRepository::checkGit()
// ----------------------------------------------------------------------------
//   Return true if Git is functional, and set the git command accordingly
// ----------------------------------------------------------------------------
{
    // Look for "git" in $PATH, then in <application's directory>/git/bin
    QStringList commands;
    commands << "git" << qApp->applicationDirPath() + "/git/bin/git";
    QStringListIterator it(commands);
    while (it.hasNext())
    {
        text errors, output;
        gitCommand = it.next();
        Process cmd(gitCommand, QStringList("--version"));
        if (cmd.done(&errors, &output))
        {
            // Check version is at least 1.7.0
            // (we need the merge -Xours / -Xtheirs feature)
            QString ver = (+output).replace(QRegExp("[^\\d\\.]"), "");
            if (versionGreaterOrEqual(ver, "1.7.0"))
                return true;
        }
    }
    return false;
}


QString GitRepository::command()
// ----------------------------------------------------------------------------
//   Return the command for 'git'
// ----------------------------------------------------------------------------
{
    return gitCommand;
}


QString GitRepository::userVisibleName()
// ----------------------------------------------------------------------------
//   Return the user visible name for the kind of repository
// ----------------------------------------------------------------------------
{
    return "Git";
}


text GitRepository::styleSheet()
// ----------------------------------------------------------------------------
//   Return the XL style sheet to be used for rendering files in git
// ----------------------------------------------------------------------------
{
    return "system:git.stylesheet";
}


bool GitRepository::valid()
// ----------------------------------------------------------------------------
//   Check if the repository exists and is a valid git repository
// ----------------------------------------------------------------------------
//   We simply use 'git branch' and check for errors
//   First, I tried with 'git status', but the return code is 1 after init
{
    if (!QDir(path).exists())
        return false;
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("branch"), path);
    return cmd.done(&errors);
}


bool GitRepository::initialize()
// ----------------------------------------------------------------------------
//    Initialize a git repository if we need to
// ----------------------------------------------------------------------------
{
    if (!QDir(path).mkpath("."))
        return false;
    Process cmd(command(), QStringList("init"), path);
    if (cmd.done(&errors))
        return initialCommit();
    return false;
}


bool GitRepository::initialCommit()
// ----------------------------------------------------------------------------
//    Commit an empty file. Required after init or "git branch" fails
// ----------------------------------------------------------------------------
{
    QFile dummy(path + "/.tao");
    if (!dummy.open(QIODevice::WriteOnly))
        return false;
    dummy.close();
    Process cmd(command(), QStringList("add") << ".tao", path);
    if (!cmd.done(&errors))
        return false;
    Process cmd2(command(), QStringList("commit") << "-a" << "-m"
                 << "Automatic initial commit", path);
    return cmd.done(&errors);
}


text GitRepository::branch()
// ----------------------------------------------------------------------------
//    Return the name of the current branch
// ----------------------------------------------------------------------------
{
    text    output, result;
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("branch"), path);
    bool    ok = cmd.done(&errors, &output);
    if (ok)
    {
        QStringList branches = (+output).split("\n");
        QRegExp re("^[*].*");
        int index = branches.indexOf(re);
        if (index >= 0)
            result = +branches[index].mid(2);
        else
            result = "master";  // May happen on a totally empty repository
    }
    if (result == "(no branch)")
        result = "";
    return result;
}


QStringList GitRepository::branches()
// ----------------------------------------------------------------------------
//    Return the names of all known branches (local and remote)
// ----------------------------------------------------------------------------
{
    text    output;
    QStringList result;
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("branch") << "-a", path);
    bool    ok = cmd.done(&errors, &output);
    if (ok)
    {
        QStringList branches = (+output).split("\n");
        foreach (QString branch, branches)
        {
            branch = branch.mid(2);
            if (branch == "(no branch)")
                continue;
            if (branch.contains(" -> "))
                continue;
            result << branch;
        }
    }
    return result;
}


bool GitRepository::addBranch(QString name, bool force)
// ----------------------------------------------------------------------------
//    Create a new branch, starting from latest commit on current branch
// ----------------------------------------------------------------------------
//    If force == false, Git will refuse to change an existing branch
{
    waitForAsyncProcessCompletion();
    QStringList args("branch");
    if (force)
        args << "-f";
    args << name;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::delBranch(QString name, bool force)
// ----------------------------------------------------------------------------
//    Delete a branch
// ----------------------------------------------------------------------------
//    If force == false, the branch to delete must be fully merged into
//    HEAD of the current branch
{
    waitForAsyncProcessCompletion();
    QStringList args("branch");
    if (force)
        args << "-D";
    else
        args << "-d";
    if (isRemoteBranch(+name))
    {
        args << "-r";
        // Strip "remotes/" prefix
        name = name.mid(8);
    }
    args << name;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::renBranch(QString oldName, QString newName, bool force)
// ----------------------------------------------------------------------------
//    Rename a branch
// ----------------------------------------------------------------------------
//    If force == true, rename event if the new branch name already exists
{
    waitForAsyncProcessCompletion();
    QStringList args("branch");
    if (force)
        args << "-M";
    else
        args << "-m";
    args << oldName << newName;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::isRemoteBranch(text branch)
// ----------------------------------------------------------------------------
//    Return true if branch is remote, false otherwise
// ----------------------------------------------------------------------------
{
    return (+branch).startsWith("remotes/");
}


bool GitRepository::checkout(text branch)
// ----------------------------------------------------------------------------
//    Checkout a given branch
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("checkout") << +branch, path);
    return cmd.done(&errors);
}


bool GitRepository::branch(text name)
// ----------------------------------------------------------------------------
//    Create a new branch
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("branch") << +name, path);
    bool result = cmd.done(&errors);
    if (!result)
        if (errors.find("already exists") != errors.npos)
            result = true;
    return result;
}


bool GitRepository::add(text name)
// ----------------------------------------------------------------------------
//   Add a new file to the repository. If name == "", add all untracked filed.
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    QStringList args("add");
    if (name.empty())
        args << "--all";
    else
        args << +name;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::change(text name)
// ----------------------------------------------------------------------------
//   Signal that a file in the repository changed
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    mergeCommitMessages(nextCommitMessage, whatsNew);
    whatsNew = "";
    Process cmd(command(), QStringList("add") << +name, path);
    return cmd.done(&errors);
}


bool GitRepository::remove(text name)
// ----------------------------------------------------------------------------
//   Remove a file from the repository
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    mergeCommitMessages(nextCommitMessage, whatsNew);
    whatsNew = "";
    Process cmd(command(), QStringList("rm") << +name, path);
    return cmd.done(&errors);
}


bool GitRepository::rename(text from, text to)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("mv") << +from << +to, path);
    return cmd.done(&errors);
}


bool GitRepository::commit(text message, bool all)
// ----------------------------------------------------------------------------
//   Record pending changes to the repository
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    if (message == "")
    {
        message = nextCommitMessage;
        nextCommitMessage = "";
    }
    if (all)
        add("");
    QStringList args("commit");
    if (all)
        args << "-a";
    args << "-m" << +message;
    Process cmd(command(), args, path);
    text output;
    bool result = cmd.done(&errors, &output);
    if (!result)
        if (errors.find("nothing added") != errors.npos ||
            errors.find("nothing to commit") != errors.npos)
            result = true;
    state = RS_Clean;
    if (result)
    {
        QString commitId, commitMsg;
        if (parseCommitOutput(output, commitId, commitMsg))
            emit commitSuccess(commitId, commitMsg);
    }
    return result;
}


bool GitRepository::revert(text id)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    if (state != RS_Clean)
    {
        commit();
        state = RS_Clean;    // Avoid recursion
        revert("HEAD");
    }

    QStringList args("revert");
    args << "--no-edit" << +id;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::cherryPick(text id)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    if (state != RS_Clean)
    {
        commit();
        state = RS_Clean;    // Avoid recursion
        revert("HEAD");
    }

    QStringList args("cherry-pick");
    args << "-x" << +id;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


void GitRepository::computePercentComplete()
// ----------------------------------------------------------------------------
//   Extract % complete from the text output of a clone or fetch process
// ----------------------------------------------------------------------------
{
    Process *cmd = (Process *)sender();
    int &pos = cmd->errPos;
    bool matched = false;
    int percent = -1;
    do
    {
        int p1 = 5, p2 = 94, p3 = 1;
        QString str = cmd->err.mid(pos);
        QRegExp rx1("Compressing objects: +([0-9]*)%");
        QRegExp rx2("Receiving objects: +([0-9]*)%");
        QRegExp rx3("Resolving deltas: +([0-9]*)%");
        matched = true;
        if (rx1.indexIn(str) != -1)
        {
            percent = rx1.cap(1).toInt() * p1 / 100;
            pos += rx1.matchedLength();
        }
        else
        if (rx2.indexIn(str) != -1)
        {
            percent = p1 + rx2.cap(1).toInt() * p2 / 100;
            pos += rx2.matchedLength();
        }
        else
        if (rx3.indexIn(str) != -1)
        {
            percent = p1 + p2 + rx3.cap(1).toInt() * p3 / 100;
            pos += rx3.matchedLength();
        }
        else
        {
            matched = false;
        }
    }
    while (matched);

    if (percent == -1 || percent == cmd->percent)
        return;

    IFTRACE(process)
        std::cerr << "Process " << cmd->num << ": " << percent << "% complete\n";
    cmd->percent = percent;
    emit percentComplete(percent);
}


void GitRepository::asyncProcessFinished(int exitCode, QProcess::ExitStatus st)
// ----------------------------------------------------------------------------
//   An asynchronous subprocess has finished
// ----------------------------------------------------------------------------
{
    (void)exitCode; (void)st;
    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.first());
    QString op = cmd->args.first();
    if (op == "clone")
    {
        QString projPath;
        projPath = parseCloneOutput(cmd->out);
        emit asyncCloneComplete(cmd->id, projPath);
    }
    else if (op == "pull")
    {
        if (!cmd->out.contains("Already up-to-date"))
            emit asyncPullComplete();
        delete cmd;  // REVISIT inconsistency: deleting only in this case
    }
    else if (op == "fetch")
    {
        emit asyncFetchComplete(cmd->id);
    }
}


void GitRepository::clearCachedDocVersion()
// ----------------------------------------------------------------------------
//   Forget cached document version
// ----------------------------------------------------------------------------
{
    cachedDocVersion = "";
}


void GitRepository::checkCurrentBranch()
// ----------------------------------------------------------------------------
//   Emit signal if current branch has changed since last call
// ----------------------------------------------------------------------------
{
    QString head = path + "/.git/HEAD";
    struct stat st;
    if (stat((+head).c_str(), &st) < 0)
        return;
    if (currentBranchMtime == 0)
        currentBranchMtime = st.st_mtime;
    if (st.st_mtime <= currentBranchMtime)
        return;
    currentBranchMtime = st.st_mtime;
    emit branchChanged(+branch());
}


bool GitRepository::parseCommitOutput(text output, QString &id, QString &msg)
// ----------------------------------------------------------------------------
//   Extract the commit ID and the commit message from "git commit" output
// ----------------------------------------------------------------------------
{
    // Commit output is like:
    // [master 35a1376] Shape moved
    //  1 files changed, 1 insertions(+), 1 deletions(-)
    QRegExp rx("\\[[^ ]+ ([0-9a-f]+)\\] ([^\r\n]+)");

    if (rx.indexIn(+output) == -1)
        return false;
    id  = rx.cap(1);
    msg = rx.cap(2);
    return true;
}



QString GitRepository::parseCloneOutput(QString output)
// ----------------------------------------------------------------------------
//   Extract the project path from "git clone" standard output
// ----------------------------------------------------------------------------
{
    // Clone output is like:
    // Initialized empty Git repository in /Some/Path/Project/.git/
    QString path;
    output.replace("/.git/", "");
    int i;
    i = output.indexOf("/");
    if (i != -1)
        path = output.remove(0, i);
    return path.trimmed();
}


void GitRepository::mergeCommitMessages(text &dest, text src)
// ----------------------------------------------------------------------------
//   Add to 'dest' all lines from 'src' not already present in 'dest'
// ----------------------------------------------------------------------------
{
    QStringList d = (+dest).split("\n");
    QStringList s = (+src).split("\n");

    foreach (QString line, s)
        if (!d.contains(line))
            d.append(s);

    QString res = d.join("\n");
    dest = +res;
}


QStringList GitRepository::crArgs(ConflictResolution mode)
// ----------------------------------------------------------------------------
//   Build the git command line arguments for conflict resolution mode 'mode'
// ----------------------------------------------------------------------------
{
    QStringList args;
    switch (mode)
    {
    case CR_Manual:  break;
    case CR_Ours:    args << "-s" << "recursive" << "-Xours";   break;
    case CR_Theirs:  args << "-s" << "recursive" << "-Xtheirs"; break;
    case CR_Unknown: std::cerr << "Unspecified conflict resolution mode\n";
    }
    return args;
}


bool GitRepository::merge(text branch, ConflictResolution how)
// ----------------------------------------------------------------------------
//   Merge another branch into the current one
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    QStringList args("merge");
    args << crArgs(how) << +branch;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::reset(text commit)
// ----------------------------------------------------------------------------
//   Reset a branch to normal state or to a given commit
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    QStringList args("reset");
    args << "--hard";
    if (!commit.empty())
        args << +commit;
    Process cmd(command(), args, path);
    return cmd.done(&errors);
}


bool GitRepository::pull()
// ----------------------------------------------------------------------------
//   Pull from remote branch, if remote is set
// ----------------------------------------------------------------------------
{
    if (pullFrom.isEmpty())
        return true;
    QString branch = +(this->branch());
    if (branch.isEmpty())
        branch = "master";
    clearCachedDocVersion();
    QStringList args("pull");
    args << crArgs(conflictResolution);
    args << pullFrom << branch;
    Process * proc = new Process(command(), args, path, false);
    connect(proc,  SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));
    dispatch(proc);
    return true;
}


bool GitRepository::push(QString pushUrl)
// ----------------------------------------------------------------------------
//   Push to a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    text branch = this->branch();
    QStringList args("push");
    args << pushUrl;
    Process cmd(command(), args << +branch, path);
    return cmd.done(&errors);
}


bool GitRepository::fetch(QString url)
// ----------------------------------------------------------------------------
//   Fetch (download objects and refs) from a remote repository
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("fetch") << url, path);
    return cmd.done(&errors);
}


QStringList GitRepository::remotes()
// ----------------------------------------------------------------------------
//   Return the names of all remotes configured in the repository
// ----------------------------------------------------------------------------
{
    QStringList result;
    text        output;
    waitForAsyncProcessCompletion();
    Process     cmd(command(), QStringList("remote"), path);
    if (cmd.done(&errors, &output))
    {
        result = (+output).split("\n");
        result.removeLast();            // Last string is always empty
    }
    return result;
}


QString GitRepository::remoteFetchUrl(QString name)
// ----------------------------------------------------------------------------
//   Return the pull/fetch URL for the specified remote
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "config" << "--get" << QString("remote.%1.url").arg(name);
    text    output;
    waitForAsyncProcessCompletion();
    Process cmd(command(), args, path);
    cmd.done(&errors, &output);
    return (+output).trimmed();
}


QString GitRepository::remotePushUrl(QString name)
// ----------------------------------------------------------------------------
//   Return the push URL for the specified remote
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "config" << "--get" << QString("remote.%1.pushUrl").arg(name);
    text    output;
    waitForAsyncProcessCompletion();
    Process cmd(command(), args, path);
    bool ok = cmd.done(&errors, &output);
    if (ok)
        return (+output).trimmed();
    return remoteFetchUrl(name);
}


QString GitRepository::remoteWithFetchUrl(QString url)
// ----------------------------------------------------------------------------
//   Return the name of the remote with specified fetch URL
// ----------------------------------------------------------------------------
{
    QStringList remotes = this->remotes();
    foreach (QString remote, remotes)
        if (remoteFetchUrl(remote) == url)
            return remote;
    return "";
}


bool GitRepository::addRemote(QString name, QString url)
// ----------------------------------------------------------------------------
//   Add a remote, giving its pull URL
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("remote") <<"add" <<name <<url, path);
    return cmd.done(&errors);
}

bool GitRepository::setRemote(QString name, QString url)
// ----------------------------------------------------------------------------
//   Set a new pull URL for a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("remote") <<"set-url" << name
                << url, path);
    return cmd.done(&errors);
}

bool GitRepository::delRemote(QString name)
// ----------------------------------------------------------------------------
//   Delete a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("remote") << "rm" << name, path);
    return cmd.done(&errors);
}


bool GitRepository::renRemote(QString oldName, QString newName)
// ----------------------------------------------------------------------------
//   Rename a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("remote") << "rename"
                << oldName << newName, path);
    return cmd.done(&errors);
}


QList<GitRepository::Commit> GitRepository::history(QString branch, int max)
// ----------------------------------------------------------------------------
//   Return the last commits on a branch in chronological order
// ----------------------------------------------------------------------------
//   If branch == "", the current branch is used
{
    QStringList args;
    args << "log" << "--pretty=format:%h:%s";
    args << "-n" << QString("%1").arg(max);
    if (!branch.isEmpty())
        args << branch;
    text    output;
    waitForAsyncProcessCompletion();
    Process cmd(command(), args, path);
    cmd.done(&errors, &output);

    QList<Commit>       result;
    QStringList         log = (+output).split("\n");
    QStringListIterator it(log);
    QRegExp             rx("([^:]+):(.*)");

    while (it.hasNext())
        if (rx.indexIn(it.next()) != -1)
            result.prepend(Repository::Commit(rx.cap(1), rx.cap(2)));

    return result;
}


Process * GitRepository::asyncClone(QString cloneUrl, QString newFolder)
// ----------------------------------------------------------------------------
//   Prepare a Process that will make a local copy of a remote project
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "clone" << "--progress" << cloneUrl;
    if (!newFolder.isEmpty())
        args << newFolder;
    Process * proc = new Process(command(), args, path, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));

    // Track % complete by reading stderr
    connect(proc, SIGNAL(standardErrorUpdated(QByteArray)),
            this, SLOT  (computePercentComplete()));
    connect(this, SIGNAL(percentComplete(int)),
            proc, SIGNAL(percentComplete(int)));  // signal forwarding

    return proc;
}


Process * GitRepository::asyncFetch(QString url)
// ----------------------------------------------------------------------------
//   Prepare a Process that will download latest changes from a remote project
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "fetch" << url;
    Process * proc = new Process(command(), args, path, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));

    // Track % complete by reading stderr
    connect(proc, SIGNAL(standardErrorUpdated(QByteArray)),
            this, SLOT  (computePercentComplete()));
    connect(this, SIGNAL(percentComplete(int)),
            proc, SIGNAL(percentComplete(int)));  // signal forwarding
    return proc;
}


text GitRepository::version()
// ----------------------------------------------------------------------------
//    Return the name of the current version of the document
// ----------------------------------------------------------------------------
{
    if (cachedDocVersion != "")
        return cachedDocVersion;

    text    output, result = +QString(tr("Unkwnown"));
    waitForAsyncProcessCompletion();
    QStringList args;
    args << "describe" << tr("--dirty=-dirty") << "--tags" << "--always";
    Process cmd(command(), args, path);
    bool    ok = cmd.done(&errors, &output);
    if (ok)
        result = cachedDocVersion = output;
    return result;
}


bool GitRepository::isClean()
// ----------------------------------------------------------------------------
//    Is working directory is clean or dirty?
// ----------------------------------------------------------------------------
//    Working directory is considered dirty when a file tracked by Git has
//    been modified or deleted and not committed, or a file not tracked by Git
//    is found in the directory.
{
    bool ok;
    text    output;
    waitForAsyncProcessCompletion();

    QStringList args;
    args << "status" << "--porcelain";
    Process cmd(command(), args, path);
    ok = cmd.done(&errors, &output);
    if (!ok || !output.empty())
        return false;

    return true;
}


QString GitRepository::url()
// ----------------------------------------------------------------------------
//    Return a valid URL for the current repository
// ----------------------------------------------------------------------------
{
    if (path.isEmpty())
        return "";

    QString hostname = QHostInfo::localHostName();
    QString url = QString("ssh://%1%2").arg(hostname).arg(path);
    return url;
}


bool GitRepository::gc()
// ----------------------------------------------------------------------------
//    Run garbage collection
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    Process cmd(command(), QStringList("gc"), path);
    return cmd.done(&errors);
}

TAO_END
