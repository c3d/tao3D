#ifndef TAO_MAIN_H
#define TAO_MAIN_H
// ****************************************************************************
//  tao_main.h                                                      Tao project
// ****************************************************************************
// 
//   File Description:
// 
// 
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "main.h"
#include <signal.h>

TAO_BEGIN

struct Main : public XL::Main
// ----------------------------------------------------------------------------
//   Customization of the Main class for Tao
// ----------------------------------------------------------------------------
{
    Main(int argc, char **argv, text name = "xl_tao",
         text syntax = "xl.syntax",
         text style = "xl.stylesheet",
         text builtins = "builtins.xl")
         : XL::Main(argc, argv, name, syntax, style, builtins)
    {
        MAIN = this;
    }

    virtual text SearchFile(text input);
    virtual bool Refresh(double delay);

    static Main *MAIN;
};

TAO_END

extern void signal_handler(int sig);
extern void install_signal_handler(sig_t);

#endif // TAO_MAIN_H
