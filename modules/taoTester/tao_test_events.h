// ****************************************************************************
//  tao_test_events.h                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//    Tao events spcific for test.
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
//  (C) 2012 Catherine BURVELLE <Catherine@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************
#ifndef TAO_TEST_EVENTS_H
#define TAO_TEST_EVENTS_H

#include "../tao_synchro/tao_control_event.h"

class TaoCheckEvent: public TaoControlEvent
// ----------------------------------------------------------------------------
//   Class used to store CheckPoint events with other events
// ----------------------------------------------------------------------------
{
public:
    TaoCheckEvent(int num, quint32 delay, double thr = 0.0)
        : TaoControlEvent(delay), number(num), threshold(thr),
          absolutePageTime(0.0), playedImage(NULL) {}

    virtual ~TaoCheckEvent()
    {
        if (playedImage)
        {
            delete playedImage;
            playedImage = NULL;
        }
    }

    virtual QString toTaoCmd();

    virtual QDataStream & serializeData(QDataStream &out);
    virtual QDataStream & unserializeData(QDataStream &in, quint32 e_type);
    virtual void simulateNow(QWidget *w);

    virtual quint32 getType() { return CHECK_EVENT_TYPE;}

public:
    qint16  number;
    double threshold;
    double absolutePageTime;
    QImage * playedImage;
public:
    TaoCheckEvent(quint32 delay)
        : TaoControlEvent(delay) {}

};

#endif // TAO_TEST_EVENTS_H
