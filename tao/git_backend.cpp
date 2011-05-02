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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "git_backend.h"
#include "renderer.h"
#include "options.h"
#include "tao_utf8.h"
#include "error_message_dialog.h"

#include <QDir>
#include <QString>
#include <QtGlobal>
#include <QApplication>
#include <QRegExp>
#include <QHostInfo>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include <iostream>

namespace Tao {

// The 'git' command. May be updated by checkGit().
QString GitRepository::gitCommand("git");
// Used if git needs GIT_EXEC_PATH/GIT_TEMPLATE_DIR to be set
QString GitRepository::gitExecPath;
QString GitRepository::gitTemplateDir;
// The command that ssh should use to prompt user for a password.
// Updated by checkGit().
QString GitRepository::sshAskPassCommand;
#ifdef CONFIG_MINGW
// Windows needs to run git process through a special spawner (detach.exe)
QString GitRepository::detachCommand;
#endif


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
//   Return true if Git is functional, and set the git commands accordingly
// ----------------------------------------------------------------------------
{
    bool ok;
    ok = checkGitCmd();
#ifdef CONFIG_MACOSX
    ok = ok && checkCmd("SshAskPass.app/Contents/MacOS/SshAskPass",
                        "SSH_ASKPASS", sshAskPassCommand);
#else
    ok = ok && checkCmd("SshAskPass", "SSH_ASKPASS", sshAskPassCommand);
#endif
#ifdef CONFIG_MINGW
    ok = ok && checkCmd("detach.exe", "TAO_DETACH", detachCommand);
#endif
    return ok;
}


bool GitRepository::checkGitCmd()
// ----------------------------------------------------------------------------
//   Locate the git command, return true on success
// ----------------------------------------------------------------------------
{
    // Test user-defined git command (if any), then
    // Look for "git" in <application's directory>/git/bin, then in $PATH
again:
    QStringList commands;
    QString userCmd = QSettings().value("GitCommand").toString();
    if (!userCmd.isEmpty())
        commands << userCmd;
    commands << qApp->applicationDirPath() + "/git/bin/git" << "git";
    QStringListIterator it(commands);
    bool ok = false;
    while (it.hasNext())
    {
        text errors, output;
        gitCommand = resolveExePath(it.next());
        Process cmd(gitCommand, QStringList("--version"));
        if (cmd.done(&errors, &output))
        {
            // Check version is at least 1.7.0
            // (we need the merge -Xours / -Xtheirs feature)
            QString ver = (+output).replace(QRegExp("[^\\d\\.]"), "");
            if (versionGreaterOrEqual(ver, "1.7.0"))
            {
                ok = true;
                break;
            }
        }
    }

    if (ok)
    {
        IFTRACE(process)
            std::cerr << "Will use git command: " << +gitCommand << "\n";
#ifndef Q_OS_WIN
        QString appPath = qApp->applicationDirPath();
        if (gitCommand == appPath + "/git/bin/git")
        {
            gitExecPath    = appPath + "/git/libexec/git-core";
            gitTemplateDir = appPath + "/git/share/git-core/templates";
            IFTRACE(process)
            {
                std::cerr << "Will set GIT_EXEC_PATH to: "
                          << +gitExecPath << "\n";
                std::cerr << "Will set GIT_TEMPLATE_DIR to: "
                          << +gitTemplateDir << "\n";
            }
        }
#endif
        return true;
    }

    // !ok
    bool again = showGitSelectionDialog();
    if (again)
        goto again;

    IFTRACE(process)
        std::cerr << "No valid git command found\n";
    RepositoryFactory::no_repo = true;
    return false;
}


bool GitRepository::checkCmd(QString cmd, QString var, QString &out)
// ----------------------------------------------------------------------------
//   Locate a command, copy full path into 'out' and return true on success
// ----------------------------------------------------------------------------
{
    // First try to locate the command in Tao's application directory, then
    // in $PATH, then use the value of the environment variable 'var', if set.
    QStringList commands;
    commands << qApp->applicationDirPath() + "/" + cmd
             << cmd;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value(var);
    if (!path.isEmpty())
        commands << path;
    bool ok = false;
    foreach (QString s, commands)
    {
        QString p = resolveExePath(s);
        if (!p.isEmpty())
        {
            out = p;
            ok = true;
            break;
        }
    }
    if (ok)
    {
        IFTRACE(process)
            std::cerr << "Will use " << +cmd << " command: " << +out << "\n";
    }
    else
    {
        IFTRACE(process)
            std::cerr << "No valid " << +cmd << " command found\n";
    }

    return true; // REVISIT: should return false
}


QString GitRepository::resolveExePath(QString cmd)
// ----------------------------------------------------------------------------
//   Find an executable, using system PATH if needed. Return absolute path.
// ----------------------------------------------------------------------------
{
    QString ret;
    if (QFileInfo(cmd).isAbsolute())
        ret = checkExe(cmd);
    else
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString path = env.value("PATH");
#ifdef CONFIG_MINGW
        QString sep = ";";
#else
        QString sep = ":";
#endif
        QStringList paths = path.split(sep);
        foreach (QString d, paths)
        {
            QString file = d + "/" + cmd;
            QString p = checkExe(file);
            if (!p.isEmpty())
            {
                ret = p;
                break;
            }
        }
    }

