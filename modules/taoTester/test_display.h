#ifndef TEST_DISPLAY_H
#define TEST_DISPLAY_H
// ****************************************************************************
//  test_display.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//    Tao display module to test presentation in offline mode.
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


#include "tao/module_api.h"
#include <iostream>
#include <QString>

class test_display
{
public:
    test_display(int w, int h);
    virtual ~test_display();
    int                            w, h;
    Tao::ModuleApi::fbo *          frame;
    bool                           shot;
    QString                        filename;
    static void                    display(void *);
    static void *                  use();
    static void                    unuse(void *obj);
    static const Tao::ModuleApi *  tao;
    void                           resize(int w, int h);
    static std::ostream &          debug();

};

extern "C" int module_init(const Tao::ModuleApi *a, const Tao::ModuleInfo *m);

#endif // TEST_DISPLAY_H
