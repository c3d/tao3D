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
#include "ansi_textedit.h"

#include <QString>
#include <QProcess>
#include <QTextEdit>
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

public slots:
    void         sendStandardOutputToTextEdit();
    void         sendStandardErrorToTextEdit();

protected:
    virtual void initialize(size_t bufSize);
    virtual void setEnvironment();

protected:                      // From std::streambuf
    virtual int sync();
    virtual int overflow(int c);

public:
    QString     commandLine;
    QString     cmd;
    QStringList args;
    QString     wd;
    AnsiTextEdit  *outTextEdit, *errTextEdit;
    void       *id;
    bool        aborted;
    QString     err, out;

protected:
    static ulong num;    // For debug traces only
};

}

#endif // PROCESS_H
