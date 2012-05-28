// ****************************************************************************
//  demangle.h                                                     Tao project 
// ****************************************************************************
// 
//   File Description:
// 
//     Demangle C++ symbol name
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#ifdef __GNUG__
#include <cxxabi.h>
#endif

inline text demangle(const char *symbol)
// ----------------------------------------------------------------------------
//    Demangle C++ symbol (GNU g++ only)
// ----------------------------------------------------------------------------
{
    text result(symbol);
#ifdef __GNUG__
    int status = 0;
    char *realname = abi::__cxa_demangle(symbol, 0, 0, &status);
    if (status == 0)
    {
        result = text(realname);
        free(realname);
    }
#endif
    return result;
}


