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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "main.h"
#include "utf8.h"

#include "../tao_synchro/event_handler.h"
#include "tao_test_events.h"
#include <QTime>
#include <QTimer>
#include <QFile>


typedef enum PlayerState_ {
    stopped   = 0,
    loading   = 1,
    ready     = 2,
    playing   = 3,
    recording = 4
} PlayerState;

#define BEFORE -1
#define AFTER -2

struct WidgetTests
{
public:
    WidgetTests();

    virtual void reset(text newName = text(), int feature = 0,
                       text desc = text(), int width = -1, int height = -1);

    QString getDir(bool makeRef);
public:
    QString    name;
    QString    description;
    int        featureId;// The id in redmine of the feature that this test tests
    qint16     nbCheckPoint;

    QFile    * logfile;

    PlayerState state;

    QImage ** shotImage;
protected:
    QList<TaoControlEvent*> testList;

};


struct TestRecorder : public TaoEventHandler, WidgetTests
{
public:
    TestRecorder(){}
    // What to do before and after start and stop
    virtual bool beforeStart();
    virtual bool afterStart();
    virtual bool beforeStop();
    virtual bool afterStop();

    // What to do with an event
    // TaoEventHandler take ownership of the TaoControlEvent evt
    virtual void add(TaoControlEvent *evt);

    void checkNow();
    void save();

    // Save utilities
    text toTaoCmd();

};


struct TestPlayer : public TaoEventHandler, WidgetTests
{
    Q_OBJECT
public:
    TestPlayer();
    // What to do before and after start and stop
    virtual bool beforeStart();
    virtual bool afterStart();
    virtual bool beforeStop();
    virtual bool afterStop();

    // What to do with an event
    // TaoEventHandler take ownership of the TaoControlEvent evt
    virtual void add(TaoControlEvent *evt);

    virtual void reset(text newName, int feature, text desc,
                       int width, int height);

    // To play a freshly creadted test set.
    void setTestList(QList<TaoControlEvent*>list);

    // Compute 2 images diff
    double diff(QImage &ref, QImage &played, QString filename,
                double threshold = 0.0, bool forceSave = false);

    // logging results
    void logOpen();
    void log(QString t);
    void log(int No, bool isOK, double Tx);
    void logClose(bool result);

    bool computeResult();
    bool saveRef();

public:
    bool    makeRef;

public slots:
    void play_and_next();
    void stop();

private:
    // This list will hold a copy of the testList for playing.
    // It should not be modified while playing, except clearing it to stop the play.
    QList<TaoControlEvent*> playingList;
    QList<TaoCheckEvent*> cpList;

    QTimer timer;
};


#endif // WIDGETTESTS_H