    return ret;
}


QString GitRepository::checkExe(QString cmd)
// ----------------------------------------------------------------------------
//    If cmd is executable, return canonical (and native) path to cmd
// ----------------------------------------------------------------------------
{
    IFTRACE(process)
        std::cerr << "Testing " << +cmd << "\n";
    QString path;
    path = QFileInfo(cmd).canonicalFilePath();
    if (QFileInfo(path).isExecutable())
    {
        path = QDir::toNativeSeparators(path);
        IFTRACE(process)
            std::cerr << "Executable found, canonical path: " << +path << "\n";
        return path;
    }

#ifdef CONFIG_MINGW
    // Second chance on Windows: append .exe
    if (!cmd.endsWith(".exe", Qt::CaseInsensitive))
        return checkExe(cmd + ".exe");
#endif

    return "";
}


bool GitRepository::showGitSelectionDialog()
// ----------------------------------------------------------------------------
//   Enable user to select the git program to use
// ----------------------------------------------------------------------------
{
    ErrorMessageDialog dialog;
    dialog.setWindowTitle(tr("Version control software"));
    bool shown = dialog.showMessage(tr("No supported version control software was "
                              "found. Some functions will not be available. "
                              "Consider re-installing the application, "
                              "or installing Git v1.7.0 or later."));
    if (!shown)
        return false;

    int ret;
    ret = QMessageBox::question(NULL, tr("Git selection"),
                                tr("Missing command: git\n"
                                   "Do you want to locate the file yourself?"),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
        return false;
    QString gitCommand;
    gitCommand = QFileDialog::getOpenFileName
                           (NULL,
                            tr("Select git program"));
    if (!gitCommand.isEmpty())
    {
        IFTRACE(settings)
            std::cerr << "Setting GitCommand to '" << +gitCommand <<"'\n";
        QSettings().setValue("GitCommand", gitCommand);
    }
    return true;
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
    GitProcess cmd(command(), QStringList("branch"), path);
    return cmd.done(&errors);
}


bool GitRepository::initialize()
// ----------------------------------------------------------------------------
//    Initialize a git repository if we need to
// ----------------------------------------------------------------------------
{
    if (!QDir(path).mkpath("."))
        return false;
    GitProcess cmd(command(), QStringList("init"), path);
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
    GitProcess cmd(command(), QStringList("add") << ".tao", path);
    if (!cmd.done(&errors))
        return false;
    GitProcess cmd2(command(), QStringList("commit") << "-a" << "-m"
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
    GitProcess cmd(command(), QStringList("branch"), path);
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
    GitProcess cmd(command(), QStringList("branch") << "-a", path);
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
            if (branch == "")
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), QStringList("checkout") << +branch, path);
    return cmd.done(&errors);
}


bool GitRepository::branch(text name)
// ----------------------------------------------------------------------------
//    Create a new branch
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), QStringList("branch") << +name, path);
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), QStringList("add") << +name, path);
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
    GitProcess cmd(command(), QStringList("rm") << +name, path);
    return cmd.done(&errors);
}


