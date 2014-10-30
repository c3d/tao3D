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
#include "display_driver.h"
#include "frame.h"
#include "license.h"
#include "info_trash_can.h"
#include "application.h"
#include "window.h"
#include "texture_cache.h"
#include "file_monitor.h"

TAO_BEGIN

#ifdef CFG_NO_LICENSE
static bool returnFalse(text)           { return false; }
static bool returnFalse2(text, bool)    { return false; }
#endif // CFG_NO_LICENSE

ModuleApiPrivate::ModuleApiPrivate()
// ------------------------------------------------------------------------
//   Set function pointers for all functions exported to modules
// ------------------------------------------------------------------------
{
    scheduleRender   = ModuleRenderer::ScheduleRender;
    refreshOn        = Widget::refreshOnAPI;
    postEvent        = Widget::postEventAPI;
    postEventOnce    = Widget::postEventOnceAPI;
    currentTime      = Widget::currentTimeAPI;
    addToLayout      = ModuleRenderer::AddToLayout;
    AddToLayout2     = ModuleRenderer::AddToLayout2;
    addControlBox    = Widget::addControlBox;

    isGLExtensionAvailable = Widget::isGLExtensionAvailable;

    //Drawing parameters
    SetTextures      = ModuleRenderer::SetTextures;
    SetFillColor     = ModuleRenderer::SetFillColor;
    SetLineColor     = ModuleRenderer::SetLineColor;
    SetShader        = ModuleRenderer::SetShader;
    ModelMatrix      = ModuleRenderer::ModelMatrix;

    deferredDelete   = InfoTrashCan::DeferredDelete;
    makeGLContextCurrent = Widget::makeGLContextCurrent;

    // Display module API
    registerDisplayFunction      = DisplayDriver::registerDisplayFunction;
    registerDisplayFunctionAlias = DisplayDriver::registerDisplayFunctionAlias;
    setGlClearColor              = DisplayDriver::setGlClearColor;
    setupGl                      = DisplayDriver::setupGl;
    showGlErrors                 = DisplayDriver::showGlErrors;
    setStereo                    = DisplayDriver::setStereo;
    setStereoPlanes              = DisplayDriver::setStereoPlanes;
    setProjectionMatrix          = DisplayDriver::setProjectionMatrix;
    setModelViewMatrix           = DisplayDriver::setModelViewMatrix;
    drawScene                    = DisplayDriver::drawScene;
    drawSelection                = DisplayDriver::drawSelection;
    drawActivities               = DisplayDriver::drawActivities;
    getCamera                    = DisplayDriver::getCamera;
    renderHeight                 = DisplayDriver::renderHeight;
    renderWidth                  = DisplayDriver::renderWidth;
    zNear                        = DisplayDriver::zNear;
    zFar                         = DisplayDriver::zFar;
    zoom                         = DisplayDriver::zoom;
    eyeSeparation                = DisplayDriver::eyeSeparation;
    getCurrentEye                = DisplayDriver::getCurrentEye;
    getEyesNumber                = DisplayDriver::getEyesNumber;
    doMouseTracking              = DisplayDriver::doMouseTracking;
    setMouseTrackingViewport     = DisplayDriver::setMouseTrackingViewport;
    setWatermarkText             = Widget::setWatermarkTextAPI;
    drawWatermark                = Widget::drawWatermarkAPI;

    // Framebuffer API
    newFrameBufferObject           = FrameInfo::newFrameBufferObject;
    newFrameBufferObjectWithFormat = FrameInfo::newFrameBufferObjectWithFormat;
    deleteFrameBufferObject        = FrameInfo::deleteFrameBufferObject;
    resizeFrameBufferObject        = FrameInfo::resizeFrameBufferObject;
    bindFrameBufferObject          = FrameInfo::bindFrameBufferObject;
    releaseFrameBufferObject       = FrameInfo::releaseFrameBufferObject;
    frameBufferObjectToTexture     = FrameInfo::frameBufferObjectToTexture;
    frameBufferAttachmentToTexture = FrameInfo::frameBufferAttachmentToTexture;
    imageFromFrameBufferObject   = FrameInfo::imageFromFrameBufferObject;

    // License checking
#ifndef CFG_NO_LICENSE
    hasLicense = Licenses::Has;
    checkLicense = Licenses::Check;
    checkImpressOrLicense = Licenses::CheckImpressOrLicense;
    hasImpressOrLicense = Licenses::HasImpressOrLicense;
#else // CFG_NO_LICENSE
    hasLicense = returnFalse;
    checkLicense = returnFalse2;
    checkImpressOrLicense = returnFalse;
    hasImpressOrLicense = returnFalse;
#endif // CFG_NO_LICENSE
    blink      = Widget::blink;

    // Current document info
    currentDocumentFolder =  Widget::currentDocumentFolder;
    taoRunTime            =  Application::runTime;

    screenNumber = Widget::screenNumber;
    addWidget    = Window::addWidget;
    removeWidget = Window::removeWidget;
    setCurrentWidget    = Window::setCurrentWidget;

    // Texture cache
    textureCacheBind = TextureCacheAPI::bind;
    textureCacheSetMinMagFilters = TextureCacheAPI::setMinMagFilters;

    // File monitor
    newFileMonitor = FileMonitorApi::newFileMonitor;
    fileMonitorAddPath = FileMonitorApi::fileMonitorAddPath;
    fileMonitorRemovePath = FileMonitorApi::fileMonitorRemovePath;
    fileMonitorRemoveAllPaths = FileMonitorApi::fileMonitorRemoveAllPaths;
    deleteFileMonitor = FileMonitorApi::deleteFileMonitor;

    offlineRendering = Widget::offlineRenderingAPI;
    currentPageTime = Widget::currentPageTimeAPI;
    DevicePixelRatio = Widget::DevicePixelRatioAPI;
    RenderingTransparency = Widget::RenderingTransparencyAPI;
}

TAO_END
