#ifndef TAOTESTER_H
#define TAOTESTER_H

#include "main.h"

#include "widgettests.h"

using namespace XL;

class taoTester
{
private:
    taoTester();
    //Tests // CaB
public:
    // Commands for toolbar
    Tree_p startRecTest(Tree_p self);
    Tree_p stopRecTest(Tree_p self);
    Tree_p playTest(Tree_p self);
    Tree_p saveTest(Tree_p self);
    Tree_p resetTest(Tree_p self);
    Tree_p testCheck(Tree_p self);

    // Definition of events to be load
    Tree_p testDef(Tree_p self, Text_p name, Integer_p fId, Text_p desc, Tree_p body,
                   Real_p thr, Integer_p width, Integer_p height);
    Tree_p testAddKeyPress(Tree_p self, Integer_p key,
                           Integer_p modifiers, Integer_p delay );
    Tree_p testAddKeyRelease(Tree_p self, Integer_p key,
                             Integer_p modifiers, Integer_p delay );
    Tree_p testAddMousePress(Tree_p self, Integer_p button, Integer_p modifiers,
                             Integer_p x, Integer_p y, Integer_p delay);
    Tree_p testAddMouseRelease(Tree_p self, Integer_p button,
                               Integer_p modifiers,
                               Integer_p x, Integer_p y, Integer_p delay);
    Tree_p testAddMouseDClick(Tree_p self, Integer_p button, Integer_p modifiers,
                              Integer_p x, Integer_p y, Integer_p delay);
    Tree_p testAddMouseMove(Tree_p self, Integer_p button, Integer_p modifiers,
                            Integer_p x, Integer_p y, Integer_p delay);
    Tree_p testAddAction(Tree_p self, Text_p name, Integer_p delay);
    Tree_p testAddCheck(Tree_p self, Integer_p num, Integer_p delay );
    Tree_p testAddFont(Tree_p self, Text_p diagname, Text_p ftname, Integer_p delay);
    Tree_p testAddColor(Tree_p self, Text_p diagname, Text_p colname, Integer_p delay);
    Tree_p testAddCloseDialog(Tree_p self, Text_p diagname,
                              Integer_p result, Integer_p delay);

    static inline taoTester *tester()
    {
        if (!testWrap)
            testWrap = new taoTester();

        return testWrap;
    }
    inline WidgetTests *currentTest()
    {
        if (! current)
        {
            current = new WidgetTests();
        }
        return current;
    }


private:
    WidgetTests *current;
    static taoTester *testWrap;

};

#endif // TAOTESTER_H
