/*
 * stepper_motor.h
 *
 *  Created on: Jan 24, 2025
 *      Author: micha
 */

#pragma once

#include "main.h"  // Include for GPIO and HAL functions

// Define the number of steps per revolution for your stepper motor
#define stepsperrev 4096  // Update this value based on your motor specification

// Declare an external variable to track the current angle of the motor
extern float currentAngle;

// Function prototypes
void delay(uint16_t us);
void stepper_set_rpm(int rpm);
void stepper_half_drive(int step);
void stepper_step_angle(float angle, int direction, int rpm);  // direction: 0 for CW, 1 for CCW
void Stepper_rotate(int angle, int rpm);
