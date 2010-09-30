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

namespace Tao {

struct Widget;

struct WidgetTests : public QObject
{
Q_OBJECT
public:
    WidgetTests(Widget *widget):widget(widget){};
    WidgetTests(Widget *widget, QString name):
            widget(widget), name(name){};

    void startRecord();
    void stopRecord();
    void load(QString newName = QString());
    void save(QString newName = QString());
    bool play();

    bool eventFilter(QObject *obj, QEvent *evt);

public slots:
    void recordAction(bool triggered);

public:
    Widget *widget;
    QString name;
    QImage  playedBefore, playedAfter;
    QImage  before, after;
    QTime   startTime;
    QTestEventList testList;

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
}
#endif // WIDGETTESTS_H
