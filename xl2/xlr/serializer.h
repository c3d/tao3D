#ifndef SERIALIZER_H
#define SERIALIZER_H
// ****************************************************************************
//  serializer.h                                                    XLR project
// ****************************************************************************
//
//   File Description:
//
//     A couple of classes used to serialize and read-back XL trees
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "base.h"
#include "tree.h"
#include <iostream>


XL_BEGIN

enum SerializationTag
// ----------------------------------------------------------------------------
//   Kind used for serialization (numerically independent from Tree kind)
// ----------------------------------------------------------------------------
{
    serialNULL,                 // NULL tree

    serialINTEGER, serialREAL, serialTEXT, serialNAME,
    serialBLOCK, serialPREFIX, serialPOSTFIX, serialINFIX,
    serialINVALID,

    serialVERSION = 0x0101,
    serialMAGIC   = 0x05121968
};


typedef std::map<text, longlong>        text_map;
typedef std::map<longlong, text>        text_ids;


struct Serializer : Action
// ----------------------------------------------------------------------------
//    Serialize a tree to a stream
// ----------------------------------------------------------------------------
{
    Serializer(std::ostream &out);
    ~Serializer() {}

    // Serialization of the canonical nodes
    Tree_p      DoInteger(Integer_p what);
    Tree_p      DoReal(Real_p what);
    Tree_p      DoText(Text_p what);
    Tree_p      DoName(Name_p what);
    Tree_p      DoPrefix(Prefix_p what);
    Tree_p      DoPostfix(Postfix_p what);
    Tree_p      DoInfix(Infix_p what);
    Tree_p      DoBlock(Block_p what);
    Tree_p      DoChild(Tree_p child);
    Tree_p      Do(Tree_p what);

    bool        IsValid()       { return out.good(); }

protected:
    // Writing data (low level)
    void        WriteSigned(longlong);
    void        WriteUnsigned(ulonglong);
    void        WriteReal(double);
    void        WriteText(text);
    void        WriteChild(Tree_p child);

protected:
    std::ostream &      out;
    text_map            texts;
};


struct Deserializer
// ----------------------------------------------------------------------------
//   Reconstruct a tree from its serialized form
// ----------------------------------------------------------------------------
{
    Deserializer(std::istream &in, TreePosition pos = Tree::NOWHERE);
    ~Deserializer();

    // Deserialize a tree from the input and return it, or return NULL
    Tree_p      ReadTree();
    bool        IsValid()       { return in.good(); }

protected:
    // Reading low-level data
    longlong    ReadSigned();
    ulonglong   ReadUnsigned();
    double      ReadReal();
    text        ReadText();

protected:
    std::istream &      in;
    TreePosition        pos;
    text_ids            texts;
};

XL_END

#endif // SERIALIZE_H
