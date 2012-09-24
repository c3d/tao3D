// ****************************************************************************
//  tao_test_events.cpp                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//    Tao events specific for test.
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

#include "tao_test_events.h"
#include "../tao_synchro/event_capture.h"
#include "widgettests.h"

// ****************************************************************************
//   TaoCheckEvent Class
//
// ****************************************************************************


QDataStream &TaoCheckEvent::serializeData(QDataStream &out)
// ----------------------------------------------------------------------------
//  Serialize event specific data.
// ----------------------------------------------------------------------------
{
    out << number;
    out << threshold;
    if (playedImage)
        out << *playedImage;
    else
        out << QImage();
    return out;
}


QDataStream & TaoCheckEvent::unserializeData(QDataStream &in,
                                             quint32)
// ----------------------------------------------------------------------------
//  Unserialize event specific data.
// ----------------------------------------------------------------------------
{
    playedImage = new QImage();
    in >> number;
    in >> threshold;
    in >> *(playedImage);
    return in;
}


void TaoCheckEvent::simulateNow(QWidget */*w*/)
// ----------------------------------------------------------------------------
//  Perform a check againts the reference view.
// ----------------------------------------------------------------------------
{
//    QImage shot = synchroBasic::base->widget->grabFrameBuffer(false);
//    playedImage = new QImage(shot);
    WidgetTests * player =
            dynamic_cast<WidgetTests *>(synchroBasic::base->tao_event_handler);
    if (player)
        player->shotImage = &playedImage;
}


QString TaoCheckEvent::toTaoCmd()
// ----------------------------------------------------------------------------
//  Return a command line for Tao.
// ----------------------------------------------------------------------------
{
    QString cmd = QString("test_add_check %1, %2, %3").
            arg(number).arg(delay).arg(threshold);
    return cmd;
}
