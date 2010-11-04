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

Repository::Repository(const QString &path): path(path),
                                     pullInterval(XL::MAIN->options
                                                  .pull_interval),
                                     state(RS_Clean), whatsNew("")
{
    connect(&branchCheckTimer, SIGNAL(timeout()),
            this, SLOT(checkCurrentBranch()));
    branchCheckTimer.start(1000);
}


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

    if (full == "")
        return NULL;  // fileName is not in the repository

    // Create the parser, with a local copy of the syntax (per-file syntax)
    XL::Syntax     syntax (XL::MAIN->syntax);
    XL::Positions &positions = XL::MAIN->positions;
    XL::Errors     errors;
    XL::Parser     parser(full.c_str(), syntax, positions, errors);

    result = parser.Parse();
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
        IFTRACE(process)
            std::cerr << "Process queue not empty (first=#" << p->num
                      << "), waiting\n";
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
    Q_ASSERT(cmd == pQueue.head()); (void)cmd;
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
    // Try a Git repository first
    Repository *git = new GitRepository(path);
    if (git->valid())
    {
        if (mode == Clone)
        {
            errors = QObject::tr("Can't clone into an existing repository");
            delete git;
            return NULL;
        }
        else
        if (mode == OpenExistingHere)
        {
            if (!git->pathIsRoot())
            {
                errors = QObject::tr("Path is not repository root");
                delete git;
                return NULL;
            }
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
}

TAO_END
