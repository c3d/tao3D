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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "info_trash_can.h"

TAO_BEGIN

std::vector<XL::Info *> InfoTrashCan::trash;

TAO_END
