// ****************************************************************************
//  resource_mgt.cpp						   Tao project
// ****************************************************************************
//
//   File Description:
//
//     Manage resource around the main document.
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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "resource_mgt.h"
#include "widget.h"
#include "window.h"
#include "tao_utf8.h"

TAO_BEGIN

ResourceMgt::ResourceMgt(Widget *widget): TaoTreeClone(widget)
//-----------------------------------------------------------------------------
//  Resource management constructor.
//-----------------------------------------------------------------------------
{
    if (cmdFileList.empty())
    {
        cmdFileList["load"]      = std::pair<int, text>(1, "");
        cmdFileList["texture"]   = std::pair<int, text>(1, "images/");
        cmdFileList["svg"]       = std::pair<int, text>(1, "images/");
        cmdFileList["image"]     = std::pair<int, text>(-1, "images/");
        cmdFileList["menu_item"] = std::pair<int, text>(3, "images/");
        cmdFileList["menu"]      = std::pair<int, text>(3, "images/");
        cmdFileList["submenu"]   = std::pair<int, text>(3, "images/");
    }
}


Tree *ResourceMgt::DoPrefix(Prefix *what)
//-----------------------------------------------------------------------------
//  The prefix action
//-----------------------------------------------------------------------------
{
    Name * n = what->left->AsName();
    if (!n || cmdFileList.count(n->value) == 0)
        return TaoTreeClone::DoPrefix(what);

    Text * arg = getArg(what, cmdFileList[n->value].first);
    if (! arg)
        return TaoTreeClone::DoPrefix(what);

    QFileInfo info(+(arg->value));
    if ( ! info.exists() || isLocalToDoc(info))
    {
        usedFile.insert(+(info.filePath()));
        return TaoTreeClone::DoPrefix(what);
    }

    arg->value = integrateFile(info, +(cmdFileList[n->value].second));
    usedFile.insert(arg->value);

    return TaoTreeClone::DoPrefix(what);
}


Text * ResourceMgt::getArg(Prefix * what, int pos)
//-----------------------------------------------------------------------------
//  return the pos'th argument of the 'what' command
//-----------------------------------------------------------------------------
{
    Infix * inf = what->right->AsInfix();
    // If the right is not an infix, it is the only argument so return it.
    if (!inf)
    {
        return what->right->AsText();
    }

    if (pos <= 0)
    {
        // pos <= 0 means the latest parameter
        inf = inf->LastStatement();

        return inf->right->AsText();
    }

    // loop to get the required argument (inf->left is already the first one)
    Infix *last = inf;
    Infix *next;
    while ( (--pos > 0) &&
            (next = last->right->AsInfix()) &&
            (next->name == "," ))
    {
        last = next;
    }

    if (next->name ==  "\n" || next->name == ";")
    {
        return next->left->AsText();
    }

    return last->left->AsText();
}


bool ResourceMgt::isLocalToDoc(QFileInfo info)
//-----------------------------------------------------------------------------
//  Check if the given file name is in or under the project folder.
//-----------------------------------------------------------------------------
{
    Window * w = (Window*)widget->parent();

    QDir dir(w->currentProjectFolderPath());
    QString relName = dir.relativeFilePath(info.absoluteFilePath());

    return ! relName.contains("..");
}


text ResourceMgt::integrateFile(QFileInfo info, QString subDir)
//-----------------------------------------------------------------------------
//  Compute the new name and copy the file
//-----------------------------------------------------------------------------
{
    Window * w = (Window*)widget->parent();
    QDir projdir(w->currentProjectFolderPath());

    QFileInfo newName(projdir, subDir+info.fileName());
    int i=0;
    while (++i, newName.exists())
    {
        newName = QFileInfo(projdir,
                            subDir
                            + info.completeBaseName()
                            + QString::number(i)
                            + "." +info.suffix());
    }

    QDir(w->currentProjectFolderPath()).mkpath(subDir);

    QFile::copy(info.absoluteFilePath(), newName.absoluteFilePath());

    text relName = +(projdir.relativeFilePath(newName.filePath()));

    Repository * repo = ((Window*)widget->parent())->repository();
    if (repo)
        repo->add(relName);

    return relName;
}


void ResourceMgt::cleanUpRepo()
//-----------------------------------------------------------------------------
//  Cleanup the Git repository
//-----------------------------------------------------------------------------
{
    // FIXME
}


std::map<text, std::pair < int, text > > ResourceMgt::cmdFileList;


TAO_END
