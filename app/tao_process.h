#ifndef TAO_PROCESS_H
#define TAO_PROCESS_H
// *****************************************************************************
// tao_process.h                                                   Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011,2013, Jérôme Forissier <jerome@taodyne.com>
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
