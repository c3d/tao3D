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
    featureId(0), threshold(0.0)
{
    folder = ((Window*)(widget->window()))->currentProjectFolderPath().append("/");
}


text WidgetTests::toString()
// ----------------------------------------------------------------------------
//   Return the test as a tao command
// ----------------------------------------------------------------------------
{
    QString testDoc = QString("%1_test -> test_definition \"%1\", %2, "
                              " <<%3>>, %5, do \n%4")
            .arg(name).arg(featureId)
            .arg(description).arg(taoCmd.isEmpty() ? "    nil" : taoCmd)
            .arg(threshold);
    return +testDoc;

}

void WidgetTests::startRecord()
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    // clear lists
    testList.clear();
    checkPointList.clear();

    //photo
    before = widget->grabFrameBuffer(true);
    //connection
    foreach (QAction* act,  widget->parent()->findChildren<QAction*>())
    {
        if (act->objectName().startsWith("toolbar:test"))
            continue;
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
//   Records actions.
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


void WidgetTests::recordColor(QColor color)
// ----------------------------------------------------------------------------
//   Record the color change event.
// ----------------------------------------------------------------------------
{
    TestColorActionEvent * evt =
            new TestColorActionEvent(QObject::sender()->objectName(),
                                     color.name(), startTime.restart());
    testList.append(evt);
    taoCmd.append(evt->toTaoCmd());
}


void WidgetTests::recordFont(QFont font)
// ----------------------------------------------------------------------------
//   Record the font change event.
// ----------------------------------------------------------------------------
{
    TestFontActionEvent * evt =
            new TestFontActionEvent(QObject::sender()->objectName(),
                                    font.toString(), startTime.restart());
    testList.append(evt);
    taoCmd.append(evt->toTaoCmd());
}


void WidgetTests::finishedDialog(int result)
// ----------------------------------------------------------------------------
//   Record the result of the dialog box (and close event).
// ----------------------------------------------------------------------------
{
    QObject *sender = QObject::sender();
    if (sender)
    {
        QDialog * dialog = dynamic_cast<QDialog*>(sender);
        if (dialog)
        {
            disconnect(dialog, 0, this, 0);
        }
        testList.append(new TestDialogActionEvent(sender->objectName(),
                                                  result, startTime.restart()));
    }
}


void WidgetTests::checkNow()
// ----------------------------------------------------------------------------
//   Records a check point and the view.
// ----------------------------------------------------------------------------
{
    TestCheckEvent *check = new TestCheckEvent(checkPointList.size(),
                                               startTime.restart());
    testList.append(check);
    taoCmd.append(check->toTaoCmd());
    QImage shot = widget->grabFrameBuffer(true);
    checkPointList.append(shot);
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
    case QEvent::ChildPolished:
        {
            QChildEvent *e = (QChildEvent*)evt;
            QString childName = e->child()->objectName();
            std::cerr<< "Object polished " << +childName << std::endl; // CaB
            if ( childName.contains("colorDialog"))
            {
                QColorDialog *diag = (QColorDialog*)e->child();
                connect(diag, SIGNAL(currentColorChanged(QColor)),
                        this, SLOT(recordColor(QColor)));
                connect(diag, SIGNAL(finished(int)),
                        this, SLOT(finishedDialog(int)));
            }
            else if (childName.contains("fontDialog"))
            {
                QFontDialog *diag = (QFontDialog*)e->child();
                connect(diag, SIGNAL(currentFontChanged(QFont)),
                        this, SLOT(recordFont(QFont)));
                connect(diag, SIGNAL(finished(int)),
                        this, SLOT(finishedDialog(int)));
            }
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
    nbChkPtKO = 0;

    if (! before.isNull())
    {
        // snap shot the widget
        playedBefore = widget->grabFrameBuffer(true);

        // save the image
        QString playedBeforeName = QString(folder).append(name).append("_playedBefore.png");
        playedBefore.save(playedBeforeName, "PNG");

        // Compute the diff between reference and played image
        QString diffBeforeName = QString(folder).append(name).append("_diffBefore.png");
        double diffVal = diff(before, playedBefore, diffBeforeName);

        if (diffVal > threshold)
        {
            text t = +name;
            qWarning("Test %s: image before test is not equal to reference in %f%%",
                     t.c_str(), diffVal);
        }
    }

    // Copy the list as the original one can be modified by a reload during simulate
    QTestEventList tempoList = QTestEventList(testList);
    tempoList.simulate(widget);

    if ( ! after.isNull() )
    {
        // snap shot the widget
        playedAfter = widget->grabFrameBuffer(true);

        // save the image
        QString playedAfterName = QString(folder).append(name).append("_playedAfter.png");
        playedAfter.save(playedAfterName, "PNG");

        // Compute the diff between reference and played image
        QString diffAfterName = QString(folder).append(name).append("_diffAfter.png");
        double diffVal = diff(after, playedAfter, diffAfterName);

        if (diffVal > threshold)
        {
            nbChkPtKO++;
            text t = +name;
            qWarning("Test %s: image after test is not equal to reference in %f%%",
                     t.c_str(), diffVal);
        }
    }

    return nbChkPtKO > 0;
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
    folder = QFileInfo(dialog.loc).canonicalFilePath();
    if (!folder.endsWith("/")) folder.append("/");

    // Store Images
    if (!before.isNull())
    {
        QString beforeName = QString(folder).append(name).append("_before.png");
        before.save(beforeName, "PNG");
    }
    if (!after.isNull())
    {
        QString afterName = QString(folder).append(name).append("_after.png");
        after.save(afterName, "PNG");
    }
    // Store test commands
    // If no action a nil list is written
    QString testName = QString(folder).append(name).append("_test.ddd");
    QFile testFile(testName);
    testFile.open(QIODevice::WriteOnly | QIODevice::Text);
    testFile.write(toString().c_str());
    testFile.flush();
    testFile.close();

    // Store check point images
    for (int i = 0; i < checkPointList.size(); i++)
    {
        QImage shot = checkPointList[i];
        QString chkPt=QString("%1%2_%3.png").arg(folder).arg(name).arg(i);
        shot.save(chkPt, "PNG");
    }
}


void WidgetTests::reset(text newName, int feature, text desc, text dir, double thr)
// ----------------------------------------------------------------------------
//   Reset the test
// ----------------------------------------------------------------------------
{
    testList.clear();
    taoCmd.clear();
    checkPointList.clear();
    nbChkPtKO = 0;
    name = +newName;
    featureId = feature;
    threshold = thr;

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
            << (nbChkPtKO == 0 ? "PASSED" : "FAILED")
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


void WidgetTests::addAction(QString actName, int delay)
// ----------------------------------------------------------------------------
// Add an action to be replayed.
// ----------------------------------------------------------------------------
{
    testList.append(new TestActionEvent(actName, delay));
}


void WidgetTests::addCheck( int num, int delay)
// ----------------------------------------------------------------------------
// Add a mouse move event to the list of action
// ----------------------------------------------------------------------------
{
    testList.append(new TestCheckEvent(num, delay));
}


void WidgetTests::addColor(QString diagName, QString colName, int delay)
// ----------------------------------------------------------------------------
// Add an action to be replayed.
// ----------------------------------------------------------------------------
{
    testList.append(new TestColorActionEvent(diagName, colName, delay));
}


void WidgetTests::addFont(QString diagName, QString ftName, int delay)
// ----------------------------------------------------------------------------
// Add an action to be replayed.
// ----------------------------------------------------------------------------
{
    testList.append(new TestFontActionEvent(diagName, ftName, delay));
}


void WidgetTests::addDialogClose(QString objName, int result,  int delay)
// ----------------------------------------------------------------------------
// Add an action to be replayed.
// ----------------------------------------------------------------------------
{
    testList.append(new TestDialogActionEvent(objName, result, delay));
}

double WidgetTests::diff(QImage &ref, QImage &played, QString filename,
                         bool forceSave)
// ----------------------------------------------------------------------------
// Generate an mage with just the different pixel in red.
// ----------------------------------------------------------------------------
{
    if (ref.size() != played.size())
    {
        std::cerr << "Image size differs: ref is (" << ref.size().width() << " x " <<
                ref.size().height() << ") and played is (" << played.size().width()
                << " x "  << played.size().height() << ")\n";
        return 100.0;
    }

    int nbDiff = 0;
    QImage imgDiff(ref.size(), ref.format());

    for (int line =0; line < ref.height(); line++)
    {
        for (int col=0; col < ref.width(); col++)
        {
            if (ref.pixel(col, line) != played.pixel(col,line))
            {
                imgDiff.setPixel(col, line, QColor(Qt::red).rgba());
                nbDiff++;
            }
            else
                imgDiff.setPixel(col, line, QColor(Qt::transparent).rgba());
        }
    }

    double res = (nbDiff * 100.0)/ref.byteCount();

    if (!filename.isEmpty() && (res > threshold || forceSave))
        imgDiff.save(filename, "PNG");

    return res;
}


// ============================================================================
//
//   Event Tests Classes
//
// ============================================================================


void TestCheckEvent::simulate(QWidget *w)
// ----------------------------------------------------------------------------
//  Perform a check againts the reference view.
// ----------------------------------------------------------------------------
{
    Widget *widget = (Widget*) w;
    QString testName = widget->currentTest.name;
    QFileInfo refFile(QString("image:%1_%2.png").arg(testName).arg(number));
    QImage ref(refFile.canonicalFilePath());
    QImage shot = widget->grabFrameBuffer(true);

    shot.save(QString("%1/%2_played_%3.png")
              .arg(refFile.canonicalPath()).arg(testName).arg(number), "PNG");

    QString diffFilename = QString("%1/%2_diff_%3.png").arg(refFile.canonicalPath())
                           .arg(testName).arg(number);
    double resDiff = widget->currentTest.diff(ref, shot, diffFilename);

    std::cerr << +testName <<  "\t Intermediate check " << number ;
    if (resDiff > widget->currentTest.threshold)
    {
        std::cerr<< " fails by " << resDiff << "%\n";
        widget->currentTest.nbChkPtKO++;
    }
    else
        std::cerr<< " succeeds by " << resDiff << "%\n";

}


QString TestCheckEvent::toTaoCmd()
// ----------------------------------------------------------------------------
//  Return a command line for Tao.
// ----------------------------------------------------------------------------
{
    QString cmd = QString("    test_add_check %1, %2\n").arg(number).arg(delay);
    return cmd;
}


void TestColorActionEvent::simulate(QWidget *w)
// ----------------------------------------------------------------------------
//   Set the current color of the QColorDialog box.
// ----------------------------------------------------------------------------
{
    QColorDialog* diag = w->findChild<QColorDialog*>(objName);
    QColor col(colorName);
    if (diag &&col.isValid())
    {
        QTest::qWait(delay);
        diag->setCurrentColor(col);
    }
}


QString TestColorActionEvent::toTaoCmd()
// ----------------------------------------------------------------------------
//  Return a command line for Tao.
// ----------------------------------------------------------------------------
{
    QString cmd = QString("    test_add_color \"%1\", \"%2\", %3\n")
                  .arg(objName).arg(colorName).arg(delay);
    return cmd;
}


void TestFontActionEvent::simulate(QWidget *w)
// ----------------------------------------------------------------------------
//  Set the current font of the QFontDialog box.
// ----------------------------------------------------------------------------
{
    QFontDialog* diag = w->findChild<QFontDialog*>(objName);
    QFont ft;
    ft.fromString(fontName);
    if (diag )
    {
        QTest::qWait(delay);
        diag->setCurrentFont(ft);
    }
}


QString TestFontActionEvent::toTaoCmd()
// ----------------------------------------------------------------------------
//  Return a command line for Tao.
// ----------------------------------------------------------------------------
{
    QString cmd = QString("    test_add_font \"%1\", \"%2\", %3\n")
                  .arg(objName).arg(fontName).arg(delay);
    return cmd;
}


void TestDialogActionEvent::simulate(QWidget *w)
// ----------------------------------------------------------------------------
//  Set the result of the dialog (Accepted or Rejected) and close it.
// ----------------------------------------------------------------------------
{
    QDialog* diag = w->findChild<QDialog*>(objName);

    if (diag)
    {
        QTest::qWait(delay);
        diag->done(result);
    }

}
TAO_END
