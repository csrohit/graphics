#ifndef MATERIAL_H
#define MATERIAL_H

/**
 * @file
 *   light.cpp
 *
 * @brief
 *   Definations of methods declared in light.h
 */

#include "vmath.h"
using namespace vmath;

struct Material
{
    vec4  ambient;   /**< Ambient component of material */
    vec4  diffuse;   /**< Diffuse component of material */
    vec4  specular;  /**< Specular component of material */
    float shininess; /**< Shininess component of material */

    inline void setAmbient(const vec4 val)
    {
        ambient[0] = val[0];
        ambient[1] = val[1];
        ambient[2] = val[2];
    }

    inline vec4& getAmbient()
    {
        return ambient;
    }

    inline void setDiffuse(const vec4 val)
    {
        diffuse[0] = val[0];
        diffuse[1] = val[1];
        diffuse[2] = val[2];
    }

    inline vec4& getDiffuse()
    {
        return diffuse;
    }

    inline void setSpecular(const vec4 val)
    {
        specular[0] = val[0];
        specular[1] = val[1];
        specular[2] = val[2];
    }

    inline vec4& getSpecular()
    {
        return specular;
    }

    inline void setShininess(const float val)
    {
        shininess = val;
    }

    inline float getShininess()
    {
        return shininess;
    }

};

struct MaterialUniform
{
    unsigned int ambient;   /**< Ambient uniform of material */
    unsigned int diffuse;   /**< Diffuse uniform of material */
    unsigned int specular;  /**< Specular uniform of material */
    unsigned int shininess; /**< Shininess uniform of material */
};

#endif // !MATERIAL_H
