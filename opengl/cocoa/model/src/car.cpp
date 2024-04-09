/**
 * @file        car.cpp
 * @description Load and display car
 * @author      Rohit Nimkar
 * @version     1.0
 * @date        2023-12-10
 * @copyright   Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

#include "car.h"
#include "glm.h"
#include <cstdlib>

static GLMmodel *pCar = nullptr;

void initializeCar()
{
    pCar = glmReadOBJ((char *)"cube.obj");
    if (nullptr == pCar)
    {
    }
    glmUnitize(pCar);
    glmFacetNormals(pCar);
    glmVertexNormals(pCar, 90.0f);
}

void updateCar()
{
}

void displayCar()
{
    glmDraw(pCar, GLM_SMOOTH | GLM_MATERIAL);
}

void freeCar()
{
    if (nullptr != pCar)
    {
        free(pCar);
        pCar = nullptr;
    }
}
