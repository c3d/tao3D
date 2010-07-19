// ****************************************************************************
//  source_control.cpp                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//     Abstract interface describing an SCM repository for the document
//     Derived class will provide implementations for git, hg, bzr
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

#include "repository.h"
#include "renderer.h"
#include "parser.h"
#include "main.h"
#include "git_backend.h"
#include "tao_utf8.h"

#include <QDir>
#include <QtGlobal>
#include <QApplication>
#include <fstream>

TAO_BEGIN

QMap<QString, QWeakPointer<Repository> > RepositoryFactory::cache;
Repository::Kind  RepositoryFactory::availableScm = Repository::Unknown;
struct Repository::Commit Repository::HeadCommit = Repository::Commit("HEAD");

Repository::~Repository()
// ----------------------------------------------------------------------------
//   Remove self from cache (if present)
// ----------------------------------------------------------------------------
{
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
    bool ok = false;
    text full = fullName(fileName);

    if (full == "")
        return false;

    // Write the file in a copy (avoid overwriting original)
    text copy = full + "~";
    {
        std::ofstream output(copy.c_str());
        XL::Renderer renderer(output);
        QFileInfo stylesheet(+styleSheet());
        QFileInfo syntax("system:xl.syntax");
        QString sspath(stylesheet.canonicalFilePath());
        QString sypath(syntax.canonicalFilePath());
        IFTRACE(paths)
                std::cerr << "Loading git stylesheet '" << +sspath
                << "' with syntax '" << +sypath << "'\n";
        renderer.SelectStyleSheet(+sspath, +sypath);
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


XL::Tree *  Repository::read(text fileName)
// ----------------------------------------------------------------------------
//   Read a tree from a given file in the repository
// ----------------------------------------------------------------------------
{
    XL::Tree *result = NULL;
    text      full   = fullName(fileName);

    // Create the parser, with a local copy of the syntax (per-file syntax)
    XL::Syntax     syntax (XL::MAIN->syntax);
    XL::Positions &positions = XL::MAIN->positions;
    XL::Errors    &errors    = XL::MAIN->errors;
    XL::Parser     parser(full.c_str(), syntax, positions, errors);

    result = parser.Parse();
    return result;
}


bool Repository::setTask(text name)
// ----------------------------------------------------------------------------
//   Set the name of the current task
// ----------------------------------------------------------------------------
//   We use the task to identify two branches in the repository
//       <name> is the work branch itself, recording user-defined checkpoints
//       <name>_tao_undo is the undo branch, managed by us
{
    text undo = name + TAO_UNDO_SUFFIX;
    task = name;

    // Don't fail if working directory is not clean
    if (!isClean())
        if (!commit(+tr("Automatic commit on document opening "
                        "(clean working directory)"), true))
            return false;
    // Check if we can checkout the task branch
    if (!checkout(task))
        if (!branch(task) || !checkout(task))
            return false;

    // Check if we can checkout the undo branch
    if (!checkout(undo))
        if (!branch(undo)||!checkout(undo))
            return false;

    // Merge the undo branch into the task branch
    if (!checkout(task) || !merge(undo))
        return false;

    // Merge the task branch into the undo branch, stay on undo
    if (!checkout(undo) || !merge(task))
        return false;

    // We are now on the _undo branch
    cachedBranch = undo;
    return true;
}


bool Repository::selectWorkBranch()
// ----------------------------------------------------------------------------
//    Select the work branch
// ----------------------------------------------------------------------------
{
    return checkout(task);
}


bool Repository::selectUndoBranch()
// ----------------------------------------------------------------------------
//    Select the undo branch
// ----------------------------------------------------------------------------
{
    return checkout(undoBranch(task));
}


bool Repository::isUndoBranch(text name)
// ----------------------------------------------------------------------------
//    Is 'branch' is an undo branch?
// ----------------------------------------------------------------------------
{
    return (+name).endsWith(TAO_UNDO_SUFFIX);
}


text Repository::undoBranch(text name)
// ----------------------------------------------------------------------------
//   Take the name of a task branch and return the name of the undo branch
// ----------------------------------------------------------------------------
{
    if (isUndoBranch(name))
        return name;

    return name + TAO_UNDO_SUFFIX;
}


text Repository::taskBranch(text name)
// ----------------------------------------------------------------------------
//   Take the name of an undo branch and return the name of the task branch
// ----------------------------------------------------------------------------
{
    if (isUndoBranch(name))
    {
        size_t len = name.length() - (sizeof(TAO_UNDO_SUFFIX) - 1);
        name = name.substr(0, len);
    }
    return name;
}


bool Repository::mergeUndoBranchIntoTaskBranch(text undo)
// ----------------------------------------------------------------------------
//    Merge the (current) undo branch into the task branch and stay on undo
// ----------------------------------------------------------------------------
{
    if (!isUndoBranch(undo))
        return false;

    text task = taskBranch(undo);

    if (!checkout(task) || !merge(undo) || !checkout(undo))
        return false;

    return true;
}


bool Repository::mergeTaskBranchIntoUndoBranch(text task)
// ----------------------------------------------------------------------------
//    Merge the (current) task branch into the undo branch and stay on task
// ----------------------------------------------------------------------------
{
    if (isUndoBranch(task))
        return false;

    text undo = undoBranch(task);

    if (!checkout(undo) || !merge(task) || !checkout(task))
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


Process * Repository::dispatch(Process *cmd,
                               AnsiTextEdit *err, AnsiTextEdit *out,
                               void *id)
// ----------------------------------------------------------------------------
//   Insert process in run queue and start first process. Return cmd.
// ----------------------------------------------------------------------------
{
    cmd->id = id;
    connect(cmd,  SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT  (asyncProcessFinished(int)));
    connect(cmd,  SIGNAL(error(QProcess::ProcessError)),
            this, SLOT  (asyncProcessError(QProcess::ProcessError)));
    if (err)
    {
        cmd->errTextEdit = err;
        connect(cmd, SIGNAL(readyReadStandardError()),
                cmd, SLOT(sendStandardErrorToTextEdit()));
    }
    if (out)
    {
        cmd->outTextEdit = out;
        connect(cmd, SIGNAL(readyReadStandardOutput()),
                cmd, SLOT(sendStandardOutputToTextEdit()));
    }
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
        IFTRACE(process)
            std::cerr << "Async process queue not empty, waiting\n";
        QApplication::processEvents();
    }
}

void Repository::abort(Process *proc)
// ----------------------------------------------------------------------------
//   Abort an asynchronous process returned by dispatch()
// ----------------------------------------------------------------------------
{
    if (pQueue.head() == proc)
    {
        proc->aborted = true;
        proc->close();
        delete pQueue.dequeue();
        if (pQueue.count())
            pQueue.head()->start();
    }
    else
    {
        pQueue.removeOne(proc);
        delete proc;
    }
}


void Repository::asyncProcessFinished(int exitCode)
// ----------------------------------------------------------------------------
//   Default action when an asynchronous subprocess has finished
// ----------------------------------------------------------------------------
{
    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.head());
    if (exitCode)
    {
        IFTRACE(process)
            std::cerr << +tr("Async command failed, exit status %1: %2\n")
                         .arg((int)exitCode).arg(cmd->commandLine);
    }
}


void Repository::asyncProcessError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//   Default action when an asynchronous subprocess has an error
// ----------------------------------------------------------------------------
{
    // Note: do not pop current process from run queue here!
    // This slot is called *in addition to* asyncProcessFinished, which
    // will do the cleanup.
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.head());
    IFTRACE(process)
        std::cerr << +tr("Async command error %1: %2\nError output:\n%3")
                     .arg((int)error).arg(cmd->commandLine)
                     .arg(QString(cmd->readAllStandardError()));
    cmd->sendStandardErrorToTextEdit();
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
//   Pop the head process from process queue, delete it and start next one
// ----------------------------------------------------------------------------
{
    if (repo.pQueue.head()->aborted)
    {
        // We're here as a result of Repository::abort(). Let him clean up.
        return;
    }
    delete repo.pQueue.dequeue();
    if (repo.pQueue.count())
        repo.pQueue.head()->start();
}



// ============================================================================
//
//   Repository factory
//
// ============================================================================

repository_ptr
RepositoryFactory::repository(QString path, RepositoryFactory::Mode mode)
// ----------------------------------------------------------------------------
//    Factory returning the right repository kind for a directory (with cache)
// ----------------------------------------------------------------------------
{
    // Do we know this guy already?
    if (cache.contains(path))
    {
//        if (mode == RepositoryFactory::Clone)
//            return repository_ptr(NULL);  // Can't clone into existing repo
        return repository_ptr(cache.value(path));
    }

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
    // Try a Git repository first
    Repository *git = new GitRepository(path);
    if (git->valid())
    {
        if (mode == Clone)
        {
            // Can't clone into an existing repository
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
    }
    if (mode == Clone)
    {
        // Caller will call Repository::clone()
        return git;
    }
    delete git;
    return NULL;
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
    if (availableScm == Repository::Unknown)
    {
        availableScm = Repository::None;
        if (GitRepository::checkGit())
            availableScm = Repository::Git;
    }
    return (availableScm != Repository::None);
}

TAO_END
