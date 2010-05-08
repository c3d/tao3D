#ifndef MAIN_H
#define MAIN_H
// ****************************************************************************
//  main.h                          (C) 1992-2009 Christophe de Dinechin (ddd)
//                                                                 XL2 project
// ****************************************************************************
//
//   File Description:
//
//     The global variables defined in main.cpp
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
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "tree.h"
#include "scanner.h"
#include "parser.h"
#include "renderer.h"
#include "errors.h"
#include "syntax.h"
#include "context.h"
#include "compiler.h"
#include "options.h"
#include <map>
#include <time.h>


XL_BEGIN

struct Serializer;
struct Deserializer;


struct SourceFile
// ----------------------------------------------------------------------------
//    A source file and associated data
// ----------------------------------------------------------------------------
{
    SourceFile(text n, Tree * t, Symbols *s);
    SourceFile();
    text        name;
    Tree_p      tree;
    Symbols_p   symbols;
    time_t      modified;
    bool        changed;
    text        hash;
};
typedef std::map<text, SourceFile> source_files;
typedef std::vector<text> source_names;


struct Main
// ----------------------------------------------------------------------------
//    The main entry point and associated data
// ----------------------------------------------------------------------------
{
    Main(int argc, char **argv, Compiler &comp);
    Main(int inArgc, char **inArgv, Compiler &comp,
         text builtinsPath, text syntaxPath, text stylesheetPath);
    ~Main();

    int ParseOptions();
    int LoadContextFiles();
    void EvalContextFiles();
    int LoadFiles();
    int LoadFile(text file, bool updateContext = false);
    int Run();
    int Diff();

public:
    int          argc;
    char **      argv;

    Positions    positions;
    Errors       errors;
    Syntax       syntax;
    text         builtins;
    Options      options;
    Compiler    &compiler;
    Context      context;
    Renderer     renderer;
    source_files files;
    source_names file_names;
    source_names context_file_names;
    Deserializer *reader;
    Serializer   *writer;

};

extern Main *MAIN;

XL_END

#endif // MAIN_H
