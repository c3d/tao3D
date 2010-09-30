// ****************************************************************************
//  widgettests.cpp							   Tao project
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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include "widgettests.h"

#include "widget.h"
//#include "qtestevent.h"
#include "tao_utf8.h"
#include "application.h"
namespace Tao {


class QTestActionEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store QAction events with other mouse/key events
// ----------------------------------------------------------------------------
{
public:
    inline QTestActionEvent(QString name, int delay)
        : _name(name), _delay(delay) {}
    inline QTestEvent *clone() const { return new QTestActionEvent(*this); }

    inline void simulate(QWidget *w)
    {
         QAction* act = w->parent()->findChild<QAction*>(_name);
        if (act)
        {
            QTest::qWait(_delay);
            act->activate(QAction::Trigger);
        }
    }

private:
    QString _name;
    int _delay;
};


void WidgetTests::startRecord()
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    testList.clear();
    //photo
    before = widget->grabFrameBuffer(true);
    //connection
    foreach (QAction* act,  widget->parent()->findChildren<QAction*>())
    {
         connect(act, SIGNAL(triggered(bool)), this, SLOT(recordAction(bool)));
    }
    widget->installEventFilter(this);
    startTime.start();
}


void WidgetTests::stopRecord()
// ----------------------------------------------------------------------------
//   Stop recording events
// ----------------------------------------------------------------------------
{
    //photo
    after = widget->grabFrameBuffer(true);
    //connection
    foreach (QAction* act,  widget->parent()->findChildren<QAction*>())
    {
        disconnect(act, SIGNAL(triggered(bool)),
                   this, SLOT(recordAction(bool)));
    }
    widget->removeEventFilter(this);
}


void WidgetTests::recordAction(bool )
// ----------------------------------------------------------------------------
//   Records actions
// ----------------------------------------------------------------------------
{
    QAction* act = dynamic_cast<QAction*>(QObject::sender());
    if ( ! act) return;

    QString actName = act->objectName();
    int time = startTime.restart();

    testList.append(new QTestActionEvent(actName, time));
}


bool WidgetTests::eventFilter(QObject */*obj*/, QEvent *evt)
// ----------------------------------------------------------------------------
//   Records all the events on widget
// ----------------------------------------------------------------------------
{
    switch(evt->type())
    {
    case QEvent::KeyPress:
        {
            QKeyEvent *e = (QKeyEvent*)evt;
            testList.addKeyPress(e->key(),
                                 e->modifiers(),
                                 startTime.restart());
            break;
        }
    case QEvent::KeyRelease:
        {
            QKeyEvent *e = (QKeyEvent*)evt;
            testList.addKeyRelease(e->key(),
                                   e->modifiers(),
                                   startTime.restart());
            break;
        }
    case QEvent::MouseButtonPress:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            testList.addMousePress(e->button(),
                                     e->modifiers(),
                                     e->pos(),
                                     startTime.restart());
            break;
        }
    case QEvent::MouseMove:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            testList.addMouseMove(e->pos(),
                                  startTime.restart());
            break;
        }
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            testList.addMouseRelease(e->button(),
                                     e->modifiers(),
                                     e->pos(),
                                     startTime.restart());
            break;
        }
    case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            testList.addMouseDClick(e->button(),
                                    e->modifiers(),
                                    e->pos(),
                                    startTime.restart());
            break;
        }
    default:
        break;
    }

    return false;

}

bool WidgetTests::play()
// ----------------------------------------------------------------------------
//   Replay the test
// ----------------------------------------------------------------------------
{
    playedBefore = widget->grabFrameBuffer(true);
    if (playedBefore != before)
        qWarning("Test %s: image before test is not equal to reference.\n",
                 (+name).c_str());
    testList.simulate(widget);
    playedAfter = widget->grabFrameBuffer(true);

    return playedAfter == after;
}


void WidgetTests::load(QString newName)
// ----------------------------------------------------------------------------
//   Load the named test
// ----------------------------------------------------------------------------
{
    if (!newName.isEmpty())
        name = newName;

    qWarning("Not Yet implemented: Should load %s\n", (+name).c_str());
}

void WidgetTests::save(QString newName)
// ----------------------------------------------------------------------------
//   Save the named test
// ----------------------------------------------------------------------------
{
    if (!newName.isEmpty())
        name = newName;

    qWarning("Not Yet implemented: Should save %s\n", (+name).c_str());

}
}