bool GitRepository::rename(text from, text to)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    clearCachedDocVersion();
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), QStringList("mv") << +from << +to, path);
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
        if (taskDescription != "")
            message = "[" + taskDescription + "]\n";
        message += nextCommitMessage;
        nextCommitMessage = "";
    }
    if (all)
        add("");
    QStringList args("commit");
    if (all)
        args << "-a";
    args << "-m" << +message;
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), args, path);
    return cmd.done(&errors);
}


void GitRepository::computePercentComplete()
// ----------------------------------------------------------------------------
//   Extract % complete from the text output of a clone/fetch/push process
// ----------------------------------------------------------------------------
{
    Process *cmd = (Process *)sender();
    int &pos = cmd->errPos;
    bool matched = false;
    int percent = -1;
    do
    {
        // The percent points allocated to each of the 3 phases below
        // (compressing, receiving, resolving).
        // Phases #1 and #3 are typically much faster than #2 so we give them
        // less weight.
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
    }
    else if (op == "ls-remote")
    {
        QStringList tags;
        tags = parseLsRemoteTagsOutput(cmd->out);
        emit asyncGetRemoteTagsComplete(tags);
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


QStringList GitRepository::parseLsRemoteTagsOutput(QString output)
// ----------------------------------------------------------------------------
//   Extract the tag names from output of "git ls-remote --tags <remote_name>"
// ----------------------------------------------------------------------------
{
    // output is like:
    // a88e2db4dee6097d32add4848d810cabd915783c	refs/tags/2.0
    // dbac2178d79ecdc58948c585bbcc5ad8f9040119	refs/tags/2.1
    // bd83a3bf62d1a92cf3a0e7c97055a69e1cbc1d4a	refs/tags/2.1^{}

    QStringList lines, tags;
    if (output != "")
        lines = output.split("\n");

    foreach (QString line, lines)
    {
        QRegExp re("^[^r]+refs/tags/([^\\^]+)");
        if (re.indexIn(line) != -1)
        {
            QString tag = re.cap(1);
            if (!tags.contains(tag))
                tags << tag;
        }
    }

    return tags;
}


void GitRepository::mergeCommitMessages(text &dest, text src)
// ----------------------------------------------------------------------------
//   Add to 'dest' all lines from 'src' not already present in 'dest'
// ----------------------------------------------------------------------------
{
    QStringList d, s;
    if (dest != "")
        d = (+dest).split("\n");
    if (src != "")
        s = (+src).split("\n");

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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), args, path);
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
    GitAuthProcess * proc = new GitAuthProcess(args, path, false);
    connect(proc,  SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));
    dispatch(process_p(proc));
    return true;
}


process_p GitRepository::asyncPush(QString pushUrl)
// ----------------------------------------------------------------------------
//   Push to a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    text branch = this->branch();
    QStringList args("push");
    args << pushUrl << +branch;
    GitAuthProcess * proc = new GitAuthProcess(args, path, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));

    // Track % complete by reading stderr
    connect(proc, SIGNAL(standardErrorUpdated(QByteArray)),
            this, SLOT  (computePercentComplete()));
    connect(this, SIGNAL(percentComplete(int)),
            proc, SIGNAL(percentComplete(int)));  // signal forwarding
    return process_p(proc);
}


bool GitRepository::fetch(QString url)
// ----------------------------------------------------------------------------
//   Fetch (download objects and refs) from a remote repository
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    GitAuthProcess cmd(QStringList("fetch") << url, path);
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
    GitProcess     cmd(command(), QStringList("remote"), path);
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), args, path);
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
    GitProcess cmd(command(), QStringList("remote") <<"add" <<name <<url, path);
    return cmd.done(&errors);
}

bool GitRepository::setRemote(QString name, QString url)
// ----------------------------------------------------------------------------
//   Set a new pull URL for a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), QStringList("remote") <<"set-url" << name
                << url, path);
    return cmd.done(&errors);
}

