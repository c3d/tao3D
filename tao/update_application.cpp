#include "update_application.h"
#include "tree.h"
#include "tao_tree.h"
#include "main.h"
#include "context.h"
#include "version.h"
#include "application.h"
#include "git_backend.h"
#include <QDir>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextStream>
#include <sstream>

namespace Tao {


UpdateApplication::UpdateApplication() : aborted(false), updating(false), useMessage(false)
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
{
    // Define remote repository for each OS
#if defined(Q_OS_MACX)
    from = "git://git.taodyne.com/software/macosx/";
#elif defined(Q_OS_WIN)
    from = "git://git.taodyne.com/software/win/";
#else
    from = "git://git.taodyne.com/software/linux/";

    // Check if we are on Debian or Ubuntu distribution to get .deb package
    QString cmd("uname");
    QStringList args;
    args << "-a";
    Process cp(cmd, args);
    text errors, output;
    if(cp.done(&errors, &output))
    {
        // Check bits number
        if(output.find("x86_64") != output.npos)
            from += "64";
        else
            from += "32";

        // Check os name
        if(output.find("Ubuntu") != output.npos ||
           output.find("Debian") != output.npos)
            from += "/debian/";
        else
            from += "/others/";
    }
#endif

#ifdef TAO_EDITION
    // Get current edition
    edition = TAO_EDITION;
    from += edition;
#endif

    // Create a message box to display download progress
    progress = new QMessageBox;
    progress->addButton(tr("Cancel"), QMessageBox::RejectRole);
    progress->setWindowModality(Qt::WindowModal);
}


void UpdateApplication::check(bool msg)
// ----------------------------------------------------------------------------
//    Check for new update
// ----------------------------------------------------------------------------
{
    useMessage = msg;

    // No need to update if already doing it
    if(!updating)
    {
        if(!repo)
        {
            IFTRACE(update)
                    debug() << "Begin check for update from " << from.toStdString() << std::endl;

            // Use empty repository
            repo = RepositoryFactory::repository("", RepositoryFactory::NoLocalRepo);
            connect(repo.data(),
                    SIGNAL(asyncGetRemoteTagsComplete(QStringList)),
                    this,
                    SLOT(processRemoteTags(QStringList)));
        }

        if(repo && repo->valid())
        {
            // Get the list of remote tags and process it
            proc = repo->asyncGetRemoteTags(from);
            repo->dispatch(proc);
            updating = true;
        }
    }
}


void UpdateApplication::update()
// ----------------------------------------------------------------------------
//    Prepare to launch update
// ----------------------------------------------------------------------------
{
    QString name = QString("Tao Presentations %1 %2").arg(edition).arg(version, 0, 'f', 2);

    // Ask for update
    QString title = tr("%1 available").arg(name);
    QString msg = tr("A new version of Tao Presentations is available, would you download it now ?");
    int ret = QMessageBox::question(NULL, title, msg,
                                    QMessageBox::Yes | QMessageBox::No);

    if(ret == QMessageBox::Yes)
    {
        // Choose folder
        QString folder = QFileDialog::getExistingDirectory(NULL,
                             tr("Select destination folder"),
                             Application::defaultProjectFolderPath());

        if(! folder.isEmpty())
        {
            // Set complete filename
            to.setFileName(folder + "/" + name + ".tar");
            info.setFile(to);

            // Start update
            start();
        }
    }
    else
    {
        updating = false;

        // Delete process
        proc.clear();
    }
}


void UpdateApplication::start()
// ----------------------------------------------------------------------------
//    Start update
// ----------------------------------------------------------------------------
{
    IFTRACE(update)
            debug() << "Begin to download the update to " << to.fileName().toStdString() << std::endl;

    aborted = false;

    // Show progress of current download
    progress->setWindowTitle(tr("Downloading..."));
    progress->setText(tr("Downloading %1...").arg(info.completeBaseName()));
    progress->show();

    connect(progress, SIGNAL(buttonClicked(QAbstractButton*)), this,
            SLOT(abortDownload()), Qt::UniqueConnection);

    // Begin download of the new version
    proc = repo->asyncArchive(from, to.fileName());
    connect(proc.data(), SIGNAL(finished(int,QProcess::ExitStatus)), this,
            SLOT(onDownloadFinished(int,QProcess::ExitStatus)), Qt::UniqueConnection);
    connect(proc.data(), SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onDownloadError(QProcess::ProcessError)), Qt::UniqueConnection);
    repo->dispatch(proc);
}


