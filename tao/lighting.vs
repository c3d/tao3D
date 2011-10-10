/****************************************************************************
**
** Copyright (C) 2011 Taodyne.
** All rights reserved.
** Contact: Taodyne (contact@taodyne.com)
**
** This file is part of the Tao Presentations application, developped by Taodyne.
** It can be only used in the software and these modules.
**
** If you have questions regarding the use of this file, please contact
** Taodyne at contact@taodyne.com.
**
****************************************************************************/

#extension GL_EXT_gpu_shader4 : enable

varying vec3 viewDir;
varying vec3 normal;
varying vec4 color;

void main(void)
{
    gl_Position = ftransform();

    // Compute normal and world position
    normal  = gl_NormalMatrix * gl_Normal;
    viewDir = -vec3(gl_ModelViewMatrix * gl_Vertex);

    // Compute texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
    gl_TexCoord[2] = gl_TextureMatrix[2] * gl_MultiTexCoord2;
    gl_TexCoord[3] = gl_TextureMatrix[3] * gl_MultiTexCoord3;

    // Compute main color
    color = gl_Color;
};
