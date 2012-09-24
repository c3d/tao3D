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

#include "tao/module_api.h"

#include "widgettests.h"
#include "tao_test_events.h"
#include "../tao_synchro/event_capture.h"
#include "save_test_dialog.h"
#include "test_display.h"

#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QStatusBar>
#include <QCoreApplication>
#include <QProcess>

#define INDENT1 "  "
#define INDENT2 "    "


WidgetTests::WidgetTests() :
// ----------------------------------------------------------------------------
//   Creates a new test.
// ----------------------------------------------------------------------------
    name(""), description(""), featureId(0), logfile(NULL),
    state(stopped), shotImage(NULL)
{
    IFTRACE(displaymode)
        test_display::debug() << "Initializing\n";

    synchroBasic::tao->registerDisplayFunction("taotester",
                                 test_display::display,
                                 test_display::use,
                                 test_display::unuse,
                                 NULL,
                                 NULL);
    state = loading;
    IFTRACE(displaymode)
        test_display::debug() << "Done\n";

}


void WidgetTests::reset(text newName, int feature, text desc, int width, int height)
// ----------------------------------------------------------------------------
//   Reset the test
// ----------------------------------------------------------------------------
{
    state = loading;

    testList.clear();
    nbCheckPoint = 0;
    name = +newName;
    featureId = feature;
    synchroBasic::base->winSize.setWidth(width);
    synchroBasic::base->winSize.setHeight(height);
    synchroBasic::base->win->statusBar()->showMessage("Loading test " + name);

    description = +desc;

    IFTRACE (tester)
            std::cerr << "WidgetTests::reset name="<< +name
                      << ", desc="<< +description <<std::endl;
}


QString WidgetTests::getDir(bool refDir)
// ----------------------------------------------------------------------------
//   Return the directory to load or save data
// ----------------------------------------------------------------------------
{
    QString dir = +synchroBasic::tao->currentDocumentFolder();
    QStringList environment = QProcess::systemEnvironment();
    QStringList var = environment.filter(refDir?"TAO_REF_DIR=":"TAO_OUT_DIR=");
    if ( ! var.size() )
    {
        if ( refDir)
            return dir;

        QString runDir = QString("run-%1")
                .arg(QDateTime::currentDateTime()
                     .toString("yyyy-MM-dd_hh.mm.ss"));
        QDir(dir).mkdir(runDir);
        dir.append("/").append(runDir);

        return dir;
    }

    QString loc = var.first().remove(refDir ? "TAO_REF_DIR=" : "TAO_OUT_DIR=");
    if (!loc.startsWith("/"))
        dir = QDir::cleanPath(dir.append("/").append(loc));
    else
        dir = QDir::cleanPath(loc);

    QDir::current().mkpath(dir);

    return dir;
}

// ============================================================================
//
//              TEST RECORDER
//
// ============================================================================


bool TestRecorder::beforeStart()
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    // clear lists
    testList.clear();
    nbCheckPoint = 0;

    return true;
}


bool TestRecorder::afterStart()
// ----------------------------------------------------------------------------
//   Start recording a sequence of events
// ----------------------------------------------------------------------------
{
    state = recording;
    return true;
}


bool TestRecorder::beforeStop()
// ----------------------------------------------------------------------------
//   Stop recording events
// ----------------------------------------------------------------------------
{
    return true;
}


bool TestRecorder::afterStop()
// ----------------------------------------------------------------------------
//   Stop recording events
// ----------------------------------------------------------------------------
{
    state = stopped;
    return true;
}


 void TestRecorder::add(TaoControlEvent *evt)
 // ----------------------------------------------------------------------------
 //   Add an event to the list
 // ----------------------------------------------------------------------------
 // Got the ownership of the evt memory. To be deleted at save or clear time.
 {
     testList.append(evt);
 }


void TestRecorder::checkNow()
// ----------------------------------------------------------------------------
//   Records a check point and the view.
// ----------------------------------------------------------------------------
{
//    QImage shot = synchroBasic::base->widget->grabFrameBuffer(false);

    TaoCheckEvent *check =
            new TaoCheckEvent(nbCheckPoint++,
                              ((EventCapture*)synchroBasic::base)->startTime.
                              restart());
    shotImage = &(check->playedImage);

    // Refresh the screen
    synchroBasic::tao->refreshOn(QEvent::Timer, -1.0);
    synchroBasic::tao->refreshOn(QEvent::MouseMove, -1.0);

    add(check);
}


