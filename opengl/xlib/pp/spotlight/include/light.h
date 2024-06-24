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
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#endif // !LIGHT_H
