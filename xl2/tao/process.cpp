// ****************************************************************************
//  process.cpp                                                     XLR project
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "process.h"
#include "errors.h"
#include "options.h"

#include <cassert>
#include <QMessageBox>


TAO_BEGIN

Process::Process(size_t bufSize)
// ----------------------------------------------------------------------------
//   Create a QProcess without starting it yet
// ----------------------------------------------------------------------------
{
    initialize(bufSize);
}


Process::Process(const QString &cmd,
                 const QStringList &args,
                 const QString &wd,
                 size_t bufSize)
// ----------------------------------------------------------------------------
//   Create a QProcess and start it
// ----------------------------------------------------------------------------
{
    setWorkingDirectory(wd);
    initialize(bufSize);
    start(cmd, args);
}


Process::~Process()
// ----------------------------------------------------------------------------
//    Make sure we are done with all the data for that process
// ----------------------------------------------------------------------------
{
    done();
}


void Process::start(const QString &cmd, const QStringList &args)
// ----------------------------------------------------------------------------
//   Start child process
// ----------------------------------------------------------------------------
{
    IFTRACE(process)
    {
        QStringList::const_iterator it;
        std::cerr << "Process: " << cmd.toStdString();
        for (it = args.begin(); it != args.end(); it++)
            std::cerr << " " << (*it).toStdString();
        std::cerr << "\n";
    }

    QProcess::start(cmd, args);
}


bool Process::done(bool showErrors)
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
    if (!waitForFinished())
    {
        if (showErrors)
        {
            
        }
        else
        {
            IFTRACE(process)
            {
                QByteArray ba = readAll();
                QString s(ba);
                std::cerr << s.toStdString() << "\n";
            }
        }
        ok = false;
    }

    if (exitCode())
        ok = false;

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
    if (!error && c != EOF)
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

TAO_END
