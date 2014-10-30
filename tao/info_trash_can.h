#ifndef INFO_TRASH_CAN_H
#define INFO_TRASH_CAN_H
// ****************************************************************************
//  info_trash_can.h                                              Tao project
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

#include "tao.h"
#include "tree.h"


TAO_BEGIN

struct InfoTrashCan
{
public:
    static void Empty();
    static void DeferredDelete(XL::Info *obj);

protected:
    static std::vector<XL::Info *> trash;
};

TAO_END

#endif // INFO_TRASH_CAN_H
