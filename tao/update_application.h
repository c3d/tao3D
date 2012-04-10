#ifndef UPDATE_APPLICATION_H
#define UPDATE_APPLICATION_H
#include "repository.h"
#include "process.h"
#include "tao.h"
#include <QObject>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QFileInfo>

namespace Tao {

class UpdateApplication : public QObject
// ------------------------------------------------------------------------
//   Asynchronously update the main application
// ------------------------------------------------------------------------
{
    Q_OBJECT

public:
    UpdateApplication();

    void           start();
    void           check(bool msg = false);
    void           update();    
    void           extract();

private slots:
    void           processRemoteTags(QStringList tags);
    void           onDownloadFinished(int exitCode, QProcess::ExitStatus status);
    void           abortDownload();
    void           onDownloadError(QProcess::ProcessError error);

private:
    QString                   from;             // Remote repository
    QFile                     to;               // Local directory
    QFileInfo                 info;

    QString                   edition;          // Tao edition
    double                    version;          // Tao version

    QMessageBox*              progress;
    bool                      aborted, updating;
    bool                      useMessage;


    repository_ptr            repo;
    process_p                 proc;
};

}

#endif // UPDATE_APPLICATION_H
