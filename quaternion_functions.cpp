#include "pch.h"

#include <iostream>
#include <Header_Files/quaternion_functions.h>

//Dot and Cross product functions
float DotProduct(std::vector<float> vec1, std::vector<float> vec2)
{
    //returns the dot product of two vectors, the vectors need to be the same size
    if (vec1.size() != vec2.size())
    {
        std::cout << "Vectors must be of the same length." << std::endl;
        return 0;
    }

    float answer = 0;
    for (int i = 0; i < vec1.size(); i++) answer += vec1[i] * vec2[i];
    return answer;
}
std::vector<float> CrossProduct(std::vector<float> vec1, std::vector<float> vec2)
{
    std::vector<float> answer;
    if (vec1.size() != vec2.size())
    {
        std::cout << "Vectors must be of the same length." << std::endl;
        return answer;
    }

    answer.push_back(vec1[1] * vec2[2] - vec1[2] * vec2[1]);
    answer.push_back(vec1[2] * vec2[0] - vec1[0] * vec2[2]);
    answer.push_back(vec1[0] * vec2[1] - vec1[1] * vec2[0]);
    return answer;
}

//Normalizing and magnitude functions
float Magnitude(std::vector<float> vec)
{
    //returns the magnitude of vec
    float answer = 0;
    for (int i = 0; i < vec.size(); i++) answer += vec[i] * vec[i];
    return sqrt(answer);
}
void Normalize(glm::quat& q)
{
    float magnitude = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q.w /= magnitude;
    q.x /= magnitude;
    q.y /= magnitude;
    q.z /= magnitude;
}
void Normalize(std::vector<float> &vec)
{
    float magnitude = Magnitude(vec);
    for (int i = 0; i < vec.size(); i++) vec[i] /= magnitude;
}

//Quaternion Manipulation functions
void QuatRotate(glm::quat q, std::vector<float>& data)
{
    //Takes the float vector data and rotates it according to quaternion q

    double w, x, y, z;

    if (data.size() == 3)
    {
        //a 3-diomensional vector is passed
        w = 0;
        x = data[0];
        y = data[1];
        z = data[2];
    }
    else
    {
        std::cout << "Need a three dimensional vector." << std::endl;
        return;
    }

    glm::quat q_star; q_star.w = q.w; q_star.x = -q.x; q_star.y = -q.y; q_star.z = -q.z;

    double temp[4] = { 0 };
    temp[0] = q.w * w - q.x * x - q.y * y - q.z * z;
    temp[1] = q.w * x + q.x * w + q.y * z - q.z * y;
    temp[2] = q.w * y - q.x * z + q.y * w + q.z * x;
    temp[3] = q.w * z + q.x * y - q.y * x + q.z * w;

    w = temp[0]; x = temp[1]; y = temp[2]; z = temp[3];

    temp[0] = w * q_star.w - x * q_star.x - y * q_star.y - z * q_star.z;
    data[0] = w * q_star.x + x * q_star.w + y * q_star.z - z * q_star.y;
    data[1] = w * q_star.y - x * q_star.z + y * q_star.w + z * q_star.x;
    data[2] = w * q_star.z + x * q_star.y - y * q_star.x + z * q_star.w;
}
void QuatRotate(glm::quat q1, glm::quat& q2)
{
    //Takes the quaternion q2 and rotates it according to quaternion q1

    double w, x, y, z;
    glm::quat q_star; q_star.w = q1.w; q_star.x = -q1.x; q_star.y = -q1.y; q_star.z = -q1.z;

    double temp[4] = { 0 };
    temp[0] = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    temp[1] = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    temp[2] = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    temp[3] = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    w = temp[0]; x = temp[1]; y = temp[2]; z = temp[3];

    q2.w = w * q_star.w - x * q_star.x - y * q_star.y - z * q_star.z;
    q2.x = w * q_star.x + x * q_star.w + y * q_star.z - z * q_star.y;
    q2.y = w * q_star.y - x * q_star.z + y * q_star.w + z * q_star.x;
    q2.z = w * q_star.z + x * q_star.y - y * q_star.x + z * q_star.w;
}
glm::quat QuaternionMultiply(glm::quat q1, glm::quat q2)
{
    glm::quat new_q;

    new_q.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    new_q.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    new_q.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    new_q.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    return new_q;
}
glm::quat GetRotationQuaternion(std::vector<float> vec1, std::vector<float> vec2)
{
    //returns the Quaternion that rotates vec1 to vec2
    glm::quat q;
    std::vector<float> cross = CrossProduct(vec1, vec2);
    q.w = sqrt(Magnitude(vec1) * Magnitude(vec1) * Magnitude(vec2) * Magnitude(vec2)) + DotProduct(vec1, vec2);
    q.x = cross[0];
    q.y = cross[1];
    q.z = cross[2];

    float mag = Magnitude({ q.w, q.x, q.y, q.z });
    q.w /= mag; q.x /= mag; q.y /= mag; q.z /= mag;
    return q;
}
glm::quat GetRotationQuaternion(float angle, std::vector<float> vec)
{
    //returns the Quaternion that rotates about the axis defined by vec, by and angle of angle
    glm::quat q;
    q.w = cos(angle * 3.14159 / 360);
    q.x = vec[0] * sin(angle * 3.14159 / 360);
    q.y = vec[0] * sin(angle * 3.14159 / 360);
    q.z = vec[0] * sin(angle * 3.14159 / 360);

    float mag = Magnitude({ q.w, q.x, q.y, q.z });
    q.w /= mag; q.x /= mag; q.y /= mag; q.z /= mag;
    return q;
}
glm::quat Conjugate(glm::quat q)
{
    glm::quat yo;
    yo.w = q.w;
    yo.x = -q.x;
    yo.y = -q.y;
    yo.z = -q.z;

    return yo;
}

//Matrix functions
void matrixMultiply(float* m1, int rows1, int columns1, float* m2, int rows2, int columns2, float* prod)
{
    //takes two arrays of float and multiplies them, answer is stored in prod
    //there's no way to figure out the size of an array in c++ just by looking at pointers, so the rows and columns of m1 and m2 are supplied

    if (columns1 != rows2)
    {
        std::cout << "Matrices aren't compatible sizes for multiplication. Answer Matrix is null." << std::endl;
        return;
    }

    int ans = 0;
    for (int row1 = 0; row1 < rows1; row1++)
    {
        for (int col2 = 0; col2 < columns2; col2++)
        {
            float answer = 0;
            for (int col1 = 0; col1 < columns1; col1++)
            {
                answer += *(m1 + columns1 * row1 + col1) * *(m2 + columns2 * col1 + col2);
            }
            *(prod+ans) = answer;
            ans++;
        }
    }
}