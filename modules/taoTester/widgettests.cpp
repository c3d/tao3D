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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include "widgettests.h"

#include <QFileInfo>
#include <QDir>
#include <QStatusBar>
#include <QTestEvent>
#include <QTextStream>
//#include "qtestevent.h"

#include "save_test_dialog.h"

#include "taotester.h"

WidgetTests::WidgetTests(QGLWidget *glw, text name, text description) :
// ----------------------------------------------------------------------------
//   Creates a new test.
// ----------------------------------------------------------------------------
    widget(glw), name(+name), description(+description),
    featureId(0), folder("./"), threshold(0.0), win(NULL), logfile(NULL),
    state(stopped), playingList(NULL)
{
    QStringList dirList = QDir::searchPaths("image");
    folder = dirList.at(1) + '/' + this->name;

    if ( ! glw )
        foreach (QWidget *w, QApplication::topLevelWidgets())
            if ((win = dynamic_cast<QMainWindow*>(w)) != NULL)
                if ((widget = dynamic_cast<QGLWidget *>(win->centralWidget()) ) != NULL)
                    break;
}


text WidgetTests::toString()
// ----------------------------------------------------------------------------
//   Return the test as a tao command
// ----------------------------------------------------------------------------
{
    QString testDoc = QString("%1_test -> test_definition \"%1\", %2, "
                              " <<%3>>, %6, %7, %5, do \n%4\n")
            .arg(name).arg(featureId)
            .arg(description).arg(taoCmd.isEmpty() ? "    nil" : taoCmd)
            .arg(threshold).arg(winSize.width()).arg(winSize.height());
    return +testDoc;

}

void WidgetTests::startRecord()
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    state = recording;
    // clear lists
    testList.clear();
    checkPointList.clear();
    win->statusBar()->showMessage("Start recording new test.");
    winSize = win->size();

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
    win->statusBar()->showMessage("End recording.");
    state = stopped;
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

    QKeySequence shortcut = act->shortcut();
    addAction(actName, time);

    QString cmd = QString("    test_add_action \"%1\", %2 // %3 \n")
                  .arg(actName).arg(time).arg(shortcut.toString());
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

void WidgetTests::recordFile(QString file)
// ----------------------------------------------------------------------------
//   Record the font change event.
// ----------------------------------------------------------------------------
{
    if (file.isEmpty()) return;

    TestFileActionEvent * evt =
            new TestFileActionEvent(QObject::sender()->objectName(),
                                    file, startTime.restart());
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
        TestDialogActionEvent* evt =
                new TestDialogActionEvent(sender->objectName(),
                                          result, startTime.restart());
        testList.append(evt);
        taoCmd.append(evt->toTaoCmd());
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
            QString cmd = QString("    test_add_key_press %1, %2, %3 // %4\n")
                          .arg(e->key()).arg(e->modifiers()).arg(delay).arg(e->text());
            taoCmd.append(cmd);
            break;
        }
    case QEvent::KeyRelease:
        {
            QKeyEvent *e = (QKeyEvent*)evt;
            int delay = startTime.restart();
            addKeyRelease((Qt::Key)e->key(), e->modifiers(), delay);
            QString cmd = QString("    test_add_key_release %1, %2, %3 // %4\n")
                          .arg(e->key()).arg(e->modifiers()).arg(delay).arg(e->text());
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
            else if (childName.contains("fileDialog") ||
                     childName.contains("QFileDialog"))
            {
                QFileDialog *diag = (QFileDialog*)e->child();
                connect(diag, SIGNAL(fileSelected(QString)),
                        this, SLOT(recordFile(QString)));
//                connect(diag, SIGNAL(finished(int)),
//                        this, SLOT(finishedDialog(int)));
            }
        }
    default:
        break;
    }

    return false;

}

bool WidgetTests::startPlay()
// ----------------------------------------------------------------------------
//   Replay the test
// ----------------------------------------------------------------------------
{
    state = playing;
    win->resize(winSize);
    win->statusBar()->showMessage("Playing Test " + name);
    nbChkPtKO = 0;
    logOpen();
    QCoreApplication::processEvents();
    QTest::qWait(1000);

    if (! before.isNull())
    {
        // snap shot the widget
        playedBefore = widget->grabFrameBuffer(true);

        // save the image
        QString playedBeforeName =
                QString(folder).append(name).append("_playedBefore.png");
        playedBefore.save(playedBeforeName, "PNG");

        // Compute the diff between reference and played image
        QString diffBeforeName = QString(folder).append(name).append("_diffBefore.png");
        double diffVal = diff(before, playedBefore, diffBeforeName);

        if (diffVal > threshold)
            nbChkPtKO++;

        log(BEFORE, diffVal <= threshold, diffVal);
    }

    // Copy the list as the original one can be modified by a reload during simulate
    QTestEventList tempoList = QTestEventList(testList);
    playingList = &tempoList;
    tempoList.simulate(widget);
    playingList = NULL;

    QCoreApplication::processEvents();

    if ( ! after.isNull() )
    {
        if (tempoList.isEmpty())
            QTest::qWait(800);
        // snap shot the widget
        playedAfter = widget->grabFrameBuffer(true);

        // save the image
        QString playedAfterName =
                QString(folder).append(name).append("_playedAfter.png");
        playedAfter.save(playedAfterName, "PNG");

        // Compute the diff between reference and played image
        QString diffAfterName = QString(folder).append(name).append("_diffAfter.png");
        double diffVal = diff(after, playedAfter, diffAfterName);

        if (diffVal > threshold)
            nbChkPtKO++;

        log(AFTER, diffVal <= threshold, diffVal);
    }

    win->statusBar()->showMessage("Test " + name +
                                  (nbChkPtKO > 0 ? " failed" : " is successful"));
    logClose(nbChkPtKO <= 0);
    QTest::qWait(1000);

    return nbChkPtKO > 0;
}

