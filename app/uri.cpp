// *****************************************************************************
// uri.cpp                                                         Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2013-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

#include "tao_utf8.h"
#include "uri.h"
#include "repository.h"
#include "application.h"
#include "destination_folder_dialog.h"
#include "module_manager.h"
#include "preferences_pages.h"
#include <QSettings>
#include <QDir>
#include <QProgressDialog>
#include <QThread>
#include <QMessageBox>
#include <QInputDialog>



// QSettings group names, where we will store associations
// (URI -> local project path).
// This is to keep track of URIs so that next time an URI is open, we can
// update the local project with "git fetch" instead of cloning the remote
// repository altogether.
//
// Known URIs for regular documents (default)
#define KNOWN_URIS_GROUP "KnownURIs"
// Known URIs for templates
#define KNOWN_URIS_TMPL_GROUP "KnownURIsTemplates"
// Known URIs for modules
#define KNOWN_URIS_MOD_GROUP "KnownURIsModules"

namespace Tao {


QSet<QString> Uri::refreshed;


Uri::Uri()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
    : QUrl(), repo(NULL), done(false), progress(NULL), proc(NULL), pos(0),
      aborted(false), settingsGroup(KNOWN_URIS_GROUP), op(NONE)
{
    checkRefresh();
}


Uri::Uri(QString uri)
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
    : QUrl(uri), repo(NULL), done(false), progress(NULL), proc(NULL), pos(0),
      aborted(false), settingsGroup(KNOWN_URIS_GROUP), op(NONE)
{
    if (!isValid())
        return;
    setSettingsGroup();
}


Uri::~Uri()
// ----------------------------------------------------------------------------
//    Delete objects that may have been created by Uri
// ----------------------------------------------------------------------------
{
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
        debug() << "Resolving URI: " << +uri << "\n";

    QString path;
    if (isLocal())
    {
        // Local URI, just translate it into a local path
        path = documentOrProjectPath();
        IFTRACE(uri)
            debug() << "URI resolves to local path: " << +path << "\n";
        emit docReady(path);
    }
    else
    {
        // Remote URI: needs download (clone or fetch)
        QStringList projects = localProjects();
        IFTRACE(uri)
        {
            if (!projects.isEmpty())
            {
                QString msg = projects.join(" ");
                debug() << "Known URI, project(s): " << +msg << "\n";
            }
        }

        if (settingsGroup == KNOWN_URIS_TMPL_GROUP ||
            settingsGroup == KNOWN_URIS_MOD_GROUP)
        {
            // We're downlading a template or a module

            if (projects.isEmpty())
            {
                // Remote not previously cloned
                QString path = newProject();
                if (path.startsWith("!"))
                {
                    // Folder with same name already exists: just error out
                    path = path.right(path.length() - 1);
                    QString native =
                        QDir::toNativeSeparators(QDir(path).absolutePath());
                    QString title;
                    if (settingsGroup == KNOWN_URIS_TMPL_GROUP)
                        title = tr("Cannot install template");
                    else if (settingsGroup == KNOWN_URIS_MOD_GROUP)
                        title = tr("Cannot install module");
                    QString msg = tr("Folder %1 already exists").arg(native);
                    IFTRACE(uri)
                        debug() << +msg << "\n";
                    QMessageBox warn(QMessageBox::Warning, "", title);
                    warn.setInformativeText(msg);
                    warn.exec();
                    return false;
                }

                // OK to clone
                project = path;
                IFTRACE(uri)
                    debug() << "Cloning URI into new local folder: "
                              << +project << "\n";
                ok = cloneAndCheckout();
            }
            else
            {
                if (projects.size() > 1)
                {
                    std::cerr << "Warning: several folders share the same "
                        "URI. Re-run with -turi for details.\n";
                }
                // Update existing template/module by fetching from remote
                // repository
                project = projects[0];
                IFTRACE(uri)
                    debug() << "Fecthing into existing folder: " << +project
                              << "\n";
                ok = fetchAndCheckout();
            }
        }
        else
        {
            // We're downlading a regular document

            QString defaultPath = newProject();
            DestinationFolderDialog dialog(defaultPath, projects);
            dialog.setWindowTitle(tr("Document download"));
            dialog.exec();
            project = dialog.target();
            if (project.isEmpty())
                return false;

            emit progressMessage(tr("Downloading %1...").arg(uri));

            IFTRACE(uri)
                debug() << "Destination folder: " << +project << "\n";

            clone = !projects.contains(project);
           if (clone)
            {
                // Clone this URI into a new project
                IFTRACE(uri)
                    debug() << "Cloning URI into new local project: "
                              << +project << "\n";
                ok = cloneAndCheckout();
            }
            else
            {
                // Just update existing project by fetching from remote
                // repository
                IFTRACE(uri)
                    debug() << "Fecthing into existing project: " << +project
                              << "\n";
                ok = fetchAndCheckout();
            }
        }
    }

    return ok;
}


void Uri::checkRefresh()
// ----------------------------------------------------------------------------
//    Make sure user settings have been read once
// ----------------------------------------------------------------------------
{
    if (!refreshed.contains(settingsGroup))
    {
        refreshed.insert(settingsGroup);
        refreshSettings();
    }
}


void Uri::refreshSettings()
// ----------------------------------------------------------------------------
//    Refresh the user QSettings that map URIs to local project paths
// ----------------------------------------------------------------------------
//    This involves:
//      - Clearing paths to non-existent projects
//      - Deleting URIs that are left with no path
//      - Scanning projects in user's Tao document folder and creating new
//        entries for them
{

    int total = 0, deleted = 0, added = 0;

    record(settings,
           "Clearing obsolete {URI -> local project} mappings"
           "for group %s", +settingsGroup);
    emit progressMessage(tr("Checking known URIs"));

    QSettings settings;
    settings.beginGroup(settingsGroup);

    QStringList keys = settings.childKeys();
    foreach (QString key, keys)
    {
        QStringList projects = settings.value(key).toStringList();
        total += projects.size();
        foreach (QString project, projects)
        {
            bool exists = QDir(project).exists();
            if (!exists)
            {
                projects.removeOne(project);
                deleted++;
            }
            IFTRACE2(settings, uri)
            {
                QByteArray ba;
                ba.append(key);
                text uri = +QUrl::fromPercentEncoding(ba);
                debug() << " {" << uri << " -> " << +project << "} ";
                if (exists)
                    std::cerr << "[keep]\n";
                else
                    std::cerr << "[deleted]\n";
            }
        }

        int dups = projects.removeDuplicates();
        record(settings, "Removed %d duplicate paths", dups);
        if (projects.isEmpty())
            settings.remove(key);
        else
            settings.setValue(key, projects);
    }
    record(settings, "Deleted %d out of %d", deleted, total);

    QString folder = parentFolderForDownload();
    if (folder.isEmpty())
        return;

    IFTRACE2(settings, uri)
        debug() << "Looking for new URIs in directory: " << +folder << "\n";
    emit progressMessage(tr("Looking for new URIs [%1]").arg(folder));

    total = 0;
    QDir dir(folder);
    if (dir.isReadable())
    {
        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString project, subdirs)
        {
            QString projDir = QDir(dir.filePath(project)).absolutePath();
            projDir = QDir::toNativeSeparators(projDir);
            IFTRACE2(settings, uri)
                debug() << " " << +projDir;

            repository_ptr repo =
                RepositoryFactory::repository(projDir,
                                              RepositoryFactory::OpenExisting);
            if (!repo)
            {
                IFTRACE2(settings, uri)
                    std::cerr << " [not a Git repository]\n";
                continue;
            }
            IFTRACE2(settings, uri)
                std::cerr << "\n";
            QStringList remotes = repo->remotes();
            foreach (QString remote, remotes)
            {
                QString uri = repo->remoteFetchUrl(remote);
                if (!uri.isEmpty())
                {
                    total++;
                    Uri uriObj(uri);
                    bool ok = uriObj.addLocalProject(projDir);
                    if (ok)
                        added++;
                    IFTRACE2(settings, uri)
                    {
                        debug() << "  {" << +uri << " -> "
                                  << +projDir << "}";
                        if (ok)
                            std::cerr << " [added]";
                        else
                            std::cerr << " [known]";
                        std::cerr << "\n";
                    }
                }
            }
        }
    }

