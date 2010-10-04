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
#include <QTestEventList>
#include <QAction>
#include "main.h"
#include "tao_utf8.h"

namespace Tao {

struct Widget;

struct WidgetTests : public QObject
{
Q_OBJECT
public:
    WidgetTests(Widget *widget, text name = "", text description = ""):
            widget(widget), name(name), description(description),
            latestResult(false){};

    void startRecord();
    void stopRecord();
    void load(text newName = text());
    void save(text newName = text());
    bool play();
    void printResult();
    void reset(text newName = text(), text desc = text());
    bool eventFilter(QObject *obj, QEvent *evt);

    // Save utilities
    text *toString()
    {
        text* testDoc = new text(name);
        testDoc->append("_test -> test_definition ");
        testDoc->append("\"").append(name).append("\", <<").append(description)
                .append(">>, do \n").append(taoCmd);
        return testDoc;
    }


public slots:
    void recordAction(bool triggered);

public:
    Widget *widget;
    text    name;
    text    description;
    bool    latestResult;
    QImage  playedBefore, playedAfter;
    QImage  before, after;
    QTime   startTime;
    QTestEventList testList;
    text taoCmd;

private slots:
    void initTestCase()
    {
        qDebug("called before everything else");
    }

    void cleanupTestCase()
    {
        qDebug("called after all tests");
    }

};

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

}
#endif // WIDGETTESTS_H