bool GitRepository::delRemote(QString name)
// ----------------------------------------------------------------------------
//   Delete a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), QStringList("remote") << "rm" << name, path);
    return cmd.done(&errors);
}


bool GitRepository::renRemote(QString oldName, QString newName)
// ----------------------------------------------------------------------------
//   Rename a remote
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), QStringList("remote") << "rename"
                << oldName << newName, path);
    return cmd.done(&errors);
}


QList<GitRepository::Commit> GitRepository::history(QString branch, int max)
// ----------------------------------------------------------------------------
//   Return the last commits on a branch in chronological order
// ----------------------------------------------------------------------------
//   If branch == "", the current branch is used
//   Returned commits are contiguous, i.e., each commit is parent of the next
//   one (the commits of merged branches are not interspersed)
{
    QStringList args;
    args << "log" << "--first-parent" << "--pretty=format:%h/%at/%an/%s";
    args << "-n" << QString("%1").arg(max);
    if (!branch.isEmpty())
        args << branch;
    text    output;
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), args, path);
    cmd.done(&errors, &output);

    QList<Commit>       result;
    QStringList         log = (+output).split("\n");
    QStringListIterator it(log);
    QRegExp             rx("([^/]+)/([0-9]+)/([^/]+)/(.*)");

    while (it.hasNext())
        if (rx.indexIn(it.next()) != -1)
            result.prepend(Repository::Commit(rx.cap(1), rx.cap(4),
                                              rx.cap(2), rx.cap(3)));
    return result;
}


QString GitRepository::diff(QString a, QString b, bool symetric)
// ----------------------------------------------------------------------------
//   Return source code difference between version a and version b (commit ids)
// ----------------------------------------------------------------------------
{
    text output;
    QStringList args;
    args << "diff";
    if (symetric)
        args << QString("%1...%2").arg(a).arg(b);
    else
        args << a << b;
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), args, path);
    cmd.done(&errors, &output);

    return +output;
}


process_p GitRepository::asyncClone(QString cloneUrl, QString newFolder)
// ----------------------------------------------------------------------------
//   Prepare a Process that will make a local copy of a remote project
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "clone" << "--progress" << cloneUrl;
    if (!newFolder.isEmpty())
        args << newFolder;
    QString wd = path;
    if (!QFileInfo(wd).exists())
        wd = "";
    GitAuthProcess * proc = new GitAuthProcess(args, wd, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));

    // Track % complete by reading stderr
    connect(proc, SIGNAL(standardErrorUpdated(QByteArray)),
            this, SLOT  (computePercentComplete()));
    connect(this, SIGNAL(percentComplete(int)),
            proc, SIGNAL(percentComplete(int)));  // signal forwarding

    return process_p(proc);
}


process_p GitRepository::asyncFetch(QString what)
// ----------------------------------------------------------------------------
//   Prepare a Process that will download latest changes from a remote project
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "fetch" << what;
    GitAuthProcess * proc = new GitAuthProcess(args, path, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));

    // Track % complete by reading stderr
    connect(proc, SIGNAL(standardErrorUpdated(QByteArray)),
            this, SLOT  (computePercentComplete()));
    connect(this, SIGNAL(percentComplete(int)),
            proc, SIGNAL(percentComplete(int)));  // signal forwarding
    return process_p(proc);
}


process_p  GitRepository::asyncGetRemoteTags(QString remote)
// ----------------------------------------------------------------------------
//   Prepare a Process to download the list of tags from a remote peer
// ----------------------------------------------------------------------------
{
    QStringList args;
    args << "ls-remote" << "--tags" << remote;
    GitAuthProcess * proc = new GitAuthProcess(args, path, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));
    return process_p(proc);
}


QStringList GitRepository::tags()
// ----------------------------------------------------------------------------
//   Return the list of all local tags
// ----------------------------------------------------------------------------
{
    QStringList tags;
    text    output;
    waitForAsyncProcessCompletion();
    GitProcess cmd(command(), QStringList("tag"), path);
    bool    ok = cmd.done(&errors, &output);
    if (ok && output != "")
        tags = (+output).split("\n");
    return tags;
}


