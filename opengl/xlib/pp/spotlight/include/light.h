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
    vec3  position;
    vec3  direction;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;

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

    inline void setConstantAttenuation(const float val)
    {
        constant = val;
    }

    inline float getConstantAttenuation()
    {
        return constant;
    }

    inline void setLinearAttenuation(const float val)
    {
        linear = val;
    }

    inline float getLinearAttenuation()
    {
        return linear;
    }

    inline void setQuadraticAttenuation(const float val)
    {
        quadratic = val;
    }

    inline float getQuadraticAttenuation()
    {
        return quadratic;
    }

    inline void SetOuterCutOff(const float val)
    {
        outerCutOff = val;
    }

    inline float getOuterCutOff()
    {
        return outerCutOff;
    }

    inline void SetCutOff(const float val)
    {
        cutOff = val;
    }

    inline float getCutOff()
    {
        return cutOff;
    }
};

struct LightUniform
{
    unsigned int position;  /**< Position uniform of light*/
    unsigned int direction; /**< Direction uniform of light */
    unsigned int ambient;   /**< Ambient uniform of light */
    unsigned int diffuse;   /**< Diffuse uniform of light */
    unsigned int specular;  /**< Specular uniform of light */
    unsigned int constant;  /**< Specular uniform of light */
    unsigned int linear;    /**< Specular uniform of light */
    unsigned int quadratic; /**< Specular uniform of light */
    unsigned int cutOff; /**< Specular uniform of light */
    unsigned int outerCutOff; /**< Specular uniform of light */

};

#endif // !LIGHT_H