    IFTRACE2(settings, uri)
        debug() << " (" << added << "/" << total
                  << " added/total)\n";

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


QString Uri::parentFolderForDownload()
// ----------------------------------------------------------------------------
//    Return the parent folder to use by default when cloning
// ----------------------------------------------------------------------------
{
    QString folder;
    if (settingsGroup == KNOWN_URIS_GROUP)
        folder = Application::defaultProjectFolderPath();
    else if (settingsGroup == KNOWN_URIS_TMPL_GROUP)
        folder = Application::defaultTaoPreferencesFolderPath() + "/templates";
    else if (settingsGroup == KNOWN_URIS_MOD_GROUP)
        folder = Application::defaultTaoPreferencesFolderPath() + "/modules";
    return folder;
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


void Uri::setSettingsGroup()
// ----------------------------------------------------------------------------
//    Helper function to avoid code duplication
// ----------------------------------------------------------------------------
{
    if (hasQueryItem("t"))
        settingsGroup = KNOWN_URIS_TMPL_GROUP;
    else if (hasQueryItem("m"))
        settingsGroup = KNOWN_URIS_MOD_GROUP;
    else
        settingsGroup = KNOWN_URIS_GROUP;
    checkRefresh();
}


void Uri::setUrl(const QString & url)
// ----------------------------------------------------------------------------
//    Change URL
// ----------------------------------------------------------------------------
{
    QUrl::setUrl(url);
    setSettingsGroup();
}


void Uri::setUrl(const QString & url, ParsingMode parsingMode)
// ----------------------------------------------------------------------------
//    Change URL
// ----------------------------------------------------------------------------
{
    QUrl::setUrl(url, parsingMode);
    setSettingsGroup();
}


void Uri::setQueryItems(const QList<QPair<QString, QString> > & query)
// ----------------------------------------------------------------------------
//    Update query string
// ----------------------------------------------------------------------------
{
#if QT_VERSION >= 0x050000
    QUrlQuery q(*this);
    q.setQueryItems(query);
    setQuery(q);
#else
    QUrl::setQueryItems(query);
#endif
    setSettingsGroup();
}


#if QT_VERSION >= 0x050000
bool Uri::hasQueryItem(const QString & key) const
// ----------------------------------------------------------------------------
//   Compatibility binding for testing if an item is in the query
// ----------------------------------------------------------------------------
{
    QUrlQuery q(*this);
    return q.hasQueryItem(key);
}

QString Uri::queryItemValue(const QString & key) const
// ----------------------------------------------------------------------------
//   Compatibility binding for querying items in the query
// ----------------------------------------------------------------------------
{
    QUrlQuery q(*this);
    return q.queryItemValue(key);
}
#endif


QStringList Uri::localProjects()
// ----------------------------------------------------------------------------
//    Return paths of local projects if URI was cloned in the past, or empty
// ----------------------------------------------------------------------------
{
    QSettings settings;
    QStringList projects = settings.value(keyName()).toStringList();
    foreach (QString project, projects)
        if (project.isEmpty() || !QDir(project).exists())
            projects.removeOne(project);
    projects.sort();
    return projects;
}


bool Uri::addLocalProject(QString path)
// ----------------------------------------------------------------------------
//    Associate current URI with a local project path
// ----------------------------------------------------------------------------
{
    bool added = false;
    QSettings settings;
    QStringList projects = settings.value(keyName()).toStringList();
    path = QDir::toNativeSeparators(path);
    if (!projects.contains(path))
    {
        projects.append(path);
        settings.setValue(keyName(), projects);
        added = true;
    }
    return added;
}


void Uri::clearLocalProject()
// ----------------------------------------------------------------------------
//    Remove association of current URI with a local project path
// ----------------------------------------------------------------------------
{
    QSettings settings;
    settings.remove(keyName());
}


bool Uri::showRepoErrorDialog()
// ----------------------------------------------------------------------------
//    Display an error dialog when repository can't be open
// ----------------------------------------------------------------------------
{
    QString uri = toString();
    QString err = RepositoryFactory::errors;
    QString msg = tr("Could not open %1:\n%2").arg(uri).arg(err);
    IFTRACE(uri)
        debug() << +msg << "\n";
    QMessageBox::warning(NULL, tr("Error"), msg);
    return false;
}


bool Uri::fetchAndCheckout()
// ----------------------------------------------------------------------------
//    Display a progress dialog and use Repository to fetch remote project
// ----------------------------------------------------------------------------
{
    XL_ASSERT(!project.isEmpty());

    // Prepare Repository object to fetch project
    repo = RepositoryFactory::repository(project,
                                         RepositoryFactory::OpenExisting);
    if (!repo)
        return showRepoErrorDialog();

    op = FETCHING;

    // Prepare progress dialog
    progress = new QProgressDialog(tr("Opening %1...").arg(toString()),
                                   tr("Abort"), 0, 100, NULL);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);
    connect(progress, SIGNAL(canceled()), this, SLOT(abortDownload()));
    if (scheme() == "http" || scheme() == "https")
    {
        // Git returns no progress information in this case. Show an animation.
        progress->setRange(0, 0);
    }

    // Save current HEAD to detect update vs. no-op
    savedHead = +repo->head();

    // Prepare fetch process
    QString repoUri = this->repoUri();
    IFTRACE(uri)
        debug() << "Fetching from " << +repoUri << "\n";

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
            debug() << "Creating remote: " << +remote << "\n";
        repo->addRemote(remote, repoUri);
    }

