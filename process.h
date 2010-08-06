#ifndef PROCESS_H
#define PROCESS_H
// ****************************************************************************
//  process.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//     Encapsulate a process with a streambuf we can easily write to
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "base.h"

#include <QString>
#include <QProcess>
#include <QTextEdit>
#include <QTime>
#include <iostream>

namespace Tao {

struct Process : QProcess, std::streambuf
// ----------------------------------------------------------------------------
//   Process used to run an SCM command
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    Process(size_t bufSize = 1024);
    Process(const QString &cmd,
            const QStringList &args = QStringList(),
            const QString &workingDirectory = "",
            bool  startImmediately = true,
            size_t bufSize = 1024);
    virtual ~Process();

    virtual void start(const QString &cmd, const QStringList & arguments,
                       const QString &wd = "");
    virtual void start();
    virtual bool done(text *errors = NULL, text *output = NULL);
    virtual bool failed();

public:
    static QString processErrorToString(QProcess::ProcessError error);
    static QString exitStatusToString(QProcess::ExitStatus status);

protected slots:
    void         readStandardOutput();
    void         readStandardError();
    void         debugProcessError(QProcess::ProcessError error);
    void         debugProcessFinished(int exitCode, QProcess::ExitStatus st);

signals:
    void         standardOutputUpdated(QByteArray newtext);
    void         standardErrorUpdated(QByteArray newtext);
    void         percentComplete(int percent);

protected:
    virtual void initialize(size_t bufSize);
    virtual void setEnvironment();
    virtual void setWd(const QString &wd);

protected:                      // From std::streambuf
    virtual int sync();
    virtual int overflow(int c);

public:
    QString     commandLine;
    QString     cmd;
    QStringList args;
    QString     wd;
    void       *id;
    bool        aborted;
    QString     err, out;
    int         errPos;    // current position in err (% complete parsing)
    int         percent;   // % complete

public:
    ulong        num;
    static ulong snum;    // For debug traces only

protected:
    QTime        startTime;
};

}

#endif // PROCESS_H
