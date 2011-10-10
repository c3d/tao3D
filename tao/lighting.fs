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

// Bitmasks of activated lights
// and textures
uniform  int  textures;
uniform  int  lights;

// textures parameters
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

varying vec3 viewDir;
varying vec3 normal;
varying vec4 color;

// lighting parameters
vec4 ambient;
vec4 diffuse;
vec4 specular;

vec3 N;
vec3 V;

// Maximum supported lights according to
// OpenGL specifications
const int MAX_LIGHTS = 8;

/*
* Compute influence of the light i (equivalent to GL_LIGHTi),
* which is positional because it depends of this position,
*/
void computePositionalLight(int i)
{
    // Difference between object and light position
    vec3 L = gl_LightSource[i].position.xyz - V;

    // Distance between object and light
    float length = length(L);

    L = normalize(L);

    // Compute attenuation
    float d = (gl_LightSource[i].constantAttenuation +
               gl_LightSource[i].linearAttenuation * length +
               gl_LightSource[i].quadraticAttenuation * length * length);

    float attenuation = 1.0 / d;

    // Compute ambient part
    ambient += gl_LightSource[i].ambient
               * attenuation;

    // Diffuse coefficient
    float nDotL = max(clamp(dot(L, N), 0.0, 1.0), 0.0);
    if (nDotL > 0.0)
    {
        // Compute diffuse part
        diffuse += gl_LightSource[i].diffuse
                   * nDotL
                   * attenuation;

        // Compute specular coefficient
        float nDotV = clamp(dot(reflect(-L, N), V), 0.0, 1.0);
        if (nDotV > 0.0)
        {
            // Compute specular part
            specular += gl_LightSource[i].specular
                        * pow(nDotV, gl_FrontMaterial.shininess)
                        * attenuation;
        }
    }
}

/*
* Compute influence of the light i (equivalent to GL_LIGHTi),
* which is directional because it is placed at the infinite,
* (for instance: a sun).
*/
void computeDirectionalLight(int i)
{
    // Normalize Light position (the light is supposed at the infinite)
    vec3 L = normalize(gl_LightSource[i].position.xyz);

    // Compute ambient part
    ambient += gl_LightSource[i].ambient;

    // Diffuse coefficient
    float nDotL = max(clamp(dot(L, N), 0.0, 1.0), 0.0);
    if (nDotL > 0.0)
    {
        // Compute diffuse part
        diffuse += gl_LightSource[i].diffuse * nDotL;

        // Compute specular coefficient
        float nDotV = clamp(dot(reflect(-L, N), V), 0.0, 1.0);
        if (nDotV > 0.0)
        {
            // Compute specular part
            specular += gl_LightSource[i].specular
                        * pow(nDotV, gl_FrontMaterial.shininess);
        }
    }
}

/*
* Compute influence of the light i (equivalent to GL_LIGHTi),
* which is spot because it is represented by a cone,
* (for instance: a lamp).
*/
void computeSpotLight(int i)
{
    // Difference between object and light position
    vec3 L = gl_LightSource[i].position.xyz - V;

    // Distance between object and light
    float length = length(L);

    L = normalize(L);

    // Compute attenuation
    float d = (gl_LightSource[i].constantAttenuation +
               gl_LightSource[i].linearAttenuation * length +
               gl_LightSource[i].quadraticAttenuation * length * length);

    float attenuation = 1.0 / d;


    // See if point on surface is inside cone of illumination
    float spotDot = dot(-L, normalize(gl_LightSource[i].spotDirection));

    // Attenuation of the spot
    float spotAttenuation = 0.0;

    if (spotDot >= gl_LightSource[i].spotCosCutoff)
        spotAttenuation = pow(spotDot, gl_LightSource[i].spotExponent);

    // Combine the spotlight and distance attenuation.
    attenuation *= spotAttenuation;

    // Compute ambient part
    ambient += gl_LightSource[i].ambient
               * attenuation;

    // Diffuse coefficient
    float nDotL = max(clamp(dot(L, N), 0.0, 1.0), 0.0);
    if (nDotL > 0.0)
    {
        // Compute diffuse part
        diffuse += gl_LightSource[i].diffuse
                   * nDotL
                   * attenuation;

        // Compute specular coefficient
        float nDotV = clamp(dot(reflect(-L, N), V), 0.0, 1.0);
        if (nDotV > 0.0)
        {
            // Compute specular part
            specular += gl_LightSource[i].specular
                        * pow(nDotV, gl_FrontMaterial.shininess)
                        * attenuation;
        }
    }
}

/*
* Compute influence of the light i (equivalent to GL_LIGHT0 + i),
* which can be a spot but also positional or directional.
*/
void computeLight(int i)
{
    if (gl_LightSource[i].spotCutoff == 180.0)
    {
        if(gl_LightSource[i].position.w == 0.0)
            computeDirectionalLight(i);
        else
            computePositionalLight(i);
    }
    else
    {
        computeSpotLight(i);
    }
}

/*
* Compute influences of all lights and materials in the scene.
*/
void computeLighting()
{
    ambient  = vec4(0.0);
    diffuse  = vec4(0.0);
    specular = vec4(0.0);
    N = normalize(normal);
    V = normalize(viewDir);

    ambient  = gl_FrontLightModelProduct.sceneColor;

    for(int i = 0; i < MAX_LIGHTS; i++)
        if(bool(lights & (1 << i)))
            computeLight(i);

    // Materials parts
    ambient  *= gl_FrontMaterial.ambient;
    diffuse  *= gl_FrontMaterial.diffuse;
    specular *= gl_FrontMaterial.specular;
}

void main(void)
{
    vec4 render_color = color;

    // Compute textures
    if(textures & 1)
        render_color *= texture2D(tex0, gl_TexCoord[0].st);
    if(textures & 2)
        render_color *= texture2D(tex1, gl_TexCoord[1].st);
    if(textures & 4)
        render_color *= texture2D(tex2, gl_TexCoord[2].st);
    if(textures & 8)
        render_color *= texture2D(tex3, gl_TexCoord[3].st);

    if(lights > 0)
    {
       computeLighting();

       // Compute final color
       render_color *= (ambient + diffuse);
       render_color += specular;
    }

    gl_FragColor = render_color;
};