    proc = repo->asyncFetch(remote);
    if (!proc)
        return false;
    connect(proc.data(), SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onDownloadFinished(int,QProcess::ExitStatus)));
    connect(proc.data(), SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onDownloadError(QProcess::ProcessError)));
    connect(proc.data(), SIGNAL(percentComplete(int)),
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
    XL_ASSERT(!project.isEmpty());

    repo = RepositoryFactory::repository(project, RepositoryFactory::Clone);
    if (!repo)
        return showRepoErrorDialog();

    op = CLONING;

    // Prepare progress dialog
    QString uri = toString();
    progress = new QProgressDialog(tr("Opening %1...").arg(uri),
                                   tr("Abort"), 0, 100, NULL);
    progress->setWindowModality(Qt::WindowModal);
    progress->setValue(0);
    connect(progress, SIGNAL(canceled()), this, SLOT(abortDownload()));
    if (scheme() == "http" || scheme() == "https")
    {
        // Git returns no progress information in this case. Show an animation.
        progress->setRange(0, 0);
    }

    // Prepare clone process
    QString repoUri = this->repoUri();
    IFTRACE(uri)
        debug() << "Cloning from " << +repoUri << "\n";
    bool shallow = (settingsGroup == KNOWN_URIS_MOD_GROUP);
    proc = repo->asyncClone(repoUri, project, shallow);
    if (!proc)
        return false;
    connect(proc.data(), SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(onDownloadFinished(int,QProcess::ExitStatus)));
    connect(proc.data(), SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onDownloadError(QProcess::ProcessError)));
    connect(proc.data(), SIGNAL(percentComplete(int)),
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
        if (op == CLONING && settingsGroup == KNOWN_URIS_MOD_GROUP)
        {
            // Since we use git fetch to clone modules we need to remove the
            // directory we have created. Not needed for templates and
            // documents (they use git clone).
            IFTRACE(uri)
                debug() << "Deleting " << +project << "\n";
            Application::recursiveDelete(project);
        }
        QString msg = tr("Download failed.\nExit code: %1\n%2")
                         .arg(exitCode).arg(proc->err);
        IFTRACE(uri)
            debug() << +msg << "\n";
        QMessageBox::warning(NULL, tr("Error"), msg);
        emit getFailed();
        return;
    }

    if (op == FETCHING && settingsGroup == KNOWN_URIS_MOD_GROUP)
    {
        // Do not checkout after fetching a module, because it may be in use
        ModuleManager *mmgr = ModuleManager::moduleManager();
        bool pending = mmgr->hasPendingUpdate(project);
        QString path;
        path = QDir::toNativeSeparators(QDir(project).absolutePath());
        if (pending)
            emit moduleUpdated(path);
        else
            emit moduleUpToDate(path);
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
    progress->close();
    QString err = Process::processErrorToString(error);
    QString msg = tr("Download failed: %1").arg(err);
    IFTRACE(uri)
        debug() << +msg << "\n";
    QMessageBox::warning(NULL, tr("Error"), msg);
    emit getFailed();
}