void TestRecorder::save()
// ----------------------------------------------------------------------------
//   Save the named test
// ----------------------------------------------------------------------------
{
    QString folder = getDir(true);
    Save_test_dialog dialog(synchroBasic::base->widget, name, folder,
                            featureId, description);
    if (dialog.exec() == QDialog::Rejected || dialog.name.isEmpty())
        return;

    name = dialog.name;
    description = dialog.desc;
    featureId = dialog.fid;
    folder = QFileInfo(dialog.loc).canonicalFilePath();

    // Store test commands
    // If no action an empty list is written
    QString testName = QString("%1/%2_test.xl").arg(folder).arg(name);
    QFile testFile(testName);
    testFile.open(QIODevice::WriteOnly | QIODevice::Text);
    testFile.write(toTaoCmd().c_str());
    testFile.flush();
    testFile.close();
}


text TestRecorder::toTaoCmd()
// ----------------------------------------------------------------------------
//   Return the test as a tao command
// ----------------------------------------------------------------------------
{
    QString folder = getDir(true);
    QString testDoc = QString("%1_test -> test_definition \"%1\", %2, "
                              " <<%3>>, %4, %5, \n")
            .arg(name).arg(featureId)
            .arg(description)
            .arg(synchroBasic::base->winSize.width())
            .arg(synchroBasic::base->winSize.height());
    if (testList.isEmpty())
        testDoc.append(INDENT1).append("nil\n");
    else
    {
        TaoMouseEvent *lastMouseEvent = NULL;
        foreach (TaoControlEvent *evt, testList)
        {
            // Merge mouseMoveEvents to only one event with latest (x,y)
            // and sum of delay values
            if (evt->getType() == QEvent::MouseMove )
            {
                TaoMouseEvent * mouse = static_cast<TaoMouseEvent*>(evt);
                TaoMouseEvent *newMouseEvent = mouse->merge(lastMouseEvent);
                if (lastMouseEvent)
                    delete lastMouseEvent;
                lastMouseEvent = newMouseEvent;
                continue;
            }
            if (lastMouseEvent)
            {
                testDoc.append(INDENT1).append(lastMouseEvent->toTaoCmd())
                        .append("\n");
                lastMouseEvent = NULL;
            }

            testDoc.append(INDENT1).append(evt->toTaoCmd()).append("\n");
            if (evt->getType() == CHECK_EVENT_TYPE)
            {
                TaoCheckEvent *cp = static_cast<TaoCheckEvent*>(evt);
                if (! cp->playedImage || cp->playedImage->isNull())
                {
                    delete evt;
                    continue;
                }
                QString cpName= QString("%1/%2_%3.png")
                        .arg(folder).arg(name).arg(cp->number);
                cp->playedImage->save(cpName);
            }
            delete evt;
        }
        testList.clear();
    }

    testDoc.append("\nstart_test -> \n")
            .append(INDENT1).append(name).append("_test\n")
            .append(INDENT1).append("display_mode \"taotester\"\n")
            .append(INDENT1).append("test_replay\n");

    testDoc.append("\nstart_ref -> \n")
            .append(INDENT1).append(name).append("_test\n")
            .append(INDENT1).append("display_mode \"taotester\"\n")
            .append(INDENT1).append("make_ref\n");

    return +testDoc;
}


// ============================================================================
//
//            TEST PLAYER
//
// ============================================================================


TestPlayer::TestPlayer():
// ----------------------------------------------------------------------------
//   Creates a new test to be played
// ----------------------------------------------------------------------------
    makeRef(false)
{
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(play_and_next()));
}


void TestPlayer::reset(text newName, int feature, text desc, int width, int height)
// ----------------------------------------------------------------------------
//   Set test information
// ----------------------------------------------------------------------------
{
    WidgetTests::reset(newName, feature, desc, width, height);
    state = loading;
}


void TestPlayer::add(TaoControlEvent *evt)
// ----------------------------------------------------------------------------
//   Add an event to the list
// ----------------------------------------------------------------------------
// Got the ownership of the evt memory. To be deleted at save or clear time.
{
    testList.append(evt);
}