void WidgetTests::stopPlay()
{
    std::cerr << "\n\n STOP demande \n\n";
    if (playingList)
    {
        playingList->clear();
        state = stopped;
    }
}

void WidgetTests::stop()
{
    switch (state) {
        case playing : stopPlay(); return;
        case recording : stopRecord(); return;
        default : return;
    }
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
    // If no action an empty list is written
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


void WidgetTests::reset(text newName, int feature, text desc, text dir, double thr,
                        int width, int height)
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
    winSize.setWidth(width);
    winSize.setHeight(height);
    win->statusBar()->showMessage("Loading test " + name);

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
        before = QImage(QString("image:").append(name).append('/').append(name).append("_before.png"));
        after = QImage(QString("image:").append(name).append('/').append(name).append("_after.png"));
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


void WidgetTests::addFile(QString diagName, QString fileName, int delay)
// ----------------------------------------------------------------------------
// Add an action to be replayed.
// ----------------------------------------------------------------------------
{
    testList.append(new TestFileActionEvent(diagName, fileName, delay));
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
       QString msg = QString("Image size differs: reference is (%1 x %2) and played is (%3 x %4).").
               arg(ref.size().width()).
               arg(ref.size().height()).
               arg(played.size().width()).
               arg(played.size().height());
        log(msg);
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

    double res = (nbDiff * 100.0)/(ref.width()* ref.height());

    if (!filename.isEmpty() && (res > threshold || forceSave))
        imgDiff.save(filename, "PNG");

    return res;
}

#define INDENT1 "  "
#define INDENT2 "    "

// ============================================================================
//
//   Log facilities
//
// ============================================================================
void WidgetTests::logOpen()
{
    logfile  = new QFile(folder+"../results.ddd");
    logfile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    QTextStream resFile(logfile);
    resFile << INDENT1 << "result \"" << name << "\", <<" << description << ">>, "
            << featureId << ", " << threshold << ", {";
}
void WidgetTests::logClose(bool result)
{
    QTextStream resFile(logfile);
    resFile << "}, " << (result ? "true" : "false") << "\n\n";
    resFile.flush();
    logfile->close();
}
void WidgetTests::log(int No, bool isOK, double Tx)
{
    QTextStream resFile(logfile);
    resFile << "\n" << INDENT2 << "check " << No << ", "
            << (isOK ? "true": "false") << ", " << Tx;
}
void WidgetTests::log(QString msg)
{
    QTextStream resFile(logfile);
    resFile << "\n" << INDENT2 << "paragraph_break\n"<<
            INDENT2 << "text <<Next check reports: "<< msg << ">>";
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
    QGLWidget * widget = (QGLWidget *)w;
    QString testName = taoTester::tester()->currentTest()->name;
    QFileInfo refFile(QString("image:%1/%1_%2.png").arg(testName).arg(number));
    QImage ref(refFile.canonicalFilePath());
    QImage shot = widget->grabFrameBuffer(true);

    shot.save(QString("%1/%2_played_%3.png")
              .arg(refFile.canonicalPath()).arg(testName).arg(number), "PNG");

    QString diffFilename = QString("%1/%2_diff_%3.png").arg(refFile.canonicalPath())
                           .arg(testName).arg(number);
    double resDiff = taoTester::tester()->currentTest()->diff(ref, shot, diffFilename);

    std::cerr << +testName <<  "\t Intermediate check " << number ;
    bool inError = resDiff > taoTester::tester()->currentTest()->threshold;
    if (inError)
    {
        std::cerr<< " fails. Diff is " << resDiff << "%\n";
        taoTester::tester()->currentTest()->nbChkPtKO++;
    }
    else
        std::cerr<< " succeeds. Diff is " << resDiff << "%\n";
    taoTester::tester()->currentTest()->log(number, !inError, resDiff);
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


void TestFileActionEvent::simulate(QWidget *w)
// ----------------------------------------------------------------------------
//  Set the current font of the QFontDialog box.
// ----------------------------------------------------------------------------
{
    QFileDialog* diag = w->findChild<QFileDialog*>(objName);
    if (diag )
    {
        QTest::qWait(delay);
        diag->setVisible(false);
        diag->selectFile(fileName);
        QDialog *d = (QDialog *) diag;
        d->accept();
    }
}


QString TestFileActionEvent::toTaoCmd()
// ----------------------------------------------------------------------------
//  Return a command line for Tao.
// ----------------------------------------------------------------------------
{
    QString cmd = QString("    test_add_file \"%1\", \"%2\", %3\n")
                  .arg(objName).arg(fileName).arg(delay);
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

QString TestDialogActionEvent::toTaoCmd()
// ----------------------------------------------------------------------------
//  Return a command line for Tao.
// ----------------------------------------------------------------------------
{
    QString cmd = QString("    test_dialog_action \"%1\", \"%2\", %3\n")
                  .arg(objName).arg(result).arg(delay);
    return cmd;
}
