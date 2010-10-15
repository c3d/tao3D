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
#include "widget.h"
#include "window.h"
#include "save_test_dialog.h"

TAO_BEGIN

WidgetTests::WidgetTests(Widget *widget, text name, text description) :
// ----------------------------------------------------------------------------
//   Creates a new test.
// ----------------------------------------------------------------------------
    widget(widget), name(+name), description(+description),
    featureId(0), latestResult(false)
{
    folder = ((Window*)(widget->window()))->currentProjectFolderPath().append("/");
}


text WidgetTests::toString()
// ----------------------------------------------------------------------------
//   Return the test as a tao command
// ----------------------------------------------------------------------------
{
    QString testDoc = QString("%1_test -> test_definition \"%1\", %2,"
                              " <<%3>>, do \n%4")
            .arg(name).arg(featureId)
            .arg(description).arg(taoCmd);
    return +testDoc;

}

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

    addAction(actName, time);

    QString cmd = QString("    test_add_action \"%1\", %2\n")
                  .arg(actName).arg(time);
    taoCmd.append(cmd);
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
            addKeyPress((Qt::Key)e->key(), e->modifiers(), delay);
            QString cmd = QString("    test_add_key_press %1, %2, %3\n")
                          .arg(e->key()).arg(e->modifiers()).arg(delay);
            taoCmd.append(cmd);
            break;
        }
    case QEvent::KeyRelease:
        {
            QKeyEvent *e = (QKeyEvent*)evt;
            int delay = startTime.restart();
            addKeyRelease((Qt::Key)e->key(), e->modifiers(), delay);
            QString cmd = QString("    test_add_key_release %1, %2, %3\n")
                          .arg(e->key()).arg(e->modifiers()).arg(delay);
            taoCmd.append(cmd);
            break;
        }
    case QEvent::MouseButtonPress:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            addMousePress(e->button(), e->modifiers(), e->pos(), delay);
            QString cmd = QString("    test_add_mouse_press %1, %2, %3, %4, %5\n")
                          .arg(e->button()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(cmd);
        }
    case QEvent::MouseMove:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            addMouseMove(e->buttons(), e->modifiers(), e->pos(), delay);
            QString cmd = QString("    test_add_mouse_move %1, %2, %3, %4, %5\n")
                          .arg(e->buttons()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(cmd);
            break;
        }
    case QEvent::MouseButtonRelease:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            addMouseRelease(e->button(), e->modifiers(), e->pos(), delay);
            QString cmd = QString("    test_add_mouse_release %1, %2, %3, %4, %5\n")
                          .arg(e->button()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(cmd);
            break;
        }
    case QEvent::MouseButtonDblClick:
        {
            QMouseEvent *e = (QMouseEvent *)evt;
            int delay = startTime.restart();
            addMouseDClick(e->button(), e->modifiers(), e->pos(), delay);
            QString cmd = QString("    test_add_mouse_dclick %1, %2, %3, %4, %5\n")
                          .arg(e->button()).arg(e->modifiers())
                          .arg(e->pos().x()).arg(e->pos().y()).arg(delay);
            taoCmd.append(cmd);
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
    QString playedBeforeName = QString(folder).append(name).append("_playedBefore.png");
    playedBefore.save(playedBeforeName, "PNG");

    if (playedBefore != before)
    {
        text t = +name;
        qWarning("Test %s: image before test is not equal to reference.",
                 t.c_str());
        if (playedBefore.rect() != before.rect())
            qWarning("\timage size are different.");
        if (playedBefore.format() != before.format())
            qWarning("\timage format differs.");
    }

    // Copy the list as the original one can be modified by a reload during simulate
    QTestEventList tempoList = QTestEventList(testList);
    tempoList.simulate(widget);
    playedAfter = widget->grabFrameBuffer(true);

    QString playedAfterName = QString(folder).append(name).append("_playedAfter.png");
    playedAfter.save(playedAfterName, "PNG");

    latestResult = (playedAfter == after);

    return latestResult;
}


