// ****************************************************************************
//  widgettests.cpp						    Tao project
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
#include "qtestevent.h"
#include "tao_utf8.h"
#include "application.h"
#include "window.h"

TAO_BEGIN


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
    if (actName.isEmpty()) return;

    int time = startTime.restart();

    testList.append(new QTestActionEvent(actName, time));

    QString cmd = QString("    test_add_action \"%1\", %2\n")
                  .arg(actName).arg(time);
    taoCmd.append(+cmd);
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
            int delay = startTime.restart();
            testList.addKeyPress(e->key(),
                                 e->modifiers(),
                                 delay);
            QString cmd = QString("    test_add_key_press %1, %2, %3\n")
                          .arg(e->key()).arg(e->modifiers()).arg(delay);
            taoCmd.append(+cmd);
            break;
        }
    case QEvent::KeyRelease:
        {
            QKeyEvent *e = (QKeyEvent*)evt;
            int delay = startTime.restart();
            testList.addKeyRelease(e->key(),
                                   e->modifiers(),
                                   delay);
            QString cmd = QString("    test_add_key_release %1, %2, %3\n")
                          .arg(e->key()).arg(e->modifiers()).arg(delay);
            taoCmd.append(+cmd);
            break;
        }
    case QEvent::MouseButtonPress:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            testList.addMousePress(e->button(),
                                     e->modifiers(),
                                     e->pos(),
                                     delay);
            QString cmd = QString("    test_add_mouse_press %1, %2, %3, %4, %5\n")
                          .arg(e->button()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(+cmd);
        }
    case QEvent::MouseMove:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            testList.addMouseMove(e->pos(),
                                  delay);
            QString cmd = QString("    test_add_mouse_move %1, %2, %3\n")
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(+cmd);
            break;
        }
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            testList.addMouseRelease(e->button(),
                                     e->modifiers(),
                                     e->pos(),
                                     delay);
            QString cmd = QString("    test_add_mouse_release %1, %2, %3, %4, %5\n")
                          .arg(e->button()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(+cmd);
            break;
        }
    case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            testList.addMouseDClick(e->button(),
                                    e->modifiers(),
                                    e->pos(),
                                    delay);
            QString cmd = QString("    test_add_mouse_dclick %1, %2, %3, %4, %5\n")
                          .arg(e->button()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(+cmd);
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
                 name.c_str());
    testList.simulate(widget);
    playedAfter = widget->grabFrameBuffer(true);

    return latestResult = (playedAfter == after);
}


void WidgetTests::load(text newName)
// ----------------------------------------------------------------------------
//   Load the named test
// ----------------------------------------------------------------------------
{
    if (!newName.empty())
        name = newName;

    qWarning("Not Yet implemented: Should load %s\n", name.c_str());
}

void WidgetTests::save(text newName)
// ----------------------------------------------------------------------------
//   Save the named test
// ----------------------------------------------------------------------------
{
    if (!newName.empty())
        name = newName;

    // Get project directory
    Window *w = (Window*) widget->parent();
    QString folder = w->currentProjectFolderPath();
    folder.append("/");

    // Store Images
    before.save(QString(folder).append(+name).append("_before.png"), "PNG");
    after.save(QString(folder).append(+name).append("_after.png"), "PNG");
    // Store test commands
    QFile testFile(QString(folder).append(+name).append("_test.ddd"));
    testFile.open(QIODevice::WriteOnly | QIODevice::Text);
    testFile.write(toString()->c_str());
    testFile.flush();
    testFile.close();

    qWarning("Not Yet implemented: Should save %s\n", name.c_str());

}


void WidgetTests::reset(text newName, text desc)
// ----------------------------------------------------------------------------
//   Reset the test
// ----------------------------------------------------------------------------
{
    testList.clear();
    name = newName;
    description = desc;
    before = QImage(QString("image:").append(+name).append("_before.png"));
    after = QImage(QString("image:").append(+name).append("_after.png"));
}

void WidgetTests::printResult()
{
    std::cerr << "Test " << name << "\t\t"
            << (latestResult ? "PASSED" : "FAILED")
            << "\t" << description << std::endl;
}

TAO_END
