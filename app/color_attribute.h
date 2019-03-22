// *****************************************************************************
// color_attribute.h                                               Tao3D project
// *****************************************************************************
//
// File description:
//
//     Generate all the useful variants for color attributes:
//     by name, RGB, HSV, HSL, CMYK, all with or without alpha
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#define COLOR_ATTRIBUTE(base, name, fn)                                 \
PREFIX(base##_namea, tree, name,                                        \
       XPARM(pos, real, )                                               \
       PARM(colorName, text, )                                          \
       PARM(colorAlpha, real, ),                                        \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorByName(colorName, colorAlpha)))), )             \
PREFIX(base##_name, tree, name,                                         \
       XPARM(pos, real, )                                               \
       PARM(colorName, text, ),                                         \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorByName(colorName)))), )                         \
                                                                        \
PREFIX(base##_rgba, tree, name,                                         \
       XPARM(pos, real, )                                               \
       PARM(colorR, real, )                                             \
       PARM(colorG, real, )                                             \
       PARM(colorB, real, )                                             \
       PARM(colorA, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorRGB(colorR,colorG,colorB,colorA)))),)           \
PREFIX(base##_rgb, tree, name,                                          \
       XPARM(pos, real, )                                               \
       PARM(colorR, real, )                                             \
       PARM(colorG, real, )                                             \
       PARM(colorB, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorRGB(colorR,colorG,colorB)))),)                  \
                                                                        \
PREFIX(base##_hsva, tree, name "_hsv",                                  \
       XPARM(pos, real, )                                               \
       PARM(colorH, real, )                                             \
       PARM(colorS, real, )                                             \
       PARM(colorV, real, )                                             \
       PARM(colorA, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorHSV(colorH, colorS,colorV,colorA)))),)          \
PREFIX(base##_hsv, tree, name "_hsv",                                   \
       XPARM(pos, real, )                                               \
       PARM(colorH, real, )                                             \
       PARM(colorS, real, )                                             \
       PARM(colorV, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorHSV(colorH, colorS,colorV)))),)                 \
                                                                        \
PREFIX(base##_hsla, tree, name "_hsl",                                  \
       XPARM(pos, real, )                                               \
       PARM(colorH, real, )                                             \
       PARM(colorS, real, )                                             \
       PARM(colorL, real, )                                             \
       PARM(colorA, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorHSL(colorH, colorS,colorL,colorA)))),)          \
PREFIX(base##_hsl, tree, name "_hsl",                                   \
       XPARM(pos, real, )                                               \
       PARM(colorH, real, )                                             \
       PARM(colorS, real, )                                             \
       PARM(colorL, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorHSL(colorH, colorS,colorL)))),)                 \
                                                                        \
PREFIX(base##_cmyka, tree, name "_cmyk",                                \
       XPARM(pos, real, )                                               \
       PARM(colorC, real, )                                             \
       PARM(colorM, real, )                                             \
       PARM(colorY, real, )                                             \
       PARM(colorK, real, )                                             \
       PARM(colorA, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorCMYK(colorC,colorM, colorY,colorK,colorA)))),)  \
PREFIX(base##_cmyk, tree, name "_cmyk",                                 \
       XPARM(pos, real, )                                               \
       PARM(colorC, real, )                                             \
       PARM(colorM, real, )                                             \
       PARM(colorY, real, )                                             \
       PARM(colorK, real, ),                                            \
       RTAO(fn(self, XARGS(pos)                                         \
               TAO(colorCMYK(colorC,colorM,colorY,colorK)))),)