bool TestPlayer::beforeStart()
// ----------------------------------------------------------------------------
//   Get ready to start the test
// ----------------------------------------------------------------------------
{
    // Feed the save_test_dialog

    state = playing;
    return true;
}


bool TestPlayer::afterStart()
// ----------------------------------------------------------------------------
//   Start the test
// ----------------------------------------------------------------------------
{
    if (testList.isEmpty())
        return false;
    cpList.clear();
    playingList.clear();
    playingList.append(testList);
    testList.clear();
    if (playingList.size())
        timer.start(playingList.first()->getDelay());
    return true;
}


void TestPlayer::play_and_next()
// ----------------------------------------------------------------------------
//   Play the current event and restart the timer for the next one
// ----------------------------------------------------------------------------
{
    TaoControlEvent * evt = playingList.first();
    IFTRACE(tester)
            std::cerr << "TestPlayer::play_and_next playing now "
                      << +evt->toTaoCmd() << std::endl;
    // getType is not usable after the simulate
    // [TODO] change control events to cleanly handle type
    quint32 evtType = evt->getType();

    // Simulate this event
    evt->simulateNow(synchroBasic::base->widget);
    // Refresh the screen
    synchroBasic::tao->refreshOn(QEvent::Timer, -1.0);

    // Keep checkPoint events for result computation
    if (evtType == CHECK_EVENT_TYPE)
        cpList.append(static_cast<TaoCheckEvent*>(evt));
    else
        delete evt;

    // Remove this event from the list
    playingList.removeFirst();
    if (playingList.isEmpty())
    {
        // Do not call base->stop directly otherwise the refresh won't take place
        disconnect(&timer, SIGNAL(timeout()), this, SLOT(play_and_next()));
        connect(&timer, SIGNAL(timeout()), this, SLOT(stop()));
        timer.start(500);
        //synchroBasic::base->stop();
        return;
    }

    // Prepare next event
    evt = playingList.first();
    timer.start(evt->getDelay());
}


void TestPlayer::stop()
// ----------------------------------------------------------------------------
//   Stop the test
// ----------------------------------------------------------------------------
{
    synchroBasic::base->stop();
}


bool TestPlayer::beforeStop()
// ----------------------------------------------------------------------------
//   Prepare to stop the test
// ----------------------------------------------------------------------------
{
    synchroBasic::tao->refreshOn(QEvent::Timer, -1.0);
    return true;
}


bool TestPlayer::afterStop()
// ----------------------------------------------------------------------------
//   Stop the test
// ----------------------------------------------------------------------------
{
    state = stopped;
    timer.stop();
    foreach(TaoControlEvent * evt, playingList)
        delete evt;
    playingList.clear();
    bool hasError = false;
    if (makeRef)
        hasError = saveRef();
    else
        hasError = computeResult();

    QCoreApplication::exit(hasError);
    return hasError; // On sait jamais...
}


bool TestPlayer::saveRef()
// ----------------------------------------------------------------------------
//   Save the played test as a reference
// ----------------------------------------------------------------------------
{
    QString folder = getDir(true);
    uint nbChkPtKO = 0;
    // Compute each checkpoint
    foreach (TaoCheckEvent* cp, cpList)
    {
        if (! cp->playedImage || cp->playedImage->isNull())
        {
            std::cerr<< "WARNING: Check point " << cp->number << " has no image.\n";
            delete cp;
            nbChkPtKO++;
            continue;
        }
        QString filename = QString("%1/%2_%3.png").arg(folder)
                               .arg(name).arg(cp->number);
        cp->playedImage->save(filename);
        delete cp;
    }
    cpList.clear();

    return nbChkPtKO > 0;
}


