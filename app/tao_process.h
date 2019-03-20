#ifndef TAO_PROCESS_H
#define TAO_PROCESS_H
// ****************************************************************************
//  tao_process.h                                                  Tao project
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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "base.h"

#include <string>
#include <iostream>

#include <QString>
#include <QProcess>
#include <QTextEdit>
#include <QTime>

namespace Tao {

struct Process : QProcess, std::streambuf
// ----------------------------------------------------------------------------
//   Process used to run an SCM command
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    Process(QObject *parent = 0, size_t bufSize = 1024);
    Process(const QString &cmd,
            const QStringList &args = QStringList(),
            const QString &workingDirectory = "",
            bool  startImmediately = true,
            size_t bufSize = 1024,
            bool combine = false);
    virtual ~Process();

    virtual void start();
    virtual bool done(text *errors = NULL, text *output = NULL);
    virtual bool failed();
    virtual void setWd(const QString &wd);

    void combineOutAndErr(bool flag=true) { combine = flag; }
    QString getTail(uint lines);

    QProcessEnvironment getProcessEnvironment();

    // Override QProcess methods
    void start(const QString &program, const QStringList &arguments,
               OpenMode openmode = ReadWrite);
    void start(const QString &program, OpenMode openmode = ReadWrite);

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

protected:                      // From std::streambuf
    virtual int sync();
    virtual int overflow(int c);

public:
    QString     cmd;
    QStringList args;
    QString     wd;
    OpenMode    mode;
    void       *id;
    bool        aborted;
    bool        combine;
    QString     err, out;
    unsigned    errOutMaxSize; // max size of err and out (characters)
    ulong        num;
    static ulong snum;    // For debug traces only

protected:
    QTime        startTime;
};

}

#endif // PROCESS_H
