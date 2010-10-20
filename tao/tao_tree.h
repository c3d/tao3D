#ifndef TAO_TREE_H
#define TAO_TREE_H
// ****************************************************************************
//  tao_tree.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//     Make the various XL tree types visible in Tao namespace
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tree.h"
#include "opcodes.h"
#include "tao.h"
#include "main.h"


TAO_BEGIN

typedef XL::Tree           Tree;
typedef XL::Tree_p         Tree_p;
typedef XL::Integer        Integer;
typedef XL::Integer_p      Integer_p;
typedef XL::Real           Real;
typedef XL::Real_p         Real_p;
typedef XL::Text           Text;
typedef XL::Text_p         Text_p;
typedef XL::Name           Name;
typedef XL::Name_p         Name_p;
typedef XL::Infix          Infix;
typedef XL::Infix_p        Infix_p;
typedef XL::Prefix         Prefix;
typedef XL::Prefix_p       Prefix_p;
typedef XL::Postfix        Postfix;
typedef XL::Postfix_p      Postfix_p;
typedef XL::Block          Block;
typedef XL::Block_p        Block_p;

typedef XL::real_r         real_r;
typedef XL::integer_r      integer_r;
typedef XL::text_r         text_r;
typedef XL::symbol_r       symbol_r;

typedef XL::real_p         real_p;
typedef XL::integer_p      integer_p;
typedef XL::text_p         text_p;
typedef XL::symbol_p       symbol_p;

typedef XL::Action         Action;
typedef XL::TreeList       TreeList;
typedef XL::Context        Context;
typedef XL::Context_p      Context_p;

typedef XL::SourceFile     SourceFile;

TAO_END

#endif // TAO_TREE_H
