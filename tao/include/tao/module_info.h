#ifndef TAO_MODULE_INFO_H
#define TAO_MODULE_INFO_H
// ****************************************************************************
//  module_info.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    Public interface to Tao module information
//
//
//
//
//
//
//
// ****************************************************************************
// This file may be used in accordance with the terms and conditions contained
// in the Tao3D license agreement, or alternatively, in a signed
// license agreement between you and Taodyne SAS.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <string>

namespace Tao {


struct ModuleInfo
// ------------------------------------------------------------------------
//   Information about a module
// ------------------------------------------------------------------------
{
    ModuleInfo() {}
    ModuleInfo(std::string id, std::string path)
        : id(id), path(path), ver(1.0), autoLoad(false) {}

    std::string id;
    std::string path;  // Qt format (e.g., "/Users/foo", "C:/Users/foo")
    std::string name;
    std::string desc;
    std::string icon;
    double      ver;
    std::string author;
    std::string website;
    std::string importName;
    bool        autoLoad;
    // Message supplied by the module as on_load_error, shown in case of
    // library load error
    std::string onLoadError;
    // Windows: value of %PATH% to be set to load the library (supplied by
    // the module as windows_load_path)
    std::string windowsLoadPath;
};

}

#endif // TAO_MODULE_INFO_H
