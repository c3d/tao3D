// ****************************************************************************
//  module_api_p.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    Implementation of interface with native modules
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "module_api_p.h"
#include "module_renderer.h"
#include "widget.h"

TAO_BEGIN

ModuleApiPrivate::ModuleApiPrivate()
// ------------------------------------------------------------------------
//   Set function pointers for all functions exported to modules
// ------------------------------------------------------------------------
{
    scheduleRender   = ModuleRenderer::ScheduleRender;
    refreshOn        = Widget::refreshOn;
    addToLayout      = ModuleRenderer::AddToLayout;
    addControlBox    = Widget::addControlBox;

    //Drawing paramaters
    SetTexture       = ModuleRenderer::SetTexture;
    SetTexCoords     = ModuleRenderer::SetTexCoords;
    SetFillColor     = ModuleRenderer::SetFillColor;
    SetLineColor     = ModuleRenderer::SetLineColor;

}

TAO_END
