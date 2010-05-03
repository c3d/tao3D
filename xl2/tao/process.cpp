// ****************************************************************************
//  process.cpp                                                     Tao project
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

#include "process.h"
#include "tao_utf8.h"
#include "errors.h"
#include "options.h"

#include <cassert>
#include <QMessageBox>
#include <QApplication>


TAO_BEGIN

Process::Process(size_t bufSize)
// ----------------------------------------------------------------------------
//   Create a QProcess without starting it yet
// ----------------------------------------------------------------------------
    : commandLine(""), outTextEdit(NULL), errTextEdit(NULL), id(NULL),
      aborted(false)
{
    initialize(bufSize);
}


Process::Process(const QString &cmd,
                 const QStringList &args,
                 const QString &wd,
                 bool  startImmediately,
                 size_t bufSize)
// ----------------------------------------------------------------------------
//   Create a QProcess
// ----------------------------------------------------------------------------
    : commandLine(""), cmd(cmd), args(args), wd(wd),
      outTextEdit(NULL), errTextEdit(NULL), id(NULL), aborted(false)
{
    setWorkingDirectory(wd);
    initialize(bufSize);
    if (startImmediately)
        start();
}


Process::~Process()
// ----------------------------------------------------------------------------
//    Make sure we are done with all the data for that process
// ----------------------------------------------------------------------------
{
    if (pbase())
        done();
}


void Process::start(const QString &cmd, const QStringList &args,
                    const QString &wd)
// ----------------------------------------------------------------------------
//   Start child process
// ----------------------------------------------------------------------------
{
    commandLine = cmd + " " + args.join(" ");
    if (!wd.isEmpty())
        setWorkingDirectory(wd);

    IFTRACE(process)
        std::cerr << "Process: " << +commandLine
                  << " (wd " << +workingDirectory() << ")\n";

    QProcess::start(cmd, args);
}


void Process::start()
// ----------------------------------------------------------------------------
//   Start child process
// ----------------------------------------------------------------------------
{
    start(cmd, args, wd);
}


bool Process::done(text *errors, text *output)
// ----------------------------------------------------------------------------
//    Return true if the process was successful
// ----------------------------------------------------------------------------
{
    bool ok = true;

    // Flush streambuf
    if (pbase())
        if (sync())
            ok = false;
    delete[] pbase();
    setp(NULL, NULL);

    // Close QProcess
    closeWriteChannel();
    if (state() != NotRunning)
        if (!waitForFinished())
            ok = false;

    int rc = exitCode();
    IFTRACE(process)
        std::cerr << "Process: " << +commandLine << " returned " << rc << "\n";
    if (rc)
        ok = false;

    bool tracing = XLTRACE(process);
    if (!ok)
    {
        err = tr("Process '%1' terminated abormally "
                 "with exit code %2:\n%3")
            .arg(commandLine) .arg(rc) .arg(QString(readAll()));
    }
    if (output || tracing)
        out += QString(readAllStandardOutput());
    if (errors || tracing)
        err += QString(readAllStandardError());
    if (output)
        *output = +out;
    if (errors)
        *errors = +err;

    if (tracing)
    {
        if (err.length())
            std::cerr << "Process: Error output:\n" << +err;
        if (out.length())
            std::cerr << "Process: Standard output\n" << +out;
    }

    return ok;
}


void Process::initialize(size_t bufSize)
// ----------------------------------------------------------------------------
//   Initialize the process and streambuf
// ----------------------------------------------------------------------------
{
    // Allocate data to be used by the streambuf
    char *ptr = new char[bufSize];
    setp(ptr, ptr + bufSize);
}


void Process::setEnvironment()
// ----------------------------------------------------------------------------
//   Set environment variables that have an influence on child process
// ----------------------------------------------------------------------------
{
}


int Process::overflow(int c)
// ------------------------------------------------------------------------
//   Override std::streambuf::overflow() to pipe to process
// ------------------------------------------------------------------------
{
    int error = sync();
    if (!error && c != traits_type::eof())
    {
        assert(pbase() != epptr());
        error = sputc(c);
    }
    return error;
}


int Process::sync()
// ------------------------------------------------------------------------
//   Overriding std::streambuf::sync(), write data to process
// ------------------------------------------------------------------------
{
    if (pbase() != pptr())
    {
        QProcess::write(pbase(), pptr() - pbase());
        setp(pbase(), epptr());
    }
    return 0;
}


void Process::sendStandardOutputToTextEdit()
// ------------------------------------------------------------------------
//   Read+save stdout and send text to previously registered QTextEdit
// ------------------------------------------------------------------------
{
    if (!outTextEdit)
        return;
    QByteArray outB = readAllStandardOutput();
    IFTRACE(process)
        std::cerr << +QString(outB);
    outTextEdit->insertAnsiText(outB);
    out.append(outB);
}


void Process::sendStandardErrorToTextEdit()
// ------------------------------------------------------------------------
//   Read+save stderr and send text to previously registered QTextEdit
// ------------------------------------------------------------------------
{
    if (!errTextEdit)
        return;
    QByteArray errB = readAllStandardError();
    IFTRACE(process)
        std::cerr << +QString(errB);
    errTextEdit->insertAnsiText(errB);
    err.append(errB);
}

TAO_END
