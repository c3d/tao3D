// ****************************************************************************
//  regexp.cpp                                                     Tao project
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

#include "regexp.h"
#include "tao/tao_utf8.h"
#include "tao/tao_info.h"
#include "tao/module_api.h"
#include "traces.h"
#include "save.h"
#include <QRegExp>

using namespace Tao;

QRegExp regexpCurrent;


Name *regexpMatch(Context *, Tree *, text input, text pattern)
// ----------------------------------------------------------------------------
//    Return true if the input matches the pattern
// ----------------------------------------------------------------------------
{
    regexpCurrent = QRegExp(+pattern, Qt::CaseSensitive, QRegExp::RegExp2);
    int position = regexpCurrent.indexIn(+input);
    return position >= 0 ? xl_true : xl_false;
}


Integer *regexpSearch(Context *, Tree *self, text input, text pattern)
// ----------------------------------------------------------------------------
//    Return the position of input in the pattern
// ----------------------------------------------------------------------------
{
    regexpCurrent = QRegExp(+pattern, Qt::CaseSensitive, QRegExp::RegExp2);
    int position = regexpCurrent.indexIn(+input);
    return new Integer(position, self->Position());
}


Tree *regexpParse(Context *context,
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
    Tree *result = xl_nil;
    text resultText = "";
    int offset = 0;
    QString in = +input;

    while (running)
    {
        uint max = patterns.size();
        Tree *found = xl_nil;
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
            result = new Infix(",", result, found, self->Position());
        }
        else
        {
            result = found;
        }

        running = keepRunning;
    }

    if (mode == REGEXP_TEXT)
        result = new Text(resultText, "\"", "\"", self->Position());

    return result;
}


Text *regexpAt(Context *, Tree *self, uint index)
// ----------------------------------------------------------------------------
//   Return the captured item n
// ----------------------------------------------------------------------------
{
    text captured = +regexpCurrent.cap(index);
    return new Text(captured, "\"", "\"", self->Position());
}


Integer *regexpPos(Context *, Tree *self, uint index)
// ----------------------------------------------------------------------------
//   Return position at given index
// ----------------------------------------------------------------------------
{
    int pos = regexpCurrent.pos(index);
    return new Integer(pos, self->Position());
}


Integer *regexpMatchedLength(Context *, Tree *self)
// ----------------------------------------------------------------------------
//   Return total matched length
// ----------------------------------------------------------------------------
{
    int length = regexpCurrent.matchedLength();
    return new Integer(length, self->Position());
   
}


Text *regexpEscape(Context *, Tree *self, text toEscape)
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

int module_init(const Tao::ModuleApi *, const Tao::ModuleInfo *)
// ----------------------------------------------------------------------------
//   Initialize the Tao module
// ----------------------------------------------------------------------------
{
    XL_INIT_TRACES();
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

