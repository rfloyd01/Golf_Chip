#pragma once

#include <vector>
#include <Header_Files/glm.h>

glm::quat Madgwick(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float delta_t, float beta);
glm::quat MadgwickModified(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float delta_t, float beta);
glm::quat MadgwickIMU(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float delta_t, float beta);
glm::quat Floyd(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float bx, float by, float bz, float delta_t, float beta); //attempt at making my own algorithm

float invSqrt(float x);
