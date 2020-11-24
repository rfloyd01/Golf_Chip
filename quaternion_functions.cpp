#include "pch.h"

#include <iostream>
#include <Eigen/Dense.h>
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
void QuatRotate(glm::quat q, glm::vec3& data)
{
    //Takes the glm::vec3 data and rotates it according to quaternion q

    double w, x, y, z;

    w = 0;
    x = data[0];
    y = data[1];
    z = data[2];

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
/*
void getEllipsePoint(float roll, float pitch, float yaw, float xr, float yr, float zr, float x_off, float y_off, float z_off, float u, float v, float& x, float& y, float& z)
{
    //This function takes an ellipse defined by the first 9 parrameters, applies the parametric values u and v and then updates x, y and z with a point on the ellipse
    float R1[3][3] = { {cosf(yaw), sinf(yaw), 0}, {-sinf(yaw), cosf(yaw), 0}, {0, 0, 1} }; //rotation in z-axis
    float R2[3][3] = { {cosf(pitch), 0, sinf(pitch)}, {0, 1, 0} , {-sinf(pitch), 0, cosf(pitch)} }; //rotation in y-axis
    float R3[3][3] = { {1, 0, 0}, {0, cosf(roll), sinf(roll)}, {0, -sinf(roll), cosf(roll)} }; //rotation in x-axis
    float V[3][1] = { {xr * cosf(u) * cosf(v)}, {yr * sinf(u) * cosf(v)}, {zr * sinf(v)} };

    //Multiply R1 and R2 to get R12
    float R12[3][3] = {
        {R1[0][0] * R2[0][0] + R1[0][1] * R2[1][0] + R1[0][2] * R2[2][0], R1[0][0] * R2[0][1] + R1[0][1] * R2[1][1] + R1[0][2] * R2[2][1], R1[0][0] * R2[0][2] + R1[0][1] * R2[1][2] + R1[0][2] * R2[2][2]},
        {R1[1][0] * R2[0][0] + R1[1][1] * R2[1][0] + R1[1][2] * R2[2][0], R1[1][0] * R2[0][1] + R1[1][1] * R2[1][1] + R1[1][2] * R2[2][1], R1[1][0] * R2[0][2] + R1[1][1] * R2[1][2] + R1[1][2] * R2[2][2]},
        {R1[2][0] * R2[0][0] + R1[2][1] * R2[1][0] + R1[2][2] * R2[2][0], R1[2][0] * R2[0][1] + R1[2][1] * R2[1][1] + R1[2][2] * R2[2][1], R1[2][0] * R2[0][2] + R1[2][1] * R2[1][2] + R1[2][2] * R2[2][2]}
    };

    //multiply R12 by R3 to get R123
    float R123[3][3] = {
        {R12[0][0] * R3[0][0] + R12[0][1] * R3[1][0] + R12[0][2] * R3[2][0], R12[0][0] * R3[0][1] + R12[0][1] * R3[1][1] + R12[0][2] * R3[2][1], R12[0][0] * R3[0][2] + R12[0][1] * R3[1][2] + R12[0][2] * R3[2][2]},
        {R12[1][0] * R3[0][0] + R12[1][1] * R3[1][0] + R12[1][2] * R3[2][0], R12[1][0] * R3[0][1] + R12[1][1] * R3[1][1] + R12[1][2] * R3[2][1], R12[1][0] * R3[0][2] + R12[1][1] * R3[1][2] + R12[1][2] * R3[2][2]},
        {R12[2][0] * R3[0][0] + R12[2][1] * R3[1][0] + R12[2][2] * R3[2][0], R12[2][0] * R3[0][1] + R12[2][1] * R3[1][1] + R12[2][2] * R3[2][1], R12[2][0] * R3[0][2] + R12[2][1] * R3[1][2] + R12[2][2] * R3[2][2]}
    };

    x = R123[0][0] * V[0][0] + R123[0][1] * V[1][0] + R123[0][2] * V[2][0];
    y = R123[1][0] * V[0][0] + R123[1][1] * V[1][0] + R123[1][2] * V[2][0];
    z = R123[2][0] * V[0][0] + R123[2][1] * V[1][0] + R123[2][2] * V[2][0];

    x += x_off;
    y += y_off;
    z += z_off;
}
std::vector<float> ellipseBestFit(std::vector<float>& x_data, std::vector<float>& y_data, std::vector<float>& z_data)
{
    //First gets a best fit solution S to the set of linear equations D * S = e
    //D is an m x 9 matrix where m is the amount of data points in the set and row i of the matrix is: [xi^2 + yi^2 - 2zi^2, xi^2 - 2yi^2 + zi^2, 4xiyi, 2xizi, 2yizi, xi, yi, zi, 1]
    //e is an m x 1 matrix where m is the amount of data points in the set and row i of the matrix is: [xi^2 + yi^2 + zi^2]
    //S is the initial estimate for best fit ellipse of the form S = [U, V, M, N, P, Q, R, S, T]transpose
    //more info can be found here: https://www.researchgate.net/publication/2239930_An_Algorithm_for_Fitting_an_Ellipsoid_to_Data

    Eigen::MatrixXf D(x_data.size(), 9);
    Eigen::VectorXf e(x_data.size());
    float xi, yi, zi;

    //Set Matrices
    for (int i = 0; i < x_data.size(); i++)
    {
        //get current x, y, z
        xi = x_data[i];
        yi = y_data[i];
        zi = z_data[i];

        D(i, 0) = xi * xi + yi * yi - 2 * zi * zi;
        D(i, 1) = xi * xi - 2 * yi * yi + zi * zi;
        D(i, 2) = 4 * xi * yi;
        D(i, 3) = 2 * xi * zi;
        D(i, 4) = 2 * yi * zi;
        D(i, 5) = xi;
        D(i, 6) = yi;
        D(i, 7) = zi;
        D(i, 8) = 1;

        e(i) = xi * xi + yi * yi + zi * zi;
    }

    Eigen::VectorXf S = D.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(e);

    std::cout << "The least-squares solution is:\n" << S << std::endl;

    //Initial estimate for parameters for Newton-Gaussian algorithm S* = [a, b, c, M, N, P, T, U, V]transpose
    //M, N, P, T, U and V were calculated above and correspond to S[2], S[3], S[4], S[8], S[0] and S[1] resspectively
    //a, b and c are the estimates for the center of the best fit ellipse and are initialized to 0

    Eigen::VectorXf S_star(9);
    S_star(0) = 0;
    S_star(1) = 0;
    S_star(2) = 0;
    S_star(3) = S(2);
    S_star(4) = S(3);
    S_star(5) = S(4);
    S_star(6) = S(8);
    S_star(7) = S(0);
    S_star(8) = S(1);

    std::vector<float> yo; //return an empty vector for now so program doesn't crash when returning from this function
    return yo;
}
*/