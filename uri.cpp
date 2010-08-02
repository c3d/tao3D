// ****************************************************************************
//  uri.cpp                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of the Uri class.
//
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

#include "tao_utf8.h"
#include "uri.h"
#include "repository.h"
#include "application.h"
#include <QSettings>
#include <QDir>
#include <QProgressDialog>
#include <QThread>
#include <QMessageBox>



// QSettings group name, where we will store associations
// (URI -> local project path).
// This is to keep track of URIs so that next time an URI is open, we just
// update the local project with "git fetch" instead of cloning the remote
// repository altogether.
#define KNOWN_URIS_GROUP "KnownURIs"

namespace Tao {


Uri::~Uri()
// ----------------------------------------------------------------------------
//    Delete objects that may have been created by Uri
// ----------------------------------------------------------------------------
{
    if (proc)
        delete proc;
    if (progress)
        delete progress;
}


bool Uri::get()
// ----------------------------------------------------------------------------
//    Fetch/clone URI and emit the path to the local project or document
// ----------------------------------------------------------------------------
{
    if (!isValid())
        return false;

    bool ok = true;
    QString uri = toString();
    IFTRACE(uri)
        std::cerr << "Resolving URI: " << +uri << "\n";

    QString path;
    if (isLocal())
    {
        // Local URI, just translate it into a local path
        path = documentOrProjectPath();
        IFTRACE(uri)
            std::cerr << "URI resolves to local path: " << +path << "\n";
        emit docReady(path);
    }
    else
    {
        // Remote URI: needs download (clone or fetch)
        emit progressMessage(tr("Downloading %1...").arg(uri));
        project = localProject();
        clone = project.isEmpty();
        if (clone)
        {
            // Never seen this URI: clone this URI into a new project
            IFTRACE(uri)
                    std::cerr << "URI is not mapped to a local project\n";
            ok = cloneAndCheckout();
        }
        else
        {
            // Just update existing project by fetching from remote repository
            IFTRACE(uri)
                    std::cerr << "Known URI, project: " << +project << "\n";
            ok = fetchAndCheckout();
        }
    }
    return ok;
}


void Uri::gc()
// ----------------------------------------------------------------------------
//    Delete all QSettings keys that point to a non-existent local project
// ----------------------------------------------------------------------------
{
    IFTRACE2(settings, uri)
        std::cerr << "Cleaning obsolete {URI -> local project} mappings\n";

    QSettings settings;
    settings.beginGroup(KNOWN_URIS_GROUP);
    QStringList keys = settings.childKeys();
    int deleted = 0;
    foreach (QString key, keys)
    {
        QString project = settings.value(key).toString();
        bool exists = QDir(project).exists();
        if (!exists)
        {
            settings.remove(key);
            deleted++;
        }
        IFTRACE2(settings, uri)
        {
            QByteArray ba;
            ba.append(key);
            text uri = +QUrl::fromPercentEncoding(ba);
            std::cerr << " {" << uri << " -> " << +project << "} ";
            if (exists)
                std::cerr << "[keep]\n";
            else
                std::cerr << "[deleted]\n";
        }
    }
    IFTRACE2(settings, uri)
        std::cerr << " (" << deleted << "/" << keys.size()
                  << " deleted/total)\n";

    settings.endGroup();
}


QString Uri::fileName()
// ----------------------------------------------------------------------------
//    Return the document to open, if present in the URI.
// ----------------------------------------------------------------------------
{
    return queryItemValue("d");
}


QString Uri::rev()
// ----------------------------------------------------------------------------
//    Return the revision to checkout, if present in the URI. Default: "master"
// ----------------------------------------------------------------------------
//    Revision may be a branch name or a commit ID
{
    QString rev = queryItemValue("r");
    if (rev.isEmpty())
        return QString("master");
    return rev;
}


QString Uri::documentOrProjectPath()
// ----------------------------------------------------------------------------
//    Return the path to the document or project
// ----------------------------------------------------------------------------
//    tao://example.com/some/project               =>  /some/project
//    tao://example.com/some/project?d=my/doc.ddd  =>  /some/project/my/doc.ddd
{
    QString path = this->path();
    QString file = fileName();
    if (!file.isEmpty())
        path += "/" + file;
    return path;
}


bool Uri::isLocal()
// ----------------------------------------------------------------------------
//    Does the URI represent local resources?
// ----------------------------------------------------------------------------
{
    if (scheme() == "file")
        return true;
    QString uri = toString();
    if (QFileInfo(uri).exists())
        return true;

   // REVISIT
    // There are other cases of local URIs, such as when scheme is ssh and
    // host is "127.0.0.1", "localhost", etc.
    return false;
}


QString Uri::localProject()
// ----------------------------------------------------------------------------
//    Return the path of the local project if URI was cloned in the past, or ""
// ----------------------------------------------------------------------------
{
    QSettings settings;
    QString project = settings.value(keyName()).toString();
    bool exists = QDir(project).exists();
    if (exists)
        return project;
    return "";
}


void Uri::setLocalProject(const QString &path)
// ----------------------------------------------------------------------------
//    Associate current URI with a local project path
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.setValue(keyName(), path);
}


