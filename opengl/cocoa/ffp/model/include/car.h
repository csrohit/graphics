/**
 * @file        car.h
 * @description Declarations for displaying and loading car
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

#ifndef CAR_H
#define CAR_H

/**
 * @brief Load model and allocate resources
 */
void initializeCar();

/**
 * @brief dislay the car in view
 */
void displayCar();

/**
 * @brief update state of car
 */
void updateCar();

/**
 * @brief Release resources for car
 */
void freeCar();



#endif // !CAR_H
