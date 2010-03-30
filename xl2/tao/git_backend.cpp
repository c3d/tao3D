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
#include <QDir>
#include <QString>
#include <QtGlobal>
#include <QApplication>
#include <iostream>

TAO_BEGIN

// The 'git' command. May be updated by checkGit().
QString GitRepository::gitCommand("git");


bool GitRepository::checkGit()
// ----------------------------------------------------------------------------
//   Return true if Git is functional, and set the git command accordingly
// ----------------------------------------------------------------------------
{
    // Look for "git" in $PATH, then in application's directory
    QStringList commands;
    commands << "git" << qApp->applicationDirPath() + "/git";
    QStringListIterator it(commands);
    while (it.hasNext())
    {
        text errors;
        gitCommand = it.next();
        Process cmd(gitCommand, QStringList("--version"));
        if (cmd.done(&errors))
            return true;
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
    return "git.stylesheet";
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
                 << "\"Automatic initial commit\"", path);
    return cmd.done(&errors);
}


text GitRepository::branch()
// ----------------------------------------------------------------------------
//    Return the name of the current branch
// ----------------------------------------------------------------------------
{
    text    output, result;
    Process cmd(command(), QStringList("branch"), path);
    bool    ok = cmd.done(&errors, &output);
    if (ok)
    {
        QStringList branches = (+output).split("\n");
        QRegExp re("^[*].*");
        int index = branches.indexOf(re);
        if (index > 0)
            result = +branches[index].mid(2);
        else
            result = "master";  // May happen on a totally empty repository
    }
    return result;
}


bool GitRepository::checkout(text branch)
// ----------------------------------------------------------------------------
//    Checkout a given branch
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("checkout") << +branch, path);
    return cmd.done(&errors);
}


bool GitRepository::branch(text name)
// ----------------------------------------------------------------------------
//    Create a new branch
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("branch") << +name, path);
    bool result = cmd.done(&errors);
    if (!result)
        if (errors.find("already exists") != errors.npos)
            result = true;
    return result;
}


bool GitRepository::add(text name)
// ----------------------------------------------------------------------------
//   Add a new file to the repository
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("add") << +name, path);
    return cmd.done(&errors);
}


bool GitRepository::change(text name)
// ----------------------------------------------------------------------------
//   Signal that a file in the repository changed
// ----------------------------------------------------------------------------
{
    dispatch(new Process(command(), QStringList("add") << +name, path, false));
    return true;
}


bool GitRepository::rename(text from, text to)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("mv") << +from << +to, path);
    return cmd.done(&errors);
}


bool GitRepository::commit(text message, bool all)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    QStringList args("commit");
    if (all)
        args << "-a";
    args << "--allow-empty"    // Don't fail if working directory is clean
         << "-m" << +message;
    Process cmd(command(), args, path);
    bool result = cmd.done(&errors);
    if (!result)
        if (errors.find("nothing added") != errors.npos)
            result = true;
    return result;
}


bool GitRepository::asyncCommit(text message, bool all)
// ----------------------------------------------------------------------------
//   Rename a file in the repository
// ----------------------------------------------------------------------------
{
    QStringList args("commit");
    if (all)
        args << "-a";
    args << "--allow-empty"    // Don't fail if working directory is clean
         << "-m" << +message;
    dispatch(new Process(command(), args, path, false));
    return true;
}


void GitRepository::asyncProcessFinished(int exitCode)
// ----------------------------------------------------------------------------
//   An asynchronous subprocess has finished normally
// ----------------------------------------------------------------------------
{
    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.first());
    if (exitCode)
    {
        bool ok = false;
        cmd->done(&errors);
        if (cmd->args.first() == "commit")
            if (errors.find("nothing added") != errors.npos)
                ok = true;
        if (!ok)
            std::cerr << +tr("Async command failed, exit status %1: %2")
                             .arg((int)exitCode).arg(cmd->commandLine);
    }
}


void  GitRepository::asyncProcessError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//   An asynchronous subprocess has finished in error (e.g., crashed)
// ----------------------------------------------------------------------------
{
    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.first());
    std::cerr << +tr("Async command error %1: %2")
                    .arg((int)error).arg(cmd->commandLine);
}


void GitRepository::dispatch(Process *cmd)
// ----------------------------------------------------------------------------
//   Insert process in run queue and start first process
// ----------------------------------------------------------------------------
{
    connect(cmd,  SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int)));
    connect(cmd,  SIGNAL(error(QProcess::ProcessError)),
            this, SLOT  (asyncProcessError(QProcess::ProcessError)));
    pQueue.append(cmd);
    if (pQueue.count() == 1)
        cmd->start();
}


bool GitRepository::merge(text branch)
// ----------------------------------------------------------------------------
//   Merge another branch into the current one
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("merge") << +branch, path);
    return cmd.done(&errors);
}


bool GitRepository::reset()
// ----------------------------------------------------------------------------
//   Reset a branch to normal state
// ----------------------------------------------------------------------------
{
    Process cmd(command(), QStringList("reset") << "--hard", path);
    return cmd.done(&errors);
}


GitRepository::ProcQueueConsumer::~ProcQueueConsumer()
// ----------------------------------------------------------------------------
//   Pop the head process from process queue, delete it and start next one
// ----------------------------------------------------------------------------
{
    delete repo.pQueue.takeFirst();
    if (repo.pQueue.count())
        repo.pQueue.first()->start();
}

TAO_END
