// ****************************************************************************
//  info_trash_can.cpp                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//    A container for XL::Info objects. Used for deferred deletion.
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
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "info_trash_can.h"

TAO_BEGIN

std::vector<XL::Info *> InfoTrashCan::trash;

void InfoTrashCan::Empty()
// ------------------------------------------------------------------------
//   Empty the trash can
// ------------------------------------------------------------------------
{
    while (!trash.empty())
    {
        XL::Info *tmp = trash.back();
        trash.pop_back();
        delete tmp;
    }

}


void InfoTrashCan::DeferredDelete(XL::Info *obj)
// ------------------------------------------------------------------------
//   Mark object for deletion
// ------------------------------------------------------------------------
{
    trash.push_back(obj);
}

TAO_END
