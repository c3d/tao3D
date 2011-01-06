#include "taotester.h"
#include "runtime.h"

#include <QDir>
using namespace XL;


XL_DEFINE_TRACES

taoTester *taoTester::testWrap = NULL;


taoTester::taoTester() : current(NULL)
{
}

// ============================================================================
//
//   Tests functions
//
// ============================================================================


Tree_p taoTester::startRecTest(Tree_p )
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    currentTest()->startRecord();
    return XL::xl_true;
}


Tree_p taoTester::stopRecTest(Tree_p )
// ----------------------------------------------------------------------------
//   Stop recording events
// ----------------------------------------------------------------------------
{
    currentTest()->stopRecord();
    return XL::xl_true;
}


Tree_p taoTester::playTest(Tree_p )
// ----------------------------------------------------------------------------
//   Replay the test
// ----------------------------------------------------------------------------
{
    bool res = currentTest()->play();
    currentTest()->printResult();
    return  res ? XL::xl_true : XL::xl_false;
}


Tree_p taoTester::resetTest(Tree_p)
 // ----------------------------------------------------------------------------
 //   Reset current test
 // ----------------------------------------------------------------------------
{
    currentTest()->stopRecord();
    currentTest()->reset();
    return XL::xl_true;
}


Tree_p taoTester::saveTest(Tree_p)
// ----------------------------------------------------------------------------
//   Save the named test
// ----------------------------------------------------------------------------
{
    currentTest()->save();
    return XL::xl_true;
}


Tree_p taoTester::testCheck(Tree_p)
// ----------------------------------------------------------------------------
//  Insert a check point in the current registering test event list.
// ----------------------------------------------------------------------------
{
    currentTest()->checkNow();
    return XL::xl_true;
}


Tree_p taoTester::testDef(Tree_p , Text_p name, Integer_p fId, Text_p desc,
                          Tree_p body, Real_p thr, Integer_p width, Integer_p height)
// ----------------------------------------------------------------------------
//   Define a new test
// ----------------------------------------------------------------------------
{
    QStringList dirList = QDir::searchPaths("image");
    currentTest()->reset(name->value, fId->value, desc->value, +dirList.at(1),
                         thr->value, width->value, height->value);
    return xl_evaluate(body);
}


Tree_p taoTester::testAddKeyPress(Tree_p , Integer_p key,
                                  Integer_p modifiers, Integer_p delay )
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addKeyPress((Qt::Key)key->value,
                            (Qt::KeyboardModifiers)modifiers->value,
                            delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddKeyRelease(Tree_p , Integer_p key,
                                    Integer_p modifiers, Integer_p delay )
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addKeyRelease((Qt::Key)key->value,
                              (Qt::KeyboardModifiers)modifiers->value,
                              delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddMousePress(Tree_p , Integer_p button, Integer_p modifiers,
                                    Integer_p x, Integer_p y, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addMousePress((Qt::MouseButton)button->value,
                              (Qt::KeyboardModifiers)modifiers->value,
                              QPoint(x->value, y->value),
                              delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddMouseRelease(Tree_p , Integer_p button,
                                      Integer_p modifiers,
                                      Integer_p x, Integer_p y, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addMouseRelease((Qt::MouseButton)button->value,
                                (Qt::KeyboardModifiers)modifiers->value,
                                QPoint(x->value, y->value),
                                delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddMouseDClick(Tree_p , Integer_p button, Integer_p modifiers,
                                     Integer_p x, Integer_p y, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addMouseDClick((Qt::MouseButton)button->value,
                               (Qt::KeyboardModifiers)modifiers->value,
                               QPoint(x->value, y->value),
                               delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddMouseMove(Tree_p , Integer_p button, Integer_p modifiers,
                                   Integer_p x, Integer_p y,
                                   Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addMouseMove((Qt::MouseButton)button->value,
                             (Qt::KeyboardModifiers)modifiers->value,
                             QPoint(x->value, y->value),
                             delay->value);
   return XL::xl_true;
}


Tree_p taoTester::testAddAction(Tree_p , Text_p name, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addAction(+name->value, delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddCheck(Tree_p , Integer_p num, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a check view event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addCheck(num->value, delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddFont(Tree_p , Text_p name, Text_p ftname, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a font change event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addColor(+name->value, +ftname->value, delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddColor(Tree_p , Text_p name, Text_p colname, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a color change event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addFont(+name->value, +colname->value, delay->value);
    return XL::xl_true;
}


Tree_p taoTester::testAddCloseDialog(Tree_p , Text_p diagname,
                                     Integer_p result, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a close dialog box event to the current test
// ----------------------------------------------------------------------------
{
    currentTest()->addDialogClose(+diagname->value, result->value, delay->value);
    return XL::xl_true;
}