bool UpdateApplication::extract()
// ----------------------------------------------------------------------------
//    Extract update
// ----------------------------------------------------------------------------
{
    QStringList cmds;
    cmds << qApp->applicationDirPath() + "/git/bin/tar" << "tar";
    QString tar;
    foreach (QString cmd, cmds)
    {
        tar = GitRepository::resolveExePath(cmd);
        if (!tar.isEmpty())
            break;
    }
    if(tar.isEmpty())
    {
        IFTRACE(update)
            debug() << "Extraction failed.\nExit code: tar not found" << std::endl;
        return false;
    }

    // Extract update
    QStringList args;
    args << "-xf" << info.fileName();
    Process cp(tar, args, info.path());
    cp.waitForFinished();

    IFTRACE(update)
        debug() << "Extract update: " << tar.toStdString() << std::endl;

    if(cp.done())
    {
        // Show final message
        QString title = tr("Download finished");
        QString msg = tr("%1 has been downloaded successfully.").arg(info.completeBaseName());
        QMessageBox::information(NULL, title, msg);

        IFTRACE(update)
                debug() << "Download successfull" << std::endl;

        return true;
    }
    else
    {
        // Show error message
        QString msg = tr("Extraction failed.\nExit code: %1")
                         .arg(cp.err);
        QMessageBox::warning(NULL, tr("Error"), msg);

        IFTRACE(update)
                debug() << "Extraction failed: " << cp.err.toStdString() << std::endl;

        return false;
    }
}


void UpdateApplication::processRemoteTags(QStringList tags)
// ----------------------------------------------------------------------------
//   Process the list of remote tags for Tao application
// ----------------------------------------------------------------------------
{
    if(! tags.isEmpty())
    {
        // Get latest version of the remote
        QString latest = tags[tags.size() - 1];
        version = latest.toDouble();

        IFTRACE(update)
                debug() << "Latest version: " << latest.toStdString() << std::endl;

        // Get current version
        QString ver = GITREV;
        QRegExp rxp("([^-]*)");
        rxp.indexIn(ver);
        double current = rxp.cap(1).toDouble();

        IFTRACE(update)
                debug() << "Current version: " << current << std::endl;

        // Update if current version is older than the remote one
        bool upToDate = (current >= version);
        if(!upToDate)
        {
            update();
        }
        else
        {
            // Show an informative message if wanted
            if(useMessage)
            {
                QString title = tr("No update available");
                QString msg = tr("Tao Presentations %1 is up-to-date.").arg(edition);
                QMessageBox::information(NULL, title, msg);
            }

            updating = false;

            // Clear process
            proc.clear();
        }
    }
    else
    {
        IFTRACE(update)
                debug() << "No remote tags" << std::endl;
    }
}


void UpdateApplication::onDownloadFinished(int exitCode, QProcess::ExitStatus status)
// ----------------------------------------------------------------------------
//    Define action on download finished
// ----------------------------------------------------------------------------
{
    bool downloaded, extracted;

    // Close progress dialog
    progress->close();

    if (aborted)
        return;

    if (exitCode != QProcess::NormalExit)
        return;    // onDownloadError will handle this case

    downloaded = (status ==  QProcess::NormalExit && exitCode == 0);
    if(! downloaded)
    {
        // Show error message
        QString msg = tr("Download failed.\nExit code: %1\n%2")
                         .arg(exitCode).arg(proc->err);
        QMessageBox::warning(NULL, tr("Error"), msg);

        IFTRACE(update)
                debug() << "Download failed : " << proc->err.toStdString() << std::endl;
    }
    else
    {
        // Extract download
        extracted = extract();
    }

    updating = false;

    // Delete archive if download has failed or
    //  extraction is successful.
    if(! downloaded || extracted)
    {
        // Clear process
        proc.clear();
        // Delete file
        to.remove();
    }
}


void UpdateApplication::abortDownload()
// ----------------------------------------------------------------------------
//    Abort current download
// ----------------------------------------------------------------------------
{
    Q_ASSERT(proc);
    Q_ASSERT(repo);

    IFTRACE(update)
            debug() << "Abort download" << std::endl;

    aborted = true;
    updating = false;

    process_p p = proc;
    proc.clear();
    if (p)
        repo->abort(p);

    to.remove();
}


void UpdateApplication::onDownloadError(QProcess::ProcessError error)
// ----------------------------------------------------------------------------
//    Display error dialog on download error
// ----------------------------------------------------------------------------
{
    if (aborted)
        return;

    updating = false;

    progress->close();
    QString err = Process::processErrorToString(error);
    QString msg = tr("Download failed: %1").arg(err);
    QMessageBox::warning(NULL, tr("Error"), msg);

    IFTRACE(update)
            debug() << "Download error :" << err.toStdString() << std::endl;

    to.remove();
}


std::ostream & UpdateApplication::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[Update] ";
    return std::cerr;
}


}
