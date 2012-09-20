// ****************************************************************************
//  taotester.cpp						    Tao project
// ****************************************************************************
//
//   File Description:
//
//     The test tools
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "taotester.h"
#include "utf8.h"
#include "widgettests.h"
#include "runtime.h"

#include "../tao_synchro/tao_control_event.h"
#include "../tao_synchro/event_capture.h"


#include <QFileInfo>
using namespace XL;

XL_DEFINE_TRACES

// ============================================================================
//
//   Test recorder functions
//
// ============================================================================


TestRecorder * recorder()
{
    return (TestRecorder * )synchroBasic::base->tao_event_handler;
}


Tree_p startRecTest(Tree_p )
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    if (synchroBasic::base)
    {
        synchroBasic::base->stop();
        delete synchroBasic::base;
    }
    EventCapture *currentCapture = new EventCapture(new TestRecorder());
    synchroBasic::base = currentCapture;
    currentCapture->startCapture();
    return XL::xl_true;
}


Tree_p testCheck(Tree_p)
// ----------------------------------------------------------------------------
//  Insert a check point in the current registering test event list.
// ----------------------------------------------------------------------------
{
    if (!synchroBasic::base) return XL::xl_false;

    recorder()->checkNow();
    return XL::xl_true;
}


Tree_p saveTest(Tree_p)
// ----------------------------------------------------------------------------
//   Save the named test
// ----------------------------------------------------------------------------
{
    if (!synchroBasic::base) return XL::xl_false;

    recorder()->save();
    return XL::xl_true;
}


// ============================================================================
//
//   Test recorder and player common functions
//
// ============================================================================


Tree_p stop(Tree_p )
// ----------------------------------------------------------------------------
//   Stop recording events
// ----------------------------------------------------------------------------
{
    if (!synchroBasic::base) return XL::xl_false;

    synchroBasic::base->stop();
    return XL::xl_true;
}


// ============================================================================
//
//   Test player functions
//
// ============================================================================

TestPlayer * player()
{
    if (!synchroBasic::base)
        synchroBasic::base = new EventClient( new TestPlayer() );

    return (TestPlayer * )synchroBasic::base->tao_event_handler;
}


Tree_p playTest(Tree_p, bool makeRef )
// ----------------------------------------------------------------------------
//   Replay the test in make ref or make check
// ----------------------------------------------------------------------------
{
    player()->makeRef = makeRef;
    EventClient *currentClient = (EventClient *)synchroBasic::base;
    currentClient->startClient();
    return XL::xl_true;
}


Tree_p resetTest(Tree_p)
 // ----------------------------------------------------------------------------
 //   Reset current test
 // ----------------------------------------------------------------------------
{
    if (!synchroBasic::base) return XL::xl_false;
    synchroBasic::base->stop();
    if (WidgetTests *t =
            dynamic_cast<WidgetTests *>(synchroBasic::base->tao_event_handler))
        t->reset();
    return XL::xl_true;
}


// ============================================================================
//
//   Loading functions
//
// ============================================================================


Tree_p testDef(Context *context,
               Tree_p, Text_p name, Integer_p fId, Text_p desc,
               Tree_p body, Integer_p width, Integer_p height)
// ----------------------------------------------------------------------------
//   Define a new test
// ----------------------------------------------------------------------------
{
    player()->reset(name->value, fId->value, desc->value,
                    width->value, height->value);
    Tree * result = xl_evaluate(context, body);
    player()->state = ready;
    return result;
}


Tree_p testAddKeyPress(Tree_p, Integer_p key,
                       Integer_p modifiers, Integer_p delay )
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    QKeyEvent ke(QEvent::KeyPress,
                 (Qt::Key)key->value,
                 (Qt::KeyboardModifiers)modifiers->value);

    player()->add( new TaoKeyEvent(ke, delay->value) );
    return XL::xl_true;
}


Tree_p testAddKeyRelease(Tree_p, Integer_p key,
                         Integer_p modifiers, Integer_p delay )
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    QKeyEvent ke(QEvent::KeyRelease,
                 (Qt::Key)key->value,
                 (Qt::KeyboardModifiers)modifiers->value);

    player()->add( new TaoKeyEvent(ke, delay->value) );
    return XL::xl_true;
}


