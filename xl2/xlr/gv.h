#ifndef GV_H
#define GV_H
// ****************************************************************************
//  gv.h                                                           XLR project
// ****************************************************************************
//
//   File Description:
//
//     Definition of a class to output a tree in Graphviz DOT format.
//       http://www.graphviz.org
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tree.h"
#include <stdio.h>


XL_BEGIN

struct GvOutput : Action
// ----------------------------------------------------------------------------
//   An action to recursively dump a tree in Graphviz DOT format
// ----------------------------------------------------------------------------
{
    GvOutput(std::ostream &out): out(out)
    {
        out << "graph {\nnode [style=filled];\n";
    }
    virtual ~GvOutput() { out << "\n};\n"; }


    struct GvNodeName
    // ------------------------------------------------------------------------
    //   Utility class to format a graph node name
    // ------------------------------------------------------------------------
    {
        GvNodeName(XL::Tree_p t) : t(t) {}
        std::ostream &operator()(std::ostream &out) const
        {
            char buf[10];
            snprintf(buf, 10, "n%lx", (long)t);
            out << buf;
            return out;
        };
        XL::Tree_p  t;
    };


    struct Id
    // ------------------------------------------------------------------------
    //   Utility class to format a node ID
    // ------------------------------------------------------------------------
    {
        Id(XL::Tree_p t) : t(t) {}
        std::ostream &operator()(std::ostream &out) const
        {
            bool present = t->Exists<NodeIdInfo>();
            if (present)
                out << t->Get<NodeIdInfo>() << "\\n";
            return out;
        };
        XL::Tree_p  t;
    };

    Tree_p DoInteger(Integer_p what);
    Tree_p DoReal(Real_p what);
    Tree_p DoText(Text_p what);
    Tree_p DoName(Name_p what);
    Tree_p DoBlock(Block_p what);
    Tree_p DoInfix(Infix_p what);
    Tree_p DoPrefix(Prefix_p what);
    Tree_p DoPostfix(Postfix_p what);
    Tree_p Do(Tree_p) { return NULL; }

    std::ostream &out;
};


std::ostream &operator<<(std::ostream &out, XL::GvOutput::GvNodeName gvnn)
{
    return gvnn(out);
}

std::ostream &operator<<(std::ostream &out, XL::GvOutput::Id id)
{
    return id(out);
}

Tree_p GvOutput::DoInteger(Integer_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=orange, label=\""
		<< Id(what) << "[Integer]\\n" << what->value << "\"]\n";
	return NULL;
}

Tree_p GvOutput::DoReal(Real_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=lightblue, label=\""
        << Id(what) << "[Real]\\n" << what->value << "\"]\n";
	return NULL;
}

Tree_p GvOutput::DoText(Text_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=darkorange1, label=\""
	    << Id(what) << "[Text]\\n" << what->value << "\"]\n";
	return NULL;
}

Tree_p GvOutput::DoName(Name_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=gold1, label=\""
	    << Id(what) << "[Name]\\n" << what->value << "\"]\n";
	return NULL;
}

Tree_p GvOutput::DoBlock(Block_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=darkolivegreen3, label=\""
	    << Id(what) << "[Block]\\n" << what->opening
	    << " " << what->closing << "\"]\n"
		<< GvNodeName(what) << " -- ";
	what->child->Do(this);
	return NULL;
}

Tree_p GvOutput::DoInfix(Infix_p what)
{
	std::string name = (!what->name.compare("\n")) ?  "<CR>" : what->name;
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=darkolivegreen4, label=\""
	    << Id(what) << "[Infix]\\n" << name << "\"]\n"
		<< GvNodeName(what) << " -- ";
	what->left->Do(this);
	out << GvNodeName(what) << " -- ";
	what->right->Do(this);
	return NULL;
}

Tree_p GvOutput::DoPrefix(Prefix_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=greenyellow, label=\""
	    << Id(what) << "[Prefix]\"]\n"
		<< GvNodeName(what) << " -- ";
	what->left->Do(this);
	out << GvNodeName(what) << " -- ";
	what->right->Do(this);
	return NULL;
}

Tree_p GvOutput::DoPostfix(Postfix_p what)
{
	out << GvNodeName(what) << "\n"
	    << GvNodeName(what) << " [color=aquamarine2, label=\""
	    << Id(what) << "[Postfix]\"]\n"
	    << GvNodeName(what) << " -- ";
	what->left->Do(this);
	out << GvNodeName(what) << " -- ";
	what->right->Do(this);
	return NULL;
}

XL_END

#endif // GV_H