bool TestPlayer::computeResult()
// ----------------------------------------------------------------------------
//   Compute and log the test's result
// ----------------------------------------------------------------------------
{
    QString runFolder = getDir(false);
    QString refFolder = getDir(true);
    uint nbChkPtKO = 0;
    logOpen();

    std::cerr << "BEGIN TEST " << +name << " id " << featureId << std::endl
              << +description << std::endl;

    // Compute each checkpoint
    foreach (TaoCheckEvent* cp, cpList)
    {
        QString imageName = QString("%1/%2_%3.png").
                arg(refFolder).arg(name).arg(cp->number);
        QImage refImage(imageName);
        if (refImage.isNull())
        {
            std::cerr << "WARNING Reference image "
                      << +imageName << " not found\n";
            delete cp;
            continue;
        }
        QString diffFilename = QString("%1/%2_diff_%3.png").arg(runFolder)
                               .arg(name).arg(cp->number);
        if (!cp->playedImage)
            cp->playedImage = new QImage;
        double resDiff = diff(refImage, *cp->playedImage, diffFilename,
                              cp->threshold);

        // Log an error on stderr
        std::cerr << "\t Intermediate check " << cp->number ;

        bool inError = resDiff > cp->threshold;
        if (inError)
        {
            // Save the image
            QString cpName = QString("%1/%2_played_%3.jpg")
                    .arg(runFolder).arg(name).arg(cp->number);
            cp->playedImage->save(cpName);

            // log an error
            std::cerr<< " fails. Diff is " << resDiff << "%\n";

            // increment the count of error
            nbChkPtKO++;
        }
        else
            std::cerr<< " succeeds. Diff is " << resDiff << "%\n";
        log(cp->number, !inError, resDiff);

        delete cp;
    }
    cpList.clear();

    std::cerr << "END TEST " << +name << " id " << featureId
              << (nbChkPtKO > 0 ? " FAILED\n" : " SUCCEDED\n");

    logClose(nbChkPtKO <= 0);
    return nbChkPtKO > 0;
}


double TestPlayer::diff(QImage &ref, QImage &played, QString filename, double threshold,
                         bool forceSave)
// ----------------------------------------------------------------------------
// Generate an image with just the different pixel in red.
// ----------------------------------------------------------------------------
{
    if (played.isNull())
    {
        log("Played image is null.");
        return 100.0;
    }
    if (ref.size() != played.size())
    {
        QString msg = QString("Image size differs: reference is (%1 x %2) and"
                              " played is (%3 x %4).").
               arg(ref.size().width()).
               arg(ref.size().height()).
               arg(played.size().width()).
               arg(played.size().height());
        log(msg);
        return 100.0;
    }
//    if (ref.format() != played.format())
//        played = played.convertToFormat(ref.format());

    int nbDiff = 0;
    QImage imgDiff(ref.size(), ref.format());

    for (int line = 0; line < ref.height(); line++)
    {
        for (int col = 0; col < ref.width(); col++)
        {
            QRgb r = ref.pixel(col, line);
            QRgb p = played.pixel(col, line);
            if (r != p)
            {
                imgDiff.setPixel(col, line, (r&p) | (~(r|p)));
                nbDiff++;
            }
            else
                imgDiff.setPixel(col, line, QColor(Qt::white).rgba());
        }
    }

    double res = (nbDiff * 100.0)/(ref.width()* ref.height());

    if (!filename.isEmpty() && (res > threshold || forceSave))
        imgDiff.save(filename, "PNG");
    IFTRACE(tester)
            std::cerr << "diff for " << +filename << " with res="
                      << res << std::endl;
    return res;
}


// ============================================================================
//
//   Test player Log facilities
//
// ============================================================================


void TestPlayer::logOpen()
// ----------------------------------------------------------------------------
// Open the log file
// ----------------------------------------------------------------------------
{
    QString folder = getDir(false);
    logfile  = new QFile(folder+"/results.xl");
    logfile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    QTextStream resFile(logfile);
    resFile << INDENT1 << "result \"" << name << "\", <<"
            << description << ">>, "
            << featureId << ", {";
}


void TestPlayer::logClose(bool result)
// ----------------------------------------------------------------------------
// Close the log file
// ----------------------------------------------------------------------------
{
    QTextStream resFile(logfile);
    resFile << "}, " << (result ? "true" : "false") << "\n\n";
    resFile.flush();
    logfile->close();
    delete logfile;
}


void TestPlayer::log(int No, bool isOK, double Tx)
// ----------------------------------------------------------------------------
// log the result of a check point
// ----------------------------------------------------------------------------
{
    QTextStream resFile(logfile);
    resFile << "\n" << INDENT2 << "check " << No << ", "
            << (isOK ? "true": "false") << ", " << Tx;
}


void TestPlayer::log(QString msg)
// ----------------------------------------------------------------------------
//  log a message for next check point result
// ----------------------------------------------------------------------------
{
    QTextStream resFile(logfile);
    resFile << "\n" << INDENT2 << "paragraph_break\n"<<
            INDENT2 << "text <<Following check reports: "<< msg << ">>";
}
