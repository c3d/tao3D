#ifndef WIDGETTESTS_H
#define WIDGETTESTS_H
// ****************************************************************************
//  widgettests.h					        Tao project
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

#include "main.h"
#include "utf8.h"

#include <QObject>
#include <QPoint>
#include <QGLWidget>
#include <QTestEventList>
#include <QAction>
#include <QTime>
#include <QColorDialog>
#include <QFontDialog>
#include <QMainWindow>
inline QString operator +(text s)
// ----------------------------------------------------------------------------
//   Quickly convert from text to QString
// ----------------------------------------------------------------------------
{
    return QString::fromUtf8(s.data(), s.length());
}

inline text operator +(QString s)
// ----------------------------------------------------------------------------
//   Quickly convert from QString to text
// ----------------------------------------------------------------------------
{
    return text(s.toUtf8().constData());
}


struct WidgetTests : public QObject
{
Q_OBJECT
public:
    WidgetTests(QGLWidget *widget = NULL, text name = "", text description = "");

    void startRecord();
    void stopRecord();
    void checkNow();
    void save();
    bool play();
    void printResult();
    void reset(text newName = text(), int feature = 0,
               text desc = text(), text folder = text("./"),
               double thr = 0.0, int width = -1, int height = -1);

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
    void addCheck(int num, int delay);
    void addColor(QString diagName, QString name, int delay = -1 );
    void addFont(QString diagName, QString name, int delay = -1  );
    void addDialogClose(QString diagName, int result, int delay = -1);

    // Save utilities
    text toString();
    double diff(QImage &ref, QImage &played, QString filename,
                bool forceSave = false);
    // Spying events on widget
    bool eventFilter(QObject *obj, QEvent *evt);

public slots:
    void recordAction(bool triggered);
    void recordColor(QColor color);
    void recordFont(QFont font);
    void finishedDialog(int result);

public:
    QGLWidget *widget;
    QString    name;
    QString    description;
    int        featureId;// The id in redmine of the feature that this test tests
    QString    folder;
    QImage     playedBefore, playedAfter;
    QImage     before, after;
    QList<QImage> checkPointList;
    QTime      startTime;
    QString    taoCmd;
    int        nbChkPtKO;
    double     threshold;

    QMainWindow * win ;
    QSize         winSize;

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
        : buttons(buttons), modifiers(modifiers), pos(pos), delay(delay)
    {
//        std::cerr << "Recreate mouseMove "<< pos.x()  << ", "<< pos.y() <<
//                ", buttons : " << buttons << std::endl; // CaB
    }
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

        // Move the visual cursor to that pos ==> will throw another mouseMove
        //      event but without the buttons information :(((
//        QCursor::setPos(w->mapToGlobal(pos));
//        qApp->processEvents();
    }

private:
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QPoint pos;
    int delay;
};


class TestColorActionEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store color change events
// ----------------------------------------------------------------------------
{
public:
    inline TestColorActionEvent(QString objName, QString name, int delay)
        : objName(objName), colorName(name), delay(delay) {}
    inline QTestEvent *clone() const { return new TestColorActionEvent(*this); }

    void simulate(QWidget *w);
    QString toTaoCmd();

private:
    QString objName;
    QString colorName;
    int delay;
};

class TestFontActionEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store font change events.
// ----------------------------------------------------------------------------
{
public:
    inline TestFontActionEvent(QString objName, QString name, int delay)
        : objName(objName), fontName(name), delay(delay) {}
    inline QTestEvent *clone() const { return new TestFontActionEvent(*this); }

    void simulate(QWidget *w);
    QString toTaoCmd();

private:
    QString objName;
    QString fontName;
    int delay;
};


class TestDialogActionEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store QAction events with other mouse/key events
// ----------------------------------------------------------------------------
{
public:
    inline TestDialogActionEvent(QString objName, int result, int delay)
        : objName(objName), result(result), delay(delay) {}
    inline QTestEvent *clone() const { return new TestDialogActionEvent(*this); }

    void simulate(QWidget *w);

private:
    QString objName;
    int result;
    int delay;
};


class TestCheckEvent: public QTestEvent
// ----------------------------------------------------------------------------
//   Class used to store QAction events with other mouse/key events
// ----------------------------------------------------------------------------
{
public:
    inline TestCheckEvent(int num, int delay)
        : number(num), delay(delay) {}
    inline QTestEvent *clone() const { return new TestCheckEvent(*this); }

    void simulate(QWidget *w);

    QString toTaoCmd();

private:
    int number;
    int delay;
};



#endif // WIDGETTESTS_H
