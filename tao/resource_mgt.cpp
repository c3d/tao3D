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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "resource_mgt.h"
#include "widget.h"
#include "window.h"
#include "tao_utf8.h"

TAO_BEGIN

ResourceMgt::ResourceMgt(Widget *widget): TreeClone()
//-----------------------------------------------------------------------------
//  Resource management constructor.
//-----------------------------------------------------------------------------
{
    this->widget = widget;
    if (cmdFileList.empty())
    {
        cmdFileList["load"]      = std::pair<int, text>(1, "xl");
        cmdFileList["texture"]   = std::pair<int, text>(1, "texture");
        cmdFileList["svg"]       = std::pair<int, text>(1, "image");
        cmdFileList["image"]     = std::pair<int, text>(-1, "image");
        cmdFileList["menu_item"] = std::pair<int, text>(3, "icon");
        cmdFileList["menu"]      = std::pair<int, text>(3, "icon");
        cmdFileList["submenu"]   = std::pair<int, text>(3, "icon");
    }
    if (subDirList.empty())
    {
        subDirList["xl"]      = "";
        subDirList["image"]   = "images/";
        subDirList["texture"] = "images/";
        subDirList["icon"]    = "images/";
        subDirList["doc"]     = "images/";
    }
}


Tree *ResourceMgt::DoPrefix(Prefix *what)
//-----------------------------------------------------------------------------
//  The prefix action
//-----------------------------------------------------------------------------
{
    Name * n = what->left->AsName();
    if (!n || cmdFileList.count(n->value) == 0)
        return TreeClone::DoPrefix(what);

    Text * arg = getArg(what, cmdFileList[n->value].first);
    if (! arg)
        return TreeClone::DoPrefix(what);

    QFileInfo info(+(arg->value));
    if (! isToBeMoved(info,+(cmdFileList[n->value].second)))
    {
        usedFile.insert(+(info.filePath()));
        IFTRACE(resources)
        {
            std::cerr << "Used file: "<< +(info.filePath()) << std::endl;
        }

        return TreeClone::DoPrefix(what);
    }

    if (movedFiles.count(+(info.canonicalFilePath())))
        arg->value = movedFiles[+(info.canonicalFilePath())];
    else
    {
        arg->value = integrateFile(info, +(cmdFileList[n->value].second));
    }


    return TreeClone::DoPrefix(what);
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
        IFTRACE(resources)
        {
            std::cerr << "Return last arg: "<<inf->right->AsText()<< std::endl;
        }
        return inf->right->AsText();
    }

    // Loop to get the required argument (inf->left is already the first one)
    Infix *last = inf;
    Infix *next = NULL;
    while ( (--pos > 1) &&
            (next = last->left->AsInfix()) &&
            (next->name == "," ))
    {
        last = next;
    }

    IFTRACE(resources)
    {
        std::cerr << "pos is: "<<pos<< std::endl;
    }
    if (next && (next->name ==  "\n" || next->name == ";"))
    {
        IFTRACE(resources)
        {
            std::cerr << "returning next->right: "<< next->right->AsText()
                    << std::endl;
        }
        return next->right->AsText();
    }

    IFTRACE(resources)
         std::cerr << "returning last->right: "<< last->right->AsText()
                << std::endl;

    return last->right->AsText();
}


bool ResourceMgt::isToBeMoved(QFileInfo &info, QString prefix)
//-----------------------------------------------------------------------------
//  Check if the given file name is in or under the project folder.
//-----------------------------------------------------------------------------
{
    Window * w = (Window*)widget->parent();

    QDir dir(w->currentProjectFolderPath());

    if (info.isRelative())
        info.setFile(prefix+':'+info.filePath());

    if (! info.exists())
        return false;

    QString relName = dir.relativeFilePath(info.absoluteFilePath());

    return relName.contains("..");
}


text ResourceMgt::integrateFile(QFileInfo info, QString prefix)
//-----------------------------------------------------------------------------
//  Compute the new name and copy the file
//-----------------------------------------------------------------------------
{
    QString subdir(+(subDirList[+(prefix)]));
    Window * w = (Window*)widget->parent();
    QDir projdir(w->currentProjectFolderPath());

    QFileInfo newName(projdir, subdir+info.fileName());
    int i=0;
    while (++i, newName.exists())
    {
        newName = QFileInfo(projdir,
                            subdir
                            + info.completeBaseName()
                            +'_'
                            + QString::number(i)
                            + '.' +info.suffix());
    }
    if (!subdir.isEmpty())
        QDir(w->currentProjectFolderPath()).mkpath(subdir);

    IFTRACE(resources)
    {
        std::cerr << "Copying "<< +(info.canonicalFilePath()) <<
                " into " << +(newName.filePath()) << std::endl;
    }
    QFile::copy(info.canonicalFilePath(), newName.filePath());

    text relName = +(prefix + ':' + newName.fileName());
    movedFiles[+(info.canonicalFilePath())] = relName;
    newName.refresh();
    usedFile.insert(+(projdir.relativeFilePath(newName.canonicalFilePath())));
    IFTRACE(resources)
    {
        std::cerr << " usedFile.insert("<<
                +(projdir.relativeFilePath(newName.canonicalFilePath()))
                <<")"<< std::endl;
        std::cerr << "newName.canonicalFilePath() "
                << +(newName.canonicalFilePath()) << std::endl;
    }

#ifndef CFG_NOGIT
    Repository * repo = ((Window*)widget->parent())->repository();
    if (repo)
        repo->add(relName);
#endif

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
std::map < text, text > ResourceMgt::subDirList;

TAO_END
