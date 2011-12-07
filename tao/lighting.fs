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

uniform  int  vendor;

// textures parameters
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

varying vec4 color;
varying vec4 viewDir;
varying vec3 normal;

vec4 ambient;
vec4 diffuse;
vec4 specular;

vec3 N;
vec3 V;

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
        float nDotV = max(0.0, dot(N, gl_LightSource[i].halfVector.xyz));
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
        float nDotV = max(0.0, dot(N, gl_LightSource[i].halfVector.xyz));
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
        float nDotV = max(0.0, dot(N, gl_LightSource[i].halfVector.xyz));
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
    if (gl_LightSource[i].spotCutoff != 180.0)
    {
        computeSpotLight(i);
    }
    else
    {
        if(gl_LightSource[i].position.w == 0.0)
            computeDirectionalLight(i);
        else
            computePositionalLight(i);
    }
}

vec4 computeLighting()
{
    ambient  = vec4 (0.0);
    diffuse  = vec4 (0.0);
    specular = vec4 (0.0);

    N = normalize(normal);
    V = (vec3 (viewDir)) / viewDir.w;

    for(int i = 0; i < 8; i++)
        if(bool(lights & (1 << i)))
            computeLight(i);

    // Materials parts
    ambient  = gl_FrontMaterial.ambient * ambient;
    diffuse  = gl_FrontMaterial.diffuse * diffuse;
    specular = gl_FrontMaterial.specular * specular;

    vec4 globalAmbient = gl_FrontLightModelProduct.sceneColor;

    // If the vendor is no ATI then we use classic calculation, otherwise
    // we have to multiply by the scene color to fix a bug.
    vec4 final_color = vec4(diffuse.rgb + globalAmbient.rgb + ambient.rgb, globalAmbient.a);
    if(vendor == 0)
        final_color *=  color;

    final_color += vec4(specular.rgb, 0.0);

    return final_color;
}

void main (void)
{
    vec4 render_color = color;

    if(lights > 0)
    {
      // Compute final color
       render_color = computeLighting();
    }

    // Compute activated textures
    if(bool(textures & 1))
        render_color *= texture2D(tex0, gl_TexCoord[0].st);
    if(bool(textures & 2))
        render_color *= texture2D(tex1, gl_TexCoord[1].st);
    if(bool(textures & 4))
        render_color *= texture2D(tex2, gl_TexCoord[2].st);
    if(bool(textures & 8))
        render_color *= texture2D(tex3, gl_TexCoord[3].st);

    gl_FragColor = render_color;
}
