#pragma once

#include <vector>
#include <Header_Files/glm.h>

float DotProduct(std::vector<float> vec1, std::vector<float> vec2);
std::vector<float> CrossProduct(std::vector<float> vec1, std::vector<float> vec2);

float Magnitude(std::vector<float> vec);
void Normalize(glm::quat& q);
void Normalize(std::vector<float> &vec);

void QuatRotate(glm::quat q, std::vector<float>& data);
void QuatRotate(glm::quat q1, glm::quat& q2);
glm::quat QuaternionMultiply(glm::quat q1, glm::quat q2);
glm::quat GetRotationQuaternion(std::vector<float> vec1, std::vector<float> vec2);
glm::quat GetRotationQuaternion(float angle, std::vector<float> vec);
glm::quat Conjugate(glm::quat q);

void matrixMultiply(float* m1, int rows1, int columns1, float* m2, int rows2, int columns2, float* prod);