void Uri::abortDownload()
// ----------------------------------------------------------------------------
//    Abort current download
// ----------------------------------------------------------------------------
{
    XL_ASSERT(proc);
    XL_ASSERT(repo);
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
        debug() << "Checking out " << +rev << " (locally: " << +co << ")\n";

    XL_ASSERT(repo);
    bool ok = (repo && repo->checkout(+co));
    if (!ok)
    {
        IFTRACE(uri)
            debug() << repo->errors << "\n";
        QMessageBox::warning(NULL, tr("Error"), +(repo->errors));
        return false;
    }

    addLocalProject(project);

    if (settingsGroup == KNOWN_URIS_GROUP)
    {
        emit docReady(docPath(project));
        return true;
    }

    QString path;
    path = QDir::toNativeSeparators(QDir(project).absolutePath());
    if (settingsGroup == KNOWN_URIS_TMPL_GROUP)
    {
        connect(this, SIGNAL(cloned(QString)),
                this, SIGNAL(templateCloned(QString)));
        connect(this, SIGNAL(updated(QString)),
                this, SIGNAL(templateUpdated(QString)));
        connect(this, SIGNAL(upToDate(QString)),
                this, SIGNAL(templateUpToDate(QString)));
    }
    else if (settingsGroup == KNOWN_URIS_MOD_GROUP)
    {
        connect(this, SIGNAL(cloned(QString)),
                this, SIGNAL(moduleCloned(QString)));
        connect(this, SIGNAL(updated(QString)),
                this, SIGNAL(moduleUpdated(QString)));
        connect(this, SIGNAL(upToDate(QString)),
                this, SIGNAL(moduleUpToDate(QString)));
    }

    switch (op)
    {
    case CLONING:
        emit cloned(path);
        break;
    case FETCHING:
        if (savedHead != +repo->head())
            emit updated(path);
        else
            emit upToDate(path);
        // Refresh even if module was up-to-date because we may have fetched
        // a higher tag that points to the same commit (although it is
        // unlikely)
        if (settingsGroup == KNOWN_URIS_MOD_GROUP)
            ModuleManager::moduleManager()->refreshModuleProperties(path);
        savedHead = "";
        break;
    default:
        break;
    }

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
    XL_ASSERT(repo);
    QString uri = repoUri();
    QString remote = repo->remoteWithFetchUrl(uri);
    QString branch = QString("remotes/%1/%2").arg(remote).arg(rev);
    return branch;
}