text GitRepository::version()
// ----------------------------------------------------------------------------
//    Return the name of the current version of the document
// ----------------------------------------------------------------------------
{
    if (cachedDocVersion != "")
        return cachedDocVersion;

    text    output, result = +QString(tr("Unknown"));
    waitForAsyncProcessCompletion();
    QStringList args;
    args << "describe" << tr("--dirty=-dirty") << "--tags" << "--always";
    GitProcess cmd(command(), args, path);
    bool    ok = cmd.done(&errors, &output);
    output = +(+output).trimmed();
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
    GitProcess cmd(command(), args, path);
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
    QString url = QString("tao://%1%2").arg(hostname).arg(path);
    return url;
}


bool GitRepository::gc()
// ----------------------------------------------------------------------------
//    Run garbage collection
// ----------------------------------------------------------------------------
{
    waitForAsyncProcessCompletion();
    GitProcess * proc = new GitProcess(command(), QStringList("gc"), path, false);
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int,QProcess::ExitStatus)));
    dispatch(process_p(proc));
    return true;
}


bool GitRepository::pathIsRoot()
// ----------------------------------------------------------------------------
//    Return true if path used to open repository is the top-level directory
// ----------------------------------------------------------------------------
{
    return (QDir(path + "/.git").exists());
}


void GitProcess::setEnvironment()
// ----------------------------------------------------------------------------
//    Set environment variables for a Git process
// ----------------------------------------------------------------------------
{
    Process::setEnvironment();

#ifndef Q_OS_WIN
    // If we use the git command packaged with Tao, we need to tell it where to
    // find its internal stuff
    QProcessEnvironment env = getProcessEnvironment();
    QString val;

    val = GitRepository::gitExecPath;
    if (!val.isEmpty())
        env.insert("GIT_EXEC_PATH", val);
    val = GitRepository::gitTemplateDir;
    if (!val.isEmpty())
        env.insert("GIT_TEMPLATE_DIR", val);

    setProcessEnvironment(env);
#else
    // On Windows, msysGit/PortableGit automatically finds its internal
    // commands based on the runtime path of the main executable
#endif
}


void GitAuthProcess::start()
// ----------------------------------------------------------------------------
//   Start child process
// ----------------------------------------------------------------------------
{
    setWd(wd);
    setEnvironment();

    // Here, 'this->cmd' is the path to the Git command, and 'this>args' are
    // the Git arguments. For instance: "/usr/bin/git fetch remote_name"
    // On Windows we MUST exec Git through the detach.exe wrapper (the source
    // code explains why -- see detach/main.cpp).
    // So, we want a command line like:
    // "C:\some\path\to\detach.exe C:\some\path\to\git.exe fetch remote_name"
    QStringList args = this->args;
    QString cmd;
#ifdef CONFIG_MINGW
    cmd = GitRepository::detachCommand;
    args.prepend(GitRepository::gitCommand);
#else
    cmd = GitRepository::gitCommand;
#endif

    IFTRACE(process)
    {
        QString commandLine = "{" + cmd;
        foreach (QString a, args)
            commandLine += "} {" + a;
        commandLine += "}";
        QString dir = workingDirectory();
        if (dir.isEmpty())
            dir = "<not set>";
        std::cerr << "Process " << num << ": " << +commandLine
                  << " (wd " << +dir << ")\n";
    }

    startTime.start();
    QProcess::start(cmd, args);
}


void GitAuthProcess::setEnvironment()
// ----------------------------------------------------------------------------
//    Set environment variables for the GitAuth process
// ----------------------------------------------------------------------------
{
    GitProcess::setEnvironment();

    // Variables needed for ssh authentication
    QString sap = GitRepository::sshAskPassCommand;
    if (!sap.isEmpty())
    {
        QProcessEnvironment env = getProcessEnvironment();

        env.insert("SSH_ASKPASS", sap);
        if (!env.contains("DISPLAY"))
            env.insert("DISPLAY", ":dummy");
        if (!env.contains("HOME"))
            env.insert("HOME", QDir::toNativeSeparators(QDir::homePath()));

        setProcessEnvironment(env);
    }
}

}
