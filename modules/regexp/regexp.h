// ****************************************************************************
//  regexp.h                                                       Tao project 
// ****************************************************************************
// 
//   File Description:
// 
//     Module offering regular expressions in Tao
//
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
//  This software is property of Taodyne SAS - Confidential
//  Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "tree.h"
#include "main.h"
#include "runtime.h"
#include "base.h"

using namespace XL;

enum RegExpMode
{
    REGEXP_FIRST,
    REGEXP_LAST,
    REGEXP_LIST,
    REGEXP_TEXT
};

Name *regexpMatch(Context *, Tree *self, text input, text pattern);
Integer *regexpSearch(Context *, Tree *self, text input, text pattern);
Tree *regexpParse(Context *, Tree *self, text input, Tree *code, RegExpMode m);
Text *regexpAt(Context *, Tree *self, uint index);
Integer *regexpPos(Context *, Tree *self, uint index);
Integer *regexpMatchedLength(Context *, Tree *self);
Text *regexpEscape(Context *, Tree *self, text toEscape);
