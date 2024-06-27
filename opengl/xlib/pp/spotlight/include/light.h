#ifndef LIGHT_H
#define LIGHT_H

/**
 * @file
 *   light.cpp
 *
 * @brief
 *   Definations of methods declared in light.h
 */

#include "vmath.h"
using namespace vmath;

/*
 * Directional Light: When the light source is modeled to be infinitely far away, it is called Directional light
 */

struct Light
{
    vec3 position;  /**< Position light in eye coordinates */
    vec3 direction; /**< Direction of light rays */
    vec3 ambient;   /**< Ambient component of light */
    vec3 diffuse;   /**< Diffuse component of light */
    vec3 specular;  /**< Specular component of light */

    inline void setAmbient(const vec3 val)
    {
        ambient[0] = val[0];
        ambient[1] = val[1];
        ambient[2] = val[2];
    }

    inline vec3& getAmbient()
    {
        return ambient;
    }

    inline void setDiffuse(const vec3 val)
    {
        diffuse[0] = val[0];
        diffuse[1] = val[1];
        diffuse[2] = val[2];
    }

    inline vec3& getDiffuse()
    {
        return diffuse;
    }

    inline void setSpecular(const vec3 val)
    {
        specular[0] = val[0];
        specular[1] = val[1];
        specular[2] = val[2];
    }

    inline vec3& getSpecular()
    {
        return specular;
    }

    inline void setDirection(const vec3 val)
    {
        direction[0] = val[0];
        direction[1] = val[1];
        direction[2] = val[2];
    }

    inline vec3& getDirection()
    {
        return direction;
    }

    inline void setPosition(const vec3 val)
    {
        position[0] = val[0];
        position[1] = val[1];
        position[2] = val[2];
    }

    inline vec3& getPosition()
    {
        return position;
    }
};

struct LightUniform
{
    unsigned int position;  /**< Position uniform of light*/
    unsigned int direction; /**< Direction uniform of light */
    unsigned int ambient;   /**< Ambient uniform of light */
    unsigned int diffuse;   /**< Diffuse uniform of light */
    unsigned int specular;  /**< Specular uniform of light */
};

#endif // !LIGHT_H
