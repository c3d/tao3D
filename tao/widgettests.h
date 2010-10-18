#ifndef WIDGETTESTS_H
#define WIDGETTESTS_H
// ****************************************************************************
//  widgettests.h							   Tao project
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

#include <QObject>
#include <QPoint>
#include "main.h"

#ifdef QT_TESTLIB_LIB
#include <QTestEventList>
#include <QAction>
#include <QTime>

namespace Tao {

struct Widget;

struct WidgetTests : public QObject
{
Q_OBJECT
public:
    WidgetTests(Widget *widget, text name = "", text description = "");

    void startRecord();
    void stopRecord();
    void save();
    bool play();
    void printResult();
    void reset(text newName = text(), int feature = 0, text desc = text());

    void addKeyPress(Qt::Key qtKey,
                     Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                     int msecs = -1 );
    void addKeyRelease(Qt::Key qtKey,
                       Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                       int msecs = -1 );
    void addMousePress(Qt::MouseButton button,
                       Qt::KeyboardModifiers modifiers = 0,
                       QPoint pos = QPoint(), int delay = -1 );
    void addMouseRelease(Qt::MouseButton button,
                         Qt::KeyboardModifiers modifiers = 0,
                         QPoint pos = QPoint(), int delay = -1 );
    void addMouseMove(Qt::MouseButtons buttons,
                       Qt::KeyboardModifiers modifiers = 0,
                       QPoint pos = QPoint(), int delay = -1 );
    void addMouseDClick(Qt::MouseButton button,
                        Qt::KeyboardModifiers modifiers = 0,
                        QPoint pos = QPoint(), int delay = -1 );
    void addAction(QString name, int delay = -1 );


    // Save utilities
    text toString();
    // Spying events on widget
    bool eventFilter(QObject *obj, QEvent *evt);

public slots:
    void recordAction(bool triggered);

public:
    Widget *widget;
    text    name;
    text    description;
    int     featureId;// The id in redmine of the feature that this test tests
    bool    latestResult;
    text    folder;
    QImage  playedBefore, playedAfter;
    QImage  before, after;
    QTime   startTime;
    text taoCmd;
protected:
    QTestEventList testList;

//private slots:
//    void initTestCase()
//    {
//        qDebug("called before everything else");
//    }

//    void cleanupTestCase()
//    {
//        qDebug("called after all tests");
//    }

};

class TestActionEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store QAction events with other mouse/key events
// ----------------------------------------------------------------------------
{
public:
    inline TestActionEvent(QString name, int delay)
        : _name(name), _delay(delay) {}
    inline QTestEvent *clone() const { return new TestActionEvent(*this); }

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

class TestMouseMoveEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store QAction events with other mouse/key events
// ----------------------------------------------------------------------------
{
public:
    inline TestMouseMoveEvent( Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers,
                               QPoint pos, int delay=-1)
        : buttons(buttons), modifiers(modifiers), pos(pos), delay(delay) {}
    inline QTestEvent *clone() const { return new TestMouseMoveEvent(*this); }

    inline void simulate(QWidget *w)
    {
        QTest::qWait(delay);
        QMouseEvent me(QEvent::MouseMove, pos, w->mapToGlobal(pos), Qt::NoButton,
                       buttons, modifiers);
        QSpontaneKeyEvent::setSpontaneous(&me);
        if (!qApp->notify(w, &me)) {
            QTest::qWarn("Mouse event \"MouseMove\" not accepted by receiving widget");
        }
//        // Move the visual cursor to that pos ==> will throw another mouseMove event :(((
//        QCursor::setPos(w->mapToGlobal(pos));
//        qApp->processEvents();
    }

private:
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QPoint pos;
    int delay;
};

} // end Tao namespace

#else
// =================================================================
// ===  USED WHEN TEST IS NOT DEFINED AT BUILD TIME
// =================================================================

namespace Tao {

struct Widget;

#pragma GCC diagnostic ignored "-Wunused-parameter"
struct WidgetTests : public QObject
{

public:
    WidgetTests(Widget *widget, text name = "", text description = "")
    {}

    void startRecord() {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }

    void stopRecord() {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }

    void save() {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }

    bool play() {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
        return true;
    }

    void printResult() {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }

    void reset(text newName = text(), int feature = 0, text desc = text()) {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }

    void addKeyPress(Qt::Key qtKey,
                     Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                     int msecs = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }
    void addKeyRelease(Qt::Key qtKey,
                       Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                       int msecs = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }
    void addMousePress(Qt::MouseButton button,
                       Qt::KeyboardModifiers modifiers = 0,
                       QPoint pos = QPoint(), int delay = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }
    void addMouseRelease(Qt::MouseButton button,
                         Qt::KeyboardModifiers modifiers = 0,
                         QPoint pos = QPoint(), int delay = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }
    void addMouseMove(Qt::MouseButtons buttons,
                       Qt::KeyboardModifiers modifiers = 0,
                       QPoint pos = QPoint(), int delay = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }
    void addMouseDClick(Qt::MouseButton button,
                        Qt::KeyboardModifiers modifiers = 0,
                        QPoint pos = QPoint(), int delay = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }
    void addAction(QString name, int delay = -1 )
    {
        qWarning("Rerun qmake with option \"QT+=testlib\"");
    }


};
#pragma GCC diagnostic error "-Wunused-parameter"
}
#endif
#endif // WIDGETTESTS_H
