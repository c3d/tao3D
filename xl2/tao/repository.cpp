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
#include <fstream>

TAO_BEGIN

QMap<QString, QWeakPointer<Repository> > Repository::cache;
Repository::Kind  Repository::availableScm = Repository::Unknown;

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
    return "xl.stylesheet";
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
        renderer.SelectStyleSheet(styleSheet());
        renderer.Render(tree);
        output.flush();
        ok = output.good()  && !output.fail() && !output.bad();
    }

    // If we were successful writing, rename to new file
    if (ok)
        ok = std::rename(copy.c_str(), full.c_str()) == 0;

    state = RS_NotClean;

    return ok;
}


XL::Tree * Repository::read(text fileName)
// ----------------------------------------------------------------------------
//   Read a tree from a given file in the repository
// ----------------------------------------------------------------------------
{
    XL::Tree      *result    = NULL;
    text           full      = fullName(fileName);

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
    if (!commit("Automatic commit on Tao startup", true))
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
    return true;
}


QSharedPointer <Repository> Repository::repository(const QString &path, bool create)
// ----------------------------------------------------------------------------
//    Factory returning the right repository kind for a directory
// ----------------------------------------------------------------------------
{
    // Do we know this guy already?
    if (cache.contains(path))
        return QSharedPointer<Repository>(cache.value(path));
    QSharedPointer <Repository> rep(newRepository(path, create));
    if (rep)
        cache.insert(path, QWeakPointer<Repository>(rep));

    return rep;
}


Repository * Repository::newRepository(const QString &path, bool create)
// ----------------------------------------------------------------------------
//    Create the right repository object kind for a directory
// ----------------------------------------------------------------------------
{
    // Try a Git repository first
    Repository *git = new GitRepository(path);
    if (git->valid())
        return git;
    if (create)
    {
        git->initialize();
        if (git->valid())
            return git;
    }
    delete git;

    // Didn't work, fail
    return NULL;
}


bool Repository::available()
// ----------------------------------------------------------------------------
//    Test if Repository features are available
// ----------------------------------------------------------------------------
{
    if (availableScm == Unknown)
    {
        availableScm = None;
        if (GitRepository::checkGit())
            availableScm = Git;
    }
    return (availableScm != None);
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
    return checkout(task + TAO_UNDO_SUFFIX);
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


void Repository::dispatch(Process *cmd, AnsiTextEdit *err, AnsiTextEdit *out,
                          void *id)
// ----------------------------------------------------------------------------
//   Insert process in run queue and start first process
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
    pQueue.append(cmd);
    if (pQueue.count() == 1)
        cmd->start();
}


void Repository::asyncProcessFinished(int exitCode)
// ----------------------------------------------------------------------------
//   Default action when an asynchronous subprocess has finished normally
// ----------------------------------------------------------------------------
{
    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.first());
    if (exitCode)
        std::cerr << +tr("Async command failed, exit status %1: %2\n")
                     .arg((int)exitCode).arg(cmd->commandLine);
    cmd->sendStandardOutputToTextEdit();
    emit asyncProcessComplete(cmd->id);
}


void Repository::asyncProcessError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//   Default action when an asynchronous subprocess has not finished normally
// ----------------------------------------------------------------------------
{
    ProcQueueConsumer p(*this);
    Process *cmd = (Process *)sender();
    Q_ASSERT(cmd == pQueue.first());
    std::cerr << +tr("Async command error %1: %2\nError output:\n%3")
                 .arg((int)error).arg(cmd->commandLine)
                 .arg(QString(cmd->readAllStandardError()));
    cmd->sendStandardErrorToTextEdit();
    emit asyncProcessComplete(cmd->id);
}


Repository::ProcQueueConsumer::~ProcQueueConsumer()
// ----------------------------------------------------------------------------
//   Pop the head process from process queue, delete it and start next one
// ----------------------------------------------------------------------------
{
    delete repo.pQueue.takeFirst();
    if (repo.pQueue.count())
        repo.pQueue.first()->start();
}

TAO_END
