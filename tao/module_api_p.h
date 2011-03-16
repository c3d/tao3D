#ifndef TAO_MODULE_API_P_H
#define TAO_MODULE_API_P_H
// ****************************************************************************
//  module_api_p.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//    Construct a ModuleApi structure exported to native modules
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tao/module_api.h"

TAO_BEGIN

struct ModuleApiPrivate : ModuleApi
{
    ModuleApiPrivate();
};

TAO_END

#endif // TAO_MODULE_API_P_H
