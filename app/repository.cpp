// *****************************************************************************
// repository.cpp                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//     Abstract interface describing an SCM repository for the document
//     Derived class will provide implementations for git, hg, bzr
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2012, Soulisse Baptiste <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "repository.h"

#if !defined(CFG_NOGIT) || !defined(CFG_NONETWORK)
#include "git_backend.h"
#endif
#include "main.h"
#include "parser.h"
#include "renderer.h"
#include "tao_options.h"
#include "tao_utf8.h"

#include <QApplication>
#include <QDir>
#include <QtGlobal>
#include <fstream>
#include <math.h>

#if defined(CFG_NOGIT) && defined(CFG_NONETWORK)
#define REPO_DISABLED 1
#endif


RECORDER(repository, 16, "Document repository operations");

namespace Tao {

QMap<QString, QWeakPointer<Repository> > RepositoryFactory::cache;
Repository::Kind  RepositoryFactory::availableScm = Repository::Unknown;
bool              RepositoryFactory::no_repo      = false;
bool              RepositoryFactory::no_local     = false;
struct Repository::Commit Repository::HeadCommit  = Repository::Commit("HEAD");


Repository::Repository(const QString &path)
// ----------------------------------------------------------------------------
//   Repository constructor
// ----------------------------------------------------------------------------
    : path(path),
      pullInterval(Opt::pullInterval),
      state(RS_Clean), whatsNew("")
{
    record(repository, "Creating repository %p path %s", this, path);
    connect(&branchCheckTimer, SIGNAL(timeout()),
            this, SLOT(checkCurrentBranch()));
    branchCheckTimer.start(1000);
}


Repository::~Repository()
// ----------------------------------------------------------------------------
//   Remove self from cache (if present)
// ----------------------------------------------------------------------------
{
    record(repository, "Deleting repository %p path %s", this, path);
    RepositoryFactory::removeFromCache(path);
}


text Repository::fullName(text fileName)
// ----------------------------------------------------------------------------
//   Return the full name of an element in the repository or "" if outside
// ----------------------------------------------------------------------------
{
    QDir dir(path);
    QString file = QString::fromUtf8(fileName.data(), fileName.length());
    QString fullPath = dir.filePath(file);
    QString cleanPath = QDir::cleanPath(fullPath);
    if (!cleanPath.startsWith(path))
        return "";
    return fullPath.toStdString();
}


text Repository::styleSheet()
// ----------------------------------------------------------------------------
//   Return the style sheet we use for this repository
// ----------------------------------------------------------------------------
{
    return "system:xl.stylesheet";
}


bool Repository::write(text fileName, XL::Tree *tree)
// ----------------------------------------------------------------------------
//   Write the text into a repository, ready to commit, return true if OK
// ----------------------------------------------------------------------------
{
    record(repository, "Writing file %s from tree %t", fileName, tree);
    bool ok = false;
    text full = fullName(fileName);

    if (full == "")
        return false;

    if (!QFileInfo(+full).isWritable())
        return false;

    // Write the file in a copy (avoid overwriting original)
    text copy = full + "~";
    {
        QFileInfo stylesheetFile(+styleSheet());
        QFileInfo syntaxFile("system:xl.syntax");
        QString sspath(stylesheetFile.absoluteFilePath());
        QString sypath(syntaxFile.absoluteFilePath());
        record(repository,
               "Rendering %t using git stylesheet %s syntax %s",
               tree, sspath, sypath);
        std::ofstream output(copy.c_str());
        XL::Syntax syntax(+sypath);
        XL::Renderer renderer(output, +sspath, syntax);
        renderer.Render(tree);
        output.flush();
        ok = output.good();
    }

    // If we were successful writing, rename to new file
    // Note: on Windows, std::rename (or QDir::rename) fail if
    // destination exists, hence the rename/rename/remove
    text backup = full + ".bak";
    if (ok)
        ok = std::rename(full.c_str(), backup.c_str()) == 0;
    if (ok)
        ok = std::rename(copy.c_str(), full.c_str()) == 0;
    if (ok)
        ok = std::remove(backup.c_str()) == 0;

    state = RS_NotClean;

    return ok;
}


XL::Tree *Repository::read(text fileName)
// ----------------------------------------------------------------------------
//   Read a tree from a given file in the repository
// ----------------------------------------------------------------------------
{
    XL::Tree *result = NULL;
    text      full   = fullName(fileName);

    if (full == "")
        return NULL;  // fileName is not in the repository

    // Create the parser, with a local copy of the syntax (per-file syntax)
    XL::Syntax     syntax (XL::MAIN->syntax);
    XL::Positions &positions = XL::MAIN->positions;
    XL::Errors     errors;
    XL::Parser     parser(full.c_str(), syntax, positions, errors);

    result = parser.Parse();
    record(repository, "Read file %s, content was %t", fileName, result);
    return result;
}


bool Repository::setTask(text name)
// ----------------------------------------------------------------------------
//   Set the name of the current task. The task is the current branch.
// ----------------------------------------------------------------------------
{
    // Don't fail if working directory is not clean
    if (!isClean())
        if (!commit(+tr("Automatic commit on document opening "
                        "(clean working directory)"), true))
            return false;

    // Temporary branch
    if (name == "")
        return true;

    // Check if we can checkout the branch
    if (!checkout(name))
        if (!branch(name) || !checkout(name))
            return false;

    return true;
}


bool Repository::idle()
// ----------------------------------------------------------------------------
//    Return true if there is no pending command to execute
// ----------------------------------------------------------------------------
{
    return pQueue.empty();
}


void Repository::markChanged(text reason)
// ----------------------------------------------------------------------------
//    Record a change, reason text will be used in next commit message
// ----------------------------------------------------------------------------
{
    if (whatsNew.find(reason) == whatsNew.npos)
    {
        if (whatsNew.length())
            whatsNew += "\n";
        whatsNew += reason;
    }
}


void Repository::setTaskDescription(QString desc)
// ----------------------------------------------------------------------------
//    Record description of current task, will be used in next commit message
// ----------------------------------------------------------------------------
{
    taskDescription = +desc;
}


process_p Repository::dispatch(process_p cmd, void *id)
// ----------------------------------------------------------------------------
//   Insert process in run queue and start first process. Return cmd.
// ----------------------------------------------------------------------------
//   Caller is responsible for deleting the returned Process
{
    cmd->id = id;
    connect(cmd.data(), SIGNAL(error(QProcess::ProcessError)),
            this, SLOT  (asyncProcessError(QProcess::ProcessError)));
    pQueue.enqueue(cmd);
    if (pQueue.count() == 1)
        cmd->start();
    return cmd;
}


void Repository::waitForAsyncProcessCompletion()
// ----------------------------------------------------------------------------
//   Make sure no asynchrounous subprocess is running
// ----------------------------------------------------------------------------
{
    while (!pQueue.empty())
    {
        process_p p = pQueue.first();
        record(repository,
               "Process queue is not empty, first %u, waiting", p->num);
        QApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
}

void Repository::abort(process_p proc)
// ----------------------------------------------------------------------------
//   Abort an asynchronous process returned by dispatch()
// ----------------------------------------------------------------------------
{
    proc->aborted = true;
    if (pQueue.head() == proc)
    {
        proc->close();
        pQueue.dequeue();
        if (pQueue.count())
            pQueue.head()->start();
    }
    else
    {
        pQueue.removeOne(proc);
    }
}


void Repository::asyncProcessFinished(int exitCode, QProcess::ExitStatus st)
// ----------------------------------------------------------------------------
//   Default action when an asynchronous subprocess has finished
// ----------------------------------------------------------------------------
{
    (void)exitCode; (void)st;

    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    XL_ASSERT(cmd == pQueue.head()); (void)cmd;
}


void Repository::asyncProcessError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//   Default action when an asynchronous subprocess has an error
// ----------------------------------------------------------------------------
{
    (void)error;

    // Note: do not pop current process from run queue here!
    // This slot is called *in addition to* asyncProcessFinished, which
    // will do the cleanup.
}


bool Repository::versionGreaterOrEqual(QString ver, QString ref)
// ----------------------------------------------------------------------------
//    Return true if ver >= ref. For instance, "1.7.0" >= "1.6.6.2"
// ----------------------------------------------------------------------------
{
    QStringListIterator vit(ver.split("."));
    QStringListIterator rit(ref.split("."));
    while (vit.hasNext() && rit.hasNext())
    {
        int vi = vit.next().toInt();
        int ri = rit.next().toInt();
        if (vi > ri)
            return true;
        if (vi < ri)
            return false;
    }
    while (rit.hasNext())
        if (rit.next().toInt())
            return false;
    return true;
}


Repository::ProcQueueConsumer::~ProcQueueConsumer()
// ----------------------------------------------------------------------------
//   Pop the head process from process queue and start next one
// ----------------------------------------------------------------------------
{
    if (repo.pQueue.head()->aborted)
    {
        // We're here as a result of Repository::abort(). Let him clean up.
        return;
    }
    repo.pQueue.dequeue();
    if (repo.pQueue.count())
        repo.pQueue.head()->start();
}



// ============================================================================
//
//   Repository factory
//
// ============================================================================

QString RepositoryFactory::errors;

repository_ptr
RepositoryFactory::repository(QString path, RepositoryFactory::Mode mode)
// ----------------------------------------------------------------------------
//    Factory returning the right repository kind for a directory (with cache)
// ----------------------------------------------------------------------------
{
    // Do we know this guy already?
    if (cache.contains(path))
        return repository_ptr(cache.value(path));

    // Open (and optionally, create) a repository in 'path'
    repository_ptr rep(newRepository(path, mode));
    if (rep)
        cache.insert(path, QWeakPointer<Repository>(rep));

    return rep;
}


Repository *
RepositoryFactory::newRepository(QString path, RepositoryFactory::Mode mode)
// ----------------------------------------------------------------------------
//    Create the right repository object kind for a directory
// ----------------------------------------------------------------------------
{
#ifdef REPO_DISABLED
    Q_UNUSED(path);
    Q_UNUSED(mode);
    return NULL;
#else
    if (no_repo) return NULL;

    // Try a Git repository first
    bool no_local = false;
    if(mode == NoLocalRepo)
        no_local = true;

    Repository *git = new GitRepository(path, no_local);
    if (mode == OpenExistingHere && !git->pathIsRoot())
    {
        errors = QObject::tr("Path is not repository root");
        delete git;
        return NULL;
    }
    if (git->valid())
    {
        if (mode == Clone)
        {
            QString native = QDir::toNativeSeparators(QDir(path).
                                                      absolutePath());
            errors = QObject::tr("Can't clone into an existing "
                                 "repository: %1").arg(native);
            delete git;
            return NULL;
        }
        return git;
    }
    // path is not a valid repository
    if (mode == Create)
    {
        git->initialize();
        if (git->valid())
            return git;
        errors = QObject::tr("Repository initialization failed");
    }
    if (mode == Clone)
    {
        // Caller will call Repository::clone()
        return git;
    }

    delete git;
    return NULL;
#endif
}


void RepositoryFactory::removeFromCache(QString path)
// ----------------------------------------------------------------------------
//    Called by Repository destructor to remove self
// ----------------------------------------------------------------------------
{
    // It is possible for a Repository object to be created, then deleted
    // without being cached (see newRepository()), hence the following test
    if (cache.contains(path))
        cache.remove(path);
}


bool RepositoryFactory::available()
// ----------------------------------------------------------------------------
//    Test if Repository features are available
// ----------------------------------------------------------------------------
{
#ifdef REPO_DISABLED
    return false;
#else
    if (availableScm == Repository::Unknown ||
        availableScm == Repository::None)
    {
        availableScm = Repository::None;
        if (GitRepository::checkGit())
            availableScm = Repository::Git;
    }
    bool available = (availableScm != Repository::None);
    if (!available)
        errors = QObject::tr("'git' command not found or invalid");
    return available;
#endif
}

}
