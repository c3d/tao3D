// *****************************************************************************
// regexp.cpp                                                      Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "regexp.h"
#include "tao/tao_utf8.h"
#include "tao/tao_info.h"
#include "tao/module_api.h"
#include "traces.h"
#include "save.h"
#include <QRegExp>

using namespace Tao;

QRegExp regexpCurrent;

// Pointer to Tao functions
const Tao::ModuleApi *tao = NULL;


Name_p regexpMatch(Context *, Tree *, text input, text pattern)
// ----------------------------------------------------------------------------
//    Return true if the input matches the pattern
// ----------------------------------------------------------------------------
{
    regexpCurrent = QRegExp(+pattern, Qt::CaseSensitive, QRegExp::RegExp2);
    int position = regexpCurrent.indexIn(+input);
    return position >= 0 ? xl_true : xl_false;
}


Integer_p regexpSearch(Context *, Tree *self, text input, text pattern)
// ----------------------------------------------------------------------------
//    Return the position of input in the pattern
// ----------------------------------------------------------------------------
{
    regexpCurrent = QRegExp(+pattern, Qt::CaseSensitive, QRegExp::RegExp2);
    int position = regexpCurrent.indexIn(+input);
    return new Integer(position, self->Position());
}


Tree_p regexpParse(Context *context,
                  Tree *self,
                  text input,
                  Tree *code,
                  RegExpMode mode)
// ----------------------------------------------------------------------------
//   Perform the parsing
// ----------------------------------------------------------------------------
{
    Symbols *symbols = self->Symbols();
    std::vector<QRegExp> patterns;
    std::vector<Tree_p>  codes;
    std::vector<int>     positions;

    // Save the current regexp
    Save<QRegExp> saveRegexp(regexpCurrent, QRegExp());

    // Skip block if any
    while (Block *block = code->AsBlock())
        code = block->child;

    // First analyze the code
    while (Infix *infix = code->AsInfix())
    {
        if (infix->name == "\n" || infix->name == ";")
        {
            code = infix->right;
            infix = infix->left->AsInfix();
            if (!infix)
                break;
        }
        else if (infix->name == "->")
        {
            code = xl_true;
        }

        if (infix->name == "->")
        {
            bool minimal = false;

            Tree *pat = infix->left;

            // Use -"Form" to have minimal matching
            if (Prefix *prefix = pat->AsPrefix())
                if (Name *name = prefix->left->AsName())
                    if (name->value == "-")
                        minimal = true, pat = prefix->right;

            if (Text *pattern = pat->AsText())
            {
                if (!infix->right->Symbols())
                    infix->right->SetSymbols(symbols);

                QRegExp re(+pattern->value,Qt::CaseSensitive,QRegExp::RegExp2);
                if (minimal)
                    re.setMinimal(true);

                patterns.push_back(re);
                codes.push_back(infix->right);
                positions.push_back(-2);
                continue;
            }
        }

        // All other cases break the loop
        break;
    }

    // Then find the best pattern
    bool running = true;
    Tree_p result = xl_nil;
    text resultText = "";
    int offset = 0;
    QString in = +input;
    TreeList list;

    while (running)
    {
        uint max = patterns.size();
        Tree_p found = xl_nil;
        uint best = ~0U;
        int bestPosition = input.length();
        bool keepRunning = mode != REGEXP_FIRST;

        // Search in all regexps, preferring the position cache if we have it
        for (uint i = 0; i < max; i++)
        {
            int pos = positions[i];
            if (pos < offset && pos != -1)
            {
                QRegExp &re = patterns[i];
                pos = re.indexIn(in, offset);
                positions[i] = pos;
            }
            if (pos >= 0 && pos < bestPosition)
            {
                best = i;
                bestPosition = pos;
            }
        }

        // Find the best candidate if there is one
        if (best != ~0U)
        {
            QRegExp &re = patterns[best];
            int pos = positions[best];
            offset = pos + re.matchedLength();
            Tree *toEval = codes[best];
            regexpCurrent = re;
            found = context->Evaluate(toEval);
        }

        // If not found, evaluate default code
        else
        {
            if (result == xl_nil)
                found = context->Evaluate(code);
            keepRunning = false;
        }

        // Build end result
        if (mode == REGEXP_TEXT)
        {
            if (keepRunning || result == xl_nil)
            {
                if (Text *t = found->AsText())
                    resultText += t->value;
                else
                    resultText += text(*found);
            }
            result = found;
        }
        else if (mode == REGEXP_LIST && result != xl_nil)
        {
            list.push_back(found);
        }
        else
        {
            result = found;
        }

        running = keepRunning;
    }

    if (mode == REGEXP_TEXT)
        result = new Text(resultText, "\"", "\"", self->Position());
    else if (mode == REGEXP_LIST)
        result = (XL::Infix*)xl_list_to_tree(list, ",");

    return result;
}


Text_p regexpAt(Context *, Tree *self, uint index)
// ----------------------------------------------------------------------------
//   Return the captured item n
// ----------------------------------------------------------------------------
{
    text captured = +regexpCurrent.cap(index);
    return new Text(captured, "\"", "\"", self->Position());
}


Integer_p regexpPos(Context *, Tree *self, uint index)
// ----------------------------------------------------------------------------
//   Return position at given index
// ----------------------------------------------------------------------------
{
    int pos = regexpCurrent.pos(index);
    return new Integer(pos, self->Position());
}


Integer_p regexpMatchedLength(Context *, Tree *self)
// ----------------------------------------------------------------------------
//   Return total matched length
// ----------------------------------------------------------------------------
{
    int length = regexpCurrent.matchedLength();
    return new Integer(length, self->Position());

}


Text_p regexpEscape(Context *, Tree *self, text toEscape)
// ----------------------------------------------------------------------------
//   Return input text, escaped so that it works in a regexp
// ----------------------------------------------------------------------------
{
    text escaped = +QRegExp::escape(+toEscape);
    return new Text(escaped, "\"", "\"", self->Position());
}



// ============================================================================
//
//    Module glue
//
// ============================================================================

int module_init(const Tao::ModuleApi *api, const Tao::ModuleInfo *)
// ----------------------------------------------------------------------------
//   Initialize the Tao module
// ----------------------------------------------------------------------------
{
    XL_INIT_TRACES();
    tao = api;
    return 0;
}


int module_exit()
// ----------------------------------------------------------------------------
//   Uninitialize the Tao module
// ----------------------------------------------------------------------------
{
    return 0;
}



// Define traces for this modules
XL_DEFINE_TRACES