void WidgetTests::save()
// ----------------------------------------------------------------------------
//   Save the named test
// ----------------------------------------------------------------------------
{
    Save_test_dialog dialog(widget, name, folder, featureId, description);
    if (dialog.exec() == QDialog::Rejected)
        return;

    name = dialog.name;
    description = dialog.desc;
    featureId = dialog.fid;
    folder = QFileInfo(dialog.loc).canonicalPath().append("/");

    // Store Images
    QString beforeName = QString(folder).append(name).append("_before.png");
    before.save(beforeName, "PNG");
    QString afterName = QString(folder).append(name).append("_after.png");
    after.save(afterName, "PNG");

    // Store test commands
    QString testName = QString(folder).append(name).append("_test.ddd");
    QFile testFile(testName);
    testFile.open(QIODevice::WriteOnly | QIODevice::Text);
    testFile.write(toString().c_str());
    testFile.flush();
    testFile.close();
}


void WidgetTests::reset(text newName, int feature, text desc, text dir)
// ----------------------------------------------------------------------------
//   Reset the test
// ----------------------------------------------------------------------------
{
    testList.clear();
    name = +newName;
    featureId = feature;
    description = +desc;
    folder = +dir;
    if (!folder.endsWith("/")) folder.append("/");

    if (name.isEmpty())
    {
        before = QImage();
        after = QImage();
    }
    else
    {
        before = QImage(QString("image:").append(name).append("_before.png"));
        after = QImage(QString("image:").append(name).append("_after.png"));
    }
}

void WidgetTests::printResult()
// ----------------------------------------------------------------------------
//   Print the result of this test on stderr
// ----------------------------------------------------------------------------
{
    std::cerr << +name << ", " << featureId << ", "
            << (latestResult ? "PASSED" : "FAILED")
            << ", " << +description << std::endl;
}


void WidgetTests::addKeyPress(Qt::Key qtKey,
                              Qt::KeyboardModifiers modifiers,
                              int msecs)
// ----------------------------------------------------------------------------
// Add a key press event to the list of action
// ----------------------------------------------------------------------------
{
    testList.addKeyPress(qtKey, modifiers, msecs);
}


void WidgetTests::addKeyRelease(Qt::Key qtKey,
                                Qt::KeyboardModifiers modifiers,
                                int msecs)
// ----------------------------------------------------------------------------
// Add a key release event to the list of action
// ----------------------------------------------------------------------------
{
    testList.addKeyRelease(qtKey, modifiers, msecs);
}


void WidgetTests::addMousePress(Qt::MouseButton button,
                                Qt::KeyboardModifiers modifiers,
                                QPoint pos, int delay )
// ----------------------------------------------------------------------------
// Add a mouse press event to the list of action
// ----------------------------------------------------------------------------
{
    testList.addMousePress(button, modifiers, pos, delay);
}


void WidgetTests::addMouseRelease(Qt::MouseButton button,
                                  Qt::KeyboardModifiers modifiers,
                                  QPoint pos, int delay )
// ----------------------------------------------------------------------------
// Add a mouse release event to the list of action
// ----------------------------------------------------------------------------
{
    testList.addMouseRelease(button, modifiers, pos, delay);

}


void WidgetTests::addMouseMove(Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers,
                               QPoint pos, int delay)
// ----------------------------------------------------------------------------
// Add a mouse move event to the list of action
// ----------------------------------------------------------------------------
{
    testList.append(new TestMouseMoveEvent(buttons, modifiers, pos, delay));
}


void WidgetTests::addMouseDClick(Qt::MouseButton button,
                                 Qt::KeyboardModifiers modifiers,
                                 QPoint pos, int delay)
// ----------------------------------------------------------------------------
// Add a mouse double click event to the list of action
// ----------------------------------------------------------------------------
{
    testList.addMouseDClick(button, modifiers, pos, delay);
}


void WidgetTests::addAction(QString name, int delay)
// ----------------------------------------------------------------------------
// Add an action to be replayed.
// ----------------------------------------------------------------------------
{
    testList.append(new TestActionEvent(name, delay));
}

TAO_END