void Uri::clearLocalProject()
// ----------------------------------------------------------------------------
//    Remove association of current URI with a local project path
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.remove(keyName());
}


bool Uri::fetchAndCheckout()
// ----------------------------------------------------------------------------
//    Display a progress dialog and use Repository to fetch remote project
// ----------------------------------------------------------------------------
{
    // Prepare Repository object to fetch project
    project = localProject();
    repo = RepositoryFactory::repository(project,
                                         RepositoryFactory::OpenExisting);
    if (!repo)
        return false;

    // Prepare progress dialog
    progress = new QProgressDialog(tr("Opening %1...").arg(toString()),
                                   tr("Abort"), 0, 100, NULL);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);
    connect(progress, SIGNAL(canceled()), this, SLOT(abortDownload()));

    // Prepare fetch process
    QString repoUri = this->repoUri();
    IFTRACE(uri)
        std::cerr << "Fetching from " << +repoUri << "\n";

    QString remote = repo->remoteWithFetchUrl(repoUri);
    if (remote.isEmpty())
    {
        // Remote has to be named so that we can checkout the branch.
        int i = 1;
        do
        {
            remote = tr("remote%1").arg(i++);
        }
        while (!repo->remoteFetchUrl(remote).isEmpty());
        IFTRACE(uri)
            std::cerr << "Creating remote: " << +remote << "\n";
        repo->addRemote(remote, repoUri);
    }

    proc = repo->asyncFetch(remote);
    if (!proc)
        return false;
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onDownloadFinished(int,QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onDownloadError(QProcess::ProcessError)));
    connect(proc, SIGNAL(percentComplete(int)),
            progress, SLOT(setValue(int)));

    // Run clone process asynchronously. Checkout will be done later.
    (void)repo->dispatch(proc);

    return true;
}


bool Uri::cloneAndCheckout()
// ----------------------------------------------------------------------------
//    Clone new URI into a new project
// ----------------------------------------------------------------------------
{
    // Prepare Repository object to clone project
    project = newProject();
    repo = RepositoryFactory::repository(project, RepositoryFactory::Clone);
    if (!repo)
        return false;

    // Prepare progress dialog
    QString uri = toString();
    progress = new QProgressDialog(tr("Opening %1...").arg(uri),
                                   tr("Abort"), 0, 100, NULL);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);
    connect(progress, SIGNAL(canceled()), this, SLOT(abortDownload()));

    // Prepare clone process
    QString repoUri = this->repoUri();
    IFTRACE(uri)
        std::cerr << "Cloning from " << +repoUri << "\n";
    proc = repo->asyncClone(repoUri, project);
    if (!proc)
        return false;
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onDownloadFinished(int,QProcess::ExitStatus)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onDownloadError(QProcess::ProcessError)));
    connect(proc, SIGNAL(percentComplete(int)),
            progress, SLOT(setValue(int)));

    // Run clone process asynchronously. Checkout will be done later.
    (void)repo->dispatch(proc);

    return true;
}


void Uri::onDownloadFinished(int exitCode, QProcess::ExitStatus exitStatus)
// ----------------------------------------------------------------------------
//    Proceed with checkout after clone or fetch has completed
// ----------------------------------------------------------------------------
{
    if (aborted)
        return;

    // Close progress dialog
    progress->setValue(100);
    progress->deleteLater();

    if (exitStatus != QProcess::NormalExit)
        return;    // onDownloadError will handle this case

    bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
    if (!success)
    {
        // Non-zero status from clone
        QString msg = tr("Download failed.\nExit code: %1\n%2")
                         .arg(exitCode).arg(proc->err);
        QMessageBox::warning(NULL, tr("Error"), msg);
        emit getFailed();
        return;
    }

    checkout();
}


void Uri::onDownloadError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//    Display error dialog on download error
// ----------------------------------------------------------------------------
{
    if (aborted)
        return;
    QString err = Process::processErrorToString(error);
    QString msg = tr("Download failed: %1").arg(err);
    QMessageBox::warning(NULL, tr("Error"), msg);
    emit getFailed();
}