QString Uri::repoUri()
// ----------------------------------------------------------------------------
//    Convert URI into a project URI suitable for use by the Repository class
// ----------------------------------------------------------------------------
//    Translate tao: and taos: schemes, and remove query/fragment info
//    tao://example.com/some/project/path?d=main.ddd
//      => git://example.com/some/project/path
{
    QUrl uri(*this);
#if QT_VERSION >= 0x050000
    // uri.setQuery("");
#else
    uri.setEncodedQuery(QByteArray());
#endif
    QString scheme = uri.scheme();
    bool isTao = scheme == "tao" || scheme == "taos";
    if (isTao)
    {
        if (scheme == "tao")
            uri.setScheme(GeneralPage::taoUriScheme());
        else if (scheme == "taos")
            uri.setScheme("ssh");

        // Use www.taodyne.com as the default host, and examples as default dir
        if (uri.host() == "")
        {
            uri.setHost("www.taodyne.com");
            QString path = uri.path();
            if (path.count("/") <= 1)
                uri.setPath("/examples" + uri.path());
        }
    }

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
    return settingsGroup + "/" + name;
}


QString Uri::newProject()
// ----------------------------------------------------------------------------
//    Return the path to use when cloning URI into a new project
// ----------------------------------------------------------------------------
//    For a document:
//    This is: <user's tao directory>/<remote project name>
//    If path already exists, append a number
//    For a template:
//    This is: <user's template directory</remote project name>
{
    QString project;
    QString remoteName = path().section('/', -1);
    bool exists = false;
    int count = 1;
    if (remoteName.isEmpty())
        remoteName = tr("Temp");
    QString folder = parentFolderForDownload();
    do
    {
        QString dir = remoteName;
        if (exists)
            dir = QString("%1_%2").arg(remoteName).arg(count++);
        project = folder + "/" + dir;
        exists = QFileInfo(project).exists();
        if (exists && (settingsGroup == KNOWN_URIS_TMPL_GROUP ||
                       settingsGroup == KNOWN_URIS_MOD_GROUP))
        {
            // We don't want several copies of the same template/module
            return "!" + project;
        }
    }
    while (exists);
    IFTRACE(uri)
        debug() << "Suggested new path to download to: " << +project << "\n";
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


std::ostream & Uri::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Uri] ";
    return std::cerr;
}

}
