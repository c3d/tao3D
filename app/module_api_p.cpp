// *****************************************************************************
// module_api_p.cpp                                                Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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
#else // CFG_NO_LICENSE
    hasLicense = returnFalse;
    checkLicense = returnFalse2;
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
    textureImage = TextureCacheAPI::textureImage;
    textureID = TextureCacheAPI::textureID;
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