void Uri::abortDownload()
// ----------------------------------------------------------------------------
//    Abort current download
// ----------------------------------------------------------------------------
{
    Q_ASSERT(proc);
    Q_ASSERT(repo);
    aborted = true;
    repo->abort(proc);
    emit getFailed();
}


bool Uri::checkout()
// ----------------------------------------------------------------------------
//    Checkout a specific revision of the project, if specified in the Uri
// ----------------------------------------------------------------------------
{
    QString rev = this->rev();
    QString co = localRev(rev);

    IFTRACE(uri)
        std::cerr << "Checking out " << +rev << " (locally: " << +co << ")\n";

    Q_ASSERT(repo);
    bool ok = (repo && repo->checkout(+co));
    if (!ok)
    {
        QMessageBox::warning(NULL, tr("Error"), +(repo->errors));
        return false;
    }

    setLocalProject(project);

    emit docReady(docPath(project));
    return true;
}


QString Uri::localRev(QString rev)
// ----------------------------------------------------------------------------
//    Return the local name to use when checking out remote revision 'rev'
// ----------------------------------------------------------------------------
//    Examples:
//    01ca870      =>   01ca870
//                      (will create an anonymous branch)
//    branch_name  =>   branch_name if clone == true
//    branch_name  =>   remotes/<X>/branch_name if clone == false
//                      (<X> is the name of a remote that matches current URI;
//                       will create an anonymous branch)
//
//    REVISIT: this code is quite git-specific; most of it probably belongs to
//    the GitRepository classes.
{
    // Is rev an SHA-1 commit ID (hex string)?
    QRegExp rx("^[A-Fa-f0-9]+$");
    if (rx.exactMatch(rev))
        return rev;

    // rev is a remote branch name. If we have just clone()'d the project,
    // just checkout the same local branch name (we'll have a remote-tracking
    // branch)
    if (clone)
        return rev;

    // We have fetch()'ed the remote, we want to checkout the remote branch into
    // an anonymous local branch.
    Q_ASSERT(repo);
    QString uri = repoUri();
    QString remote = repo->remoteWithFetchUrl(uri);
    QString branch = QString("remotes/%1/%2").arg(remote).arg(rev);
    return branch;
}


QString Uri::repoUri()
// ----------------------------------------------------------------------------
//    Convert URI into a project URI suitable for use by the Repository class
// ----------------------------------------------------------------------------
//    Change tao: scheme to ssh:, and remove query/fragment info
//    tao://example.com/some/project/path?d=main.ddd
//      => ssh://example.com/some/project/path        (tao: is ssh: by default)
{
    QUrl uri(*this);
    uri.setEncodedQuery(QByteArray());
    if (uri.scheme() == "tao")
        uri.setScheme("ssh");

    return uri.toString();
}


QString Uri::keyName()
// ----------------------------------------------------------------------------
//    Convert URI in suitable format to use as a QSettings key name
// ----------------------------------------------------------------------------
{
    QString uri = repoUri();
    QByteArray ba = QUrl::toPercentEncoding(uri);
    QString name(ba);
    return KNOWN_URIS_GROUP "/" + name;
}


QString Uri::newProject()
// ----------------------------------------------------------------------------
//    Return the path to use when cloning URI into a new project
// ----------------------------------------------------------------------------
//    This is: <user's tao directory>/<remote project name>
//    If path already exists, append a number
{
    QString project;
    QString remoteName = path().section('/', -1);
    bool exists = false;
    int count = 1;
    if (remoteName.isEmpty())
        remoteName = tr("Temp");
    do
    {
        QString dir = remoteName;
        if (exists)
            dir = QString("%1%2").arg(remoteName).arg(count++);
        project = Application::defaultProjectFolderPath() + "/" + dir;
        exists = QFileInfo(project).exists();
    }
    while (exists);
    IFTRACE(uri)
        std::cerr << "Will clone into " << +project << "\n";
    return project;
}


QString Uri::docPath(QString project)
// ----------------------------------------------------------------------------
//    Given the path to a local repository, return the doc path to open
// ----------------------------------------------------------------------------
//    - If URI contains a file name parameter, append it to 'project'
//    - Otherwise, if 'project' contains only one .ddd file, append this file
//    - Otherwise, return 'project'
{
    QString path = project;
    QString name = this->fileName();
    if (!name.isEmpty())
    {
        path = project + "/" + name;
    }
    else
    {
        QDir dir(project);
        QStringList list = dir.entryList(QStringList("*.ddd"));
        if (list.size() == 1)
            path = project + "/" + list.first();
    }
    return path;
}

}
