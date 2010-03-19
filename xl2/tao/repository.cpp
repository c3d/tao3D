// ****************************************************************************
//  source_control.cpp                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//     Abstract interface describing an SCM repository for the document
//     Derived class will provide implementations for git, hg, bzr
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
//  (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "repository.h"
#include "renderer.h"
#include "parser.h"
#include "main.h"
#include "git_backend.h"

#include <QDir>
#include <QtGlobal>
#include <fstream>

TAO_BEGIN


text Repository::fullName(text fileName)
// ----------------------------------------------------------------------------
//   Return the full name of an element in the repository
// ----------------------------------------------------------------------------
{
    QDir dir(path);
    QString file = QString::fromUtf8(fileName.data(), fileName.length());
    QString fullPath = dir.filePath(file);
    return fullPath.toStdString();
}


text Repository::styleSheet()
// ----------------------------------------------------------------------------
//   Return the style sheet we use for this repository
// ----------------------------------------------------------------------------
{
    return "xl.stylesheet";
}


bool Repository::write(text fileName, XL::Tree *tree)
// ----------------------------------------------------------------------------
//   Write the text into a repository, ready to commit, return true if OK
// ----------------------------------------------------------------------------
{
    bool ok = false;
    text full = fullName(fileName);

    // Write the file in a copy (avoid overwriting original)
    text copy = full + "~";
    {
        std::ofstream output(copy.c_str());
        XL::Renderer renderer(output);
        renderer.SelectStyleSheet(styleSheet());
        renderer.Render(tree);
        output.flush();
        ok = output.good()  && !output.fail() && !output.bad();
    }

    // If we were successful writing, rename to new file
    if (ok)
        ok = std::rename(copy.c_str(), full.c_str()) == 0;

    return ok;
}


XL::Tree * Repository::read(text fileName)
// ----------------------------------------------------------------------------
//   Read a tree from a given file in the repository
// ----------------------------------------------------------------------------
{
    XL::Tree      *result    = NULL;
    text           full      = fullName(fileName);

    // Create the parser, with a local copy of the syntax (per-file syntax)
    XL::Syntax     syntax (XL::MAIN->syntax);
    XL::Positions &positions = XL::MAIN->positions;
    XL::Errors    &errors    = XL::MAIN->errors;
    XL::Parser     parser(full.c_str(), syntax, positions, errors);

    result = parser.Parse();
    return result;
}


bool Repository::setTask(text name)
// ----------------------------------------------------------------------------
//   Set the name of the current task
// ----------------------------------------------------------------------------
//   We use the task to identify two branches in the repository
//       <name> is the work branch itself, recording user-defined checkpoints
//       <name>.undo is the undo branch, managed by us
{
    text undo = name + TAO_UNDO_SUFFIX;
    task = name;

    // Check if we can checkout the task branch
    if (!checkout(task))
        if (!branch(task) || !checkout(task))
            return false;
    if (!commit("Automatic commit on Tao startup", true))
        return false;
    if (!merge(undo))
        return false;

    // Check if we can checkout the undo branch
    if (!checkout(undo))
        if (!branch(undo)||!checkout(undo))
            return false;
    if (!commit("Automatic commit on Tao startup", true))
        return false;
    if (!merge(task))
        return false;

    // We are now on the _undo branch
    return true;
}


Repository *Repository::repository(const QString &path)
// ----------------------------------------------------------------------------
//    Factory creating the right repository kind for the current directory
// ----------------------------------------------------------------------------
{
    // Try a Git repository first
    Repository *git = new GitRepository(path);
    if (git->valid())
        return git;
    delete git;

    // Didn't work, fail
    return NULL;
}


bool Repository::selectWorkBranch()
// ----------------------------------------------------------------------------
//    Select the work branch
// ----------------------------------------------------------------------------
{
    return checkout(task);
}


bool Repository::selectUndoBranch()
// ----------------------------------------------------------------------------
//    Select the undo branch
// ----------------------------------------------------------------------------
{
    return checkout(task + TAO_UNDO_SUFFIX);
}

TAO_END