Tree_p testAddMousePress(Tree_p, Integer_p button, Integer_p buttons,
                         Integer_p modifiers,
                         Integer_p x, Integer_p y, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    QMouseEvent me(QEvent::MouseButtonPress,
                   QPoint(x->value, y->value),
                   (Qt::MouseButton)button->value,
                   (Qt::MouseButtons)buttons->value,
                   (Qt::KeyboardModifiers)modifiers->value);

    player()->add( new TaoMouseEvent(me, delay->value) );
    return XL::xl_true;
}


Tree_p testAddMouseRelease(Tree_p, Integer_p button,
                           Integer_p buttons, Integer_p modifiers,
                           Integer_p x, Integer_p y, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    QMouseEvent me(QEvent::MouseButtonRelease,
                   QPoint(x->value, y->value),
                   (Qt::MouseButton)button->value,
                   (Qt::MouseButtons)buttons->value,
                   (Qt::KeyboardModifiers)modifiers->value);

    player()->add( new TaoMouseEvent(me, delay->value) );
    return XL::xl_true;
}


Tree_p testAddMouseDClick(Tree_p, Integer_p button,
                          Integer_p buttons, Integer_p modifiers,
                          Integer_p x, Integer_p y, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    QMouseEvent me(QEvent::MouseButtonDblClick,
                   QPoint(x->value, y->value),
                   (Qt::MouseButton)button->value,
                   (Qt::MouseButtons)buttons->value,
                   (Qt::KeyboardModifiers)modifiers->value);

    player()->add( new TaoMouseEvent(me, delay->value) );
    return XL::xl_true;
}


Tree_p testAddMouseMove(Tree_p, Integer_p buttons,
                        Integer_p modifiers,
                        Integer_p x, Integer_p y,
                        Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    QMouseEvent me(QEvent::MouseMove,
                   QPoint(x->value, y->value),
                   Qt::NoButton,
                   (Qt::MouseButtons)buttons->value,
                   (Qt::KeyboardModifiers)modifiers->value);

    player()->add( new TaoMouseEvent(me, delay->value) );

   return XL::xl_true;
}


Tree_p testAddAction(Tree_p , Text_p name, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a key press event to the current test
// ----------------------------------------------------------------------------
{
    player()->add(new TaoActionEvent(+name->value, delay->value));
    return XL::xl_true;
}


Tree_p testAddCheck(Tree_p , Integer_p num, Integer_p delay, double thr)
// ----------------------------------------------------------------------------
//  Add a check view event to the current test
// ----------------------------------------------------------------------------
{
    player()->add( new TaoCheckEvent(num->value, delay->value, thr));
    return XL::xl_true;
}


Tree_p testAddFont(Tree_p, Text_p name, Text_p ftname, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a font change event to the current test
// ----------------------------------------------------------------------------
{
    player()->add(new TaoFontActionEvent(+name->value, +ftname->value,
                                         delay->value));
    return XL::xl_true;
}


Tree_p testAddColor(Tree_p, Text_p name, Text_p colname,
                    Real_p alpha, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a color change event to the current test
// ----------------------------------------------------------------------------
{
    player()->add(new TaoColorActionEvent(+name->value, +colname->value,
                                          alpha->value, delay->value));
    return XL::xl_true;
}


Tree_p testAddFile(Tree_p, Text_p name, Text_p filename, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a font change event to the current test
// ----------------------------------------------------------------------------
{
    player()->add(new TaoFileActionEvent(+name->value, +filename->value,
                                         delay->value));
    return XL::xl_true;
}


Tree_p testAddCloseDialog(Tree_p, Text_p diagname,
                          Integer_p result, Integer_p delay)
// ----------------------------------------------------------------------------
//  Add a close dialog box event to the current test
// ----------------------------------------------------------------------------
{
    player()->add(new TaoDialogActionEvent(+diagname->value, result->value,
                                                delay->value));
    return XL::xl_true;
}


extern "C"
{

int module_init(const Tao::ModuleApi *api, const Tao::ModuleInfo *)
// ----------------------------------------------------------------------------
//   On module initialization, register display function
// ----------------------------------------------------------------------------
{
    XL_INIT_TRACES();
    synchroBasic::tao = api;
    return 0;
}

} // extern "C"

