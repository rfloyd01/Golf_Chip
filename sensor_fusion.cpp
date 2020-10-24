#include "pch.h"

#include <iostream>
#include <Header_Files/sensor_fusion.h>
#include <Header_Files/quaternion_functions.h>

glm::quat Madgwick(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float delta_t, float beta)
{
    //The standard Madgwick algorithm as taken from online
    float recipNorm;
    float s0, s1, s2, s3;
    float hx, hy, hz;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2by, _2bz, _4bx, _4by, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    //convert Gyroscope readings to rad/s
    gx *= 3.14159 / 180.0;
    gy *= 3.14159 / 180.0;
    gz *= 3.14159 / 180.0;

    // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
    // add in call to MadgwickIMU() in the future
    if ((mx == 0.0f) && ((my == 0.0f) && (mz == 0.0f))) return MadgwickIMU(q, gx, gy, gz, ax, ay, az, delta_t, beta);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if ((ax == 0.0f) && ((ay == 0.0f) && (az == 0.0f))) return q;

    // Normalise accelerometer and magnetometer measurements
    recipNorm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;

    recipNorm = invSqrt(mx * mx + my * my + mz * mz);
    mx *= recipNorm;
    my *= recipNorm;
    mz *= recipNorm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0mx = 2.0f * q.w * mx;
    _2q0my = 2.0f * q.w * my;
    _2q0mz = 2.0f * q.w * mz;
    _2q1mx = 2.0f * q.x * mx;
    _2q0 = 2.0f * q.w;
    _2q1 = 2.0f * q.x;
    _2q2 = 2.0f * q.y;
    _2q3 = 2.0f * q.z;
    _2q0q2 = 2.0f * q.w * q.y;
    _2q2q3 = 2.0f * q.y * q.z;
    q0q0 = q.w * q.w;
    q0q1 = q.w * q.x;
    q0q2 = q.w * q.y;
    q0q3 = q.w * q.z;
    q1q1 = q.x * q.x;
    q1q2 = q.x * q.y;
    q1q3 = q.x * q.z;
    q2q2 = q.y * q.y;
    q2q3 = q.y * q.z;
    q3q3 = q.z * q.z;

    /*
    //Calculate the reference direction of Earth's magnetic field, b
    glm::quat h = QuaternionMultiply(q, QuaternionMultiply({ 0, mx, my, mz }, Conjugate(q)));
    glm::quat b = { 0, Magnitude({h.x, h.y}), 0, h.z };

    // Gradient decent algorithm corrective step
    float F[6][1];
    F[0][0] = 2 * (q.x * q.z - q.w * q.y) - ax; //2*(q(2)*q(4) - q(1)*q(3)) - Accelerometer(1)
    F[1][0] = 2 * (q.w * q.x + q.y * q.z) - ay; //2*(q(1)*q(2) + q(3)*q(4)) - Accelerometer(2)
    F[2][0] = 2 * (0.5 - q.x * q.x - q.y * q.y) - az; //2*(0.5 - q(2)^2 - q(3)^2) - Accelerometer(3)
    F[3][0] = 2 * b.x * (0.5 - q.y * q.y - q.z * q.z) + 2 * b.z * (q.x * q.z - q.w * q.y) - mx; //2*b(2)*(0.5 - q(3)^2 - q(4)^2) + 2*b(4)*(q(2)*q(4) - q(1)*q(3)) - Magnetometer(1)
    F[4][0] = 2 * b.x * (q.x * q.y - q.w * q.z) + 2 * b.z * (q.w * q.x + q.y * q.z) - my; //2*b(2)*(q(2)*q(3) - q(1)*q(4)) + 2*b(4)*(q(1)*q(2) + q(3)*q(4)) - Magnetometer(2)
    F[5][0] = 2 * b.x * (q.w * q.y + q.x * q.z) + 2 * b.z * (0.5 - q.x * q.x - q.y * q.y) - mz; //2*b(2)*(q(1)*q(3) + q(2)*q(4)) + 2*b(4)*(0.5 - q(2)^2 - q(3)^2) - Magnetometer(3)]

    float J[6][4];
    J[0][0] = -2 * q.y; J[0][1] = 2 * q.z; J[0][2] = -2 * q.w; J[0][3] = 2 * q.x;
    J[1][0] = 2 * q.x;  J[1][1] = 2 * q.w; J[1][2] = -2 * q.z; J[1][3] = 2 * q.y;
    J[2][0] = 0;        J[2][1] = -4 * q.x; J[2][2] = -4 * q.y; J[2][3] = 0;
    J[3][0] = -2 * b.z * q.y; J[3][1] = 2 * b.z * q.z; J[3][2] = -4 * b.x * q.y - 2 * b.z * q.w; J[3][3] = -4 * b.x * q.z + 2 * b.z * q.x;
    J[4][0] = -2 * b.x * q.z + 2 * b.z * q.x; J[4][1] = 2 * b.x * q.y + 2 * b.z * q.w; J[4][2] = 2 * b.x * q.x + 2 * b.z * q.z; J[4][3] = -2 * b.x * q.w + 2 * b.z * q.y;
    J[5][0] = 2 * b.x * q.y; J[5][1] = 2 * b.x * q.z - 4 * b.z * q.x; J[5][2] = 2 * b.x * q.w - 4 * b.z * q.y; J[5][3] = 2 * b.x * q.x;

    
    glm::quat step;
    step.w = (J[0][0] * F[0][0]) + (J[1][0] * F[1][0]) + (J[2][0] * F[2][0]) + (J[3][0] * F[3][0]) + (J[4][0] * F[4][0]) + (J[5][0] * F[5][0]);
    step.x = (J[0][1] * F[0][0]) + (J[1][1] * F[1][0]) + (J[2][1] * F[2][0]) + (J[3][1] * F[3][0]) + (J[4][1] * F[4][0]) + (J[5][1] * F[5][0]);
    step.y = (J[0][2] * F[0][0]) + (J[1][2] * F[1][0]) + (J[2][2] * F[2][0]) + (J[3][2] * F[3][0]) + (J[4][2] * F[4][0]) + (J[5][2] * F[5][0]);
    step.z = (J[0][3] * F[0][0]) + (J[1][3] * F[1][0]) + (J[2][3] * F[2][0]) + (J[3][3] * F[3][0]) + (J[4][3] * F[4][0]) + (J[5][3] * F[5][0]);

    recipNorm = invSqrt(step.w * step.w + step.x * step.x + step.y * step.y + step.z * step.z); // normalise step magnitude
    step *= recipNorm;
    
    hx = mx * q0q0 - _2q0my * q.z + _2q0mz * q.y + mx * q1q1 + _2q1 * my * q.y + _2q1 * mz * q.z - mx * q2q2 - mx * q3q3;
    hy = _2q0mx * q.z + my * q0q0 - _2q0mz * q.x + _2q1mx * q.y - my * q1q1 + my * q2q2 + _2q2 * mz * q.z - my * q3q3;
    _2bx = sqrt(hx * hx + hy * hy);
    _2bz = -_2q0mx * q.y + _2q0my * q.x + mz * q0q0 + _2q1mx * q.z - mz * q1q1 + _2q2 * my * q.z - mz * q2q2 + mz * q3q3;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;
    */

    glm::quat h = QuaternionMultiply(q, QuaternionMultiply({ 0, mx, my, mz }, Conjugate(q)));
    glm::quat b = { 0, Magnitude({h.x, h.y}), 0, h.z };
    _2bx = sqrt(h.x * h.x + h.y * h.y);
    _2bz = h.z;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;
    //Original Gradient decent code, is much more compact and doesn't take as much storage, however, makes it harder to tell what's going on
    s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q.y * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q.z + _2bz * q.x) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q.y * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
    s1= _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q.x * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q.z * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q.y + _2bz * q.w) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q.z - _4bz * q.x) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
    s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q.y * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q.y - _2bz * q.w) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q.x + _2bz * q.z) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q.w - _4bz * q.y) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
    s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q.z + _2bz * q.x) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q.w + _2bz * q.y) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q.x * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);

    recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    s0 *= recipNorm;
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;

    // Rate of change of quaternion from gyroscope
    glm::quat qDot = QuaternionMultiply(q, { 0, gx, gy, gz });
    qDot *= 0.5f;

    // Apply feedback step
    qDot.w -= beta * s0;
    qDot.x -= beta * s1;
    qDot.y -= beta * s2;
    qDot.z -= beta * s3;

    // Integrate rate of change of quaternion to yield quaternion
    q += qDot * delta_t;

    // Normalise quaternion
    recipNorm = invSqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q *= recipNorm;

    return q;
}

glm::quat MadgwickModified(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, glm::quat h, float delta_t, float beta)
{
    //A slightly modified take on Madgwick's algorithm, uses a set Magnetic field for the Earth instead of calculating it to establish original orientation as straight into computer monitor
    float recipNorm;
    float s0, s1, s2, s3;
    float _2hx, _2hy, _2hz, _4hx, _4hy, _4hz;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2by, _2bz, _4bx, _4by, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    //convert Gyroscope readings to rad/s
    gx *= 3.14159 / 180.0;
    gy *= 3.14159 / 180.0;
    gz *= 3.14159 / 180.0;

    // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
    // add in call to MadgwickIMU() in the future
    if ((mx == 0.0f) && ((my == 0.0f) && (mz == 0.0f))) return q; //MadgwickIMU();

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if ((ax == 0.0f) && ((ay == 0.0f) && (az == 0.0f))) return q;

    // Normalise accelerometer and magnetometer measurements
    recipNorm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;

    recipNorm = invSqrt(mx * mx + my * my + mz * mz);
    mx *= recipNorm;
    my *= recipNorm;
    mz *= recipNorm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0mx = 2.0f * q.w * mx;
    _2q0my = 2.0f * q.w * my;
    _2q0mz = 2.0f * q.w * mz;
    _2q1mx = 2.0f * q.x * mx;
    _2q0 = 2.0f * q.w;
    _2q1 = 2.0f * q.x;
    _2q2 = 2.0f * q.y;
    _2q3 = 2.0f * q.z;
    _2q0q2 = 2.0f * q.w * q.y;
    _2q2q3 = 2.0f * q.y * q.z;
    q0q0 = q.w * q.w;
    q0q1 = q.w * q.x;
    q0q2 = q.w * q.y;
    q0q3 = q.w * q.z;
    q1q1 = q.x * q.x;
    q1q2 = q.x * q.y;
    q1q3 = q.x * q.z;
    q2q2 = q.y * q.y;
    q2q3 = q.y * q.z;
    q3q3 = q.z * q.z;

    //Calculate the reference direction of Earth's magnetic field, b
    //h = known hx, hy, hz

    Normalize(h);
    _2hx = 2 * h.x;
    _2hy = 2 * h.y;
    _2hz = 2 * h.z;
    _4hx = 2 * _2hx;
    _4hy = 2 * _2hy;
    _4hz = 2 * _2hz;

    /*
    float F[6][1];
    F[0][0] = 2 * (q.x * q.z - q.w * q.y) - ax; //2*(q(2)*q(4) - q(1)*q(3)) - Accelerometer(1)
    F[1][0] = 2 * (q.w * q.x + q.y * q.z) - ay; //2*(q(1)*q(2) + q(3)*q(4)) - Accelerometer(2)
    F[2][0] = 2 * (0.5 - q.x * q.x - q.y * q.y) - az; //2*(0.5 - q(2)^2 - q(3)^2) - Accelerometer(3)
    F[3][0] = _2hx * (0.5 - q.y * q.y - q.z * q.z) + _2hy * (q.w * q.z + q.x * q.y) + _2hz * (q.x * q.z - q.w * q.y) - mx; //2*b(2)*(0.5 - q(3)^2 - q(4)^2) + 2*b(4)*(q(2)*q(4) - q(1)*q(3)) - Magnetometer(1)
    F[4][0] = _2hx * (q.x * q.y - q.w * q.z) + _2hy * (0.5 - q.x * q.x - q.z * q.z) + _2hz * (q.w * q.x + q.y * q.z) - my; //2*b(2)*(q(2)*q(3) - q(1)*q(4)) + 2*b(4)*(q(1)*q(2) + q(3)*q(4)) - Magnetometer(2)
    F[5][0] = _2hx * (q.w * q.y + q.x * q.z) + _2hy * (q.y * q.z - q.w * q.x) + _2hz * (0.5 - q.x * q.x - q.y * q.y) - mz; //2*b(2)*(q(1)*q(3) + q(2)*q(4)) + 2*b(4)*(0.5 - q(2)^2 - q(3)^2) - Magnetometer(3)]

    float J[6][4];
    J[0][0] = -2 * q.y; J[0][1] = 2 * q.z; J[0][2] = -2 * q.w; J[0][3] = 2 * q.x;
    J[1][0] = 2 * q.x;  J[1][1] = 2 * q.w; J[1][2] = -2 * q.z; J[1][3] = 2 * q.y;
    J[2][0] = 0;        J[2][1] = -4 * q.x; J[2][2] = -4 * q.y; J[2][3] = 0;
    J[3][0] = _2hy * q.z - _2hz * q.y; J[3][1] = _2hy * q.y + _2hz * q.z; J[3][2] = -_4hx * q.y + _2hy * q.x - _2hz * q.w; J[3][3] = -_4hx * q.z + _2hy * q.w + _2hz * q.x;
    J[4][0] = -_2hx * q.z + _2hz * q.x; J[4][1] = _2hx * q.y - _4hy * q.x + _2hz * q.w; J[4][2] = _2hx * q.x + _2hz * q.z; J[4][3] = -_2hx * q.w - _4hy * q.z + _2hz * q.y;
    J[5][0] = _2hx * q.y - _2hy * q.x; J[5][1] = _2hx * q.z - _2hy * q.w - _4hz * q.x; J[5][2] = _2hx * q.w + _2hy * q.z - _4hz * q.y; J[5][3] = _2hx * q.x + _2hy * q.y;
    */

    //s0 = (J[0][0] * F[0][0]) + (J[1][0] * F[1][0]) + (J[2][0] * F[2][0]) + (J[3][0] * F[3][0]) + (J[4][0] * F[4][0]) + (J[5][0] * F[5][0])
    //s1 = (J[0][1] * F[0][0]) + (J[1][1] * F[1][0]) + (J[2][1] * F[2][0]) + (J[3][1] * F[3][0]) + (J[4][1] * F[4][0]) + (J[5][1] * F[5][0])
    //s2 = (J[0][2] * F[0][0]) + (J[1][2] * F[1][0]) + (J[2][2] * F[2][0]) + (J[3][2] * F[3][0]) + (J[4][2] * F[4][0]) + (J[5][2] * F[5][0])
    //s3 = (J[0][3] * F[0][0]) + (J[1][3] * F[1][0]) + (J[2][3] * F[2][0]) + (J[3][3] * F[3][0]) + (J[4][3] * F[4][0]) + (J[5][3] * F[5][0])

    // Gradient decent algorithm corrective step
    s0 = ((-2 * q.y) * (2 * (q.x * q.z - q.w * q.y) - ax)) + ((2 * q.x) * (2 * (q.w * q.x + q.y * q.z) - ay)) + (0 * (2 * (0.5 - q.x * q.x - q.y * q.y) - az)) + ((_2hy * q.z - _2hz * q.y) * (_2hx * (0.5 - q.y * q.y - q.z * q.z) + _2hy * (q.w * q.z + q.x * q.y) + _2hz * (q.x * q.z - q.w * q.y) - mx)) + ((-_2hx * q.z + _2hz * q.x) * (_2hx * (q.x * q.y - q.w * q.z) + _2hy * (0.5 - q.x * q.x - q.z * q.z) + _2hz * (q.w * q.x + q.y * q.z) - my)) + ((_2hx * q.y - _2hy * q.x) * (_2hx * (q.w * q.y + q.x * q.z) + _2hy * (q.y * q.z - q.w * q.x) + _2hz * (0.5 - q.x * q.x - q.y * q.y) - mz));
    s1 = ((2 * q.z) * (2 * (q.x * q.z - q.w * q.y) - ax)) + ((2 * q.w) * (2 * (q.w * q.x + q.y * q.z) - ay)) + ((-4 * q.x) * (2 * (0.5 - q.x * q.x - q.y * q.y) - az)) + ((_2hy * q.y + _2hz * q.z) * (_2hx * (0.5 - q.y * q.y - q.z * q.z) + _2hy * (q.w * q.z + q.x * q.y) + _2hz * (q.x * q.z - q.w * q.y) - mx)) + ((_2hx * q.y - _4hy * q.x + _2hz * q.w) * (_2hx * (q.x * q.y - q.w * q.z) + _2hy * (0.5 - q.x * q.x - q.z * q.z) + _2hz * (q.w * q.x + q.y * q.z) - my)) + ((_2hx * q.z - _2hy * q.w - _4hz * q.x) * (_2hx * (q.w * q.y + q.x * q.z) + _2hy * (q.y * q.z - q.w * q.x) + _2hz * (0.5 - q.x * q.x - q.y * q.y) - mz));
    s2 = ((-2 * q.w) * (2 * (q.x * q.z - q.w * q.y) - ax)) + ((-2 * q.z) * (2 * (q.w * q.x + q.y * q.z) - ay)) + ((-4 * q.y) * (2 * (0.5 - q.x * q.x - q.y * q.y) - az)) + ((-_4hx * q.y + _2hy * q.x - _2hz * q.w) * (_2hx * (0.5 - q.y * q.y - q.z * q.z) + _2hy * (q.w * q.z + q.x * q.y) + _2hz * (q.x * q.z - q.w * q.y) - mx)) + ((_2hx * q.x + _2hz * q.z) * (_2hx * (q.x * q.y - q.w * q.z) + _2hy * (0.5 - q.x * q.x - q.z * q.z) + _2hz * (q.w * q.x + q.y * q.z) - my)) + ((_2hx * q.w + _2hy * q.z - _4hz * q.y) * (_2hx * (q.w * q.y + q.x * q.z) + _2hy * (q.y * q.z - q.w * q.x) + _2hz * (0.5 - q.x * q.x - q.y * q.y) - mz));
    s3 = ((2 * q.x) * (2 * (q.x * q.z - q.w * q.y) - ax)) + ((2 * q.y) * (2 * (q.w * q.x + q.y * q.z) - ay)) + (0 * (2 * (0.5 - q.x * q.x - q.y * q.y) - az)) + ((-_4hx * q.z + _2hy * q.w + _2hz * q.x) * (_2hx * (0.5 - q.y * q.y - q.z * q.z) + _2hy * (q.w * q.z + q.x * q.y) + _2hz * (q.x * q.z - q.w * q.y) - mx)) + ((-_2hx * q.w - _4hy * q.z + _2hz * q.y) * (_2hx * (q.x * q.y - q.w * q.z) + _2hy * (0.5 - q.x * q.x - q.z * q.z) + _2hz * (q.w * q.x + q.y * q.z) - my)) + ((_2hx * q.x + _2hy * q.y) * (_2hx * (q.w * q.y + q.x * q.z) + _2hy * (q.y * q.z - q.w * q.x) + _2hz * (0.5 - q.x * q.x - q.y * q.y) - mz));

    recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
    s0 *= recipNorm;
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;

    // Rate of change of quaternion from gyroscope
    glm::quat qDot = QuaternionMultiply(q, { 0, gx, gy, gz });
    qDot *= 0.5f;

    // Apply feedback step
    qDot.w -= beta * s0;
    qDot.x -= beta * s1;
    qDot.y -= beta * s2;
    qDot.z -= beta * s3;

    // Integrate rate of change of quaternion to yield quaternion
    q += qDot * delta_t;

    // Normalise quaternion
    recipNorm = invSqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q *= recipNorm;

    return q;
}

glm::quat MadgwickIMU(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float delta_t, float beta)
{
    float recipNorm;
    float s0, s1, s2, s3;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

    //convert Gyroscope readings to rad/s
    gx *= 3.14159 / 180.0;
    gy *= 3.14159 / 180.0;
    gz *= 3.14159 / 180.0;

    // Rate of change of quaternion from gyroscope
    glm::quat qDot = QuaternionMultiply(q, { 0, gx, gy, gz });
    qDot *= 0.5f;

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if ((ax == 0.0f) && ((ay == 0.0f) && (az == 0.0f))) return q;

    // Normalise accelerometer measurement
    recipNorm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= recipNorm;
    ay *= recipNorm;
    az *= recipNorm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0 = 2.0f * q.w;
    _2q1 = 2.0f * q.x;
    _2q2 = 2.0f * q.y;
    _2q3 = 2.0f * q.z;
    _4q0 = 4.0f * q.w;
    _4q1 = 4.0f * q.x;
    _4q2 = 4.0f * q.y;
    _8q1 = 8.0f * q.x;
    _8q2 = 8.0f * q.y;
    q0q0 = q.w * q.w;
    q1q1 = q.x * q.x;
    q2q2 = q.y * q.y;
    q3q3 = q.z * q.z;

    // Gradient decent algorithm corrective step
    s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
    s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q.x - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
    s2 = 4.0f * q0q0 * q.y + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
    s3 = 4.0f * q1q1 * q.z - _2q1 * ax + 4.0f * q2q2 * q.z - _2q2 * ay;

    // normalise step magnitude
    recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    s0 *= recipNorm;
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;

    //Apply feedback step
    qDot.w -= beta * s0;
    qDot.x -= beta * s1;
    qDot.y -= beta * s2;
    qDot.z -= beta * s3;

    // Integrate rate of change of quaternion to yield quaternion
    q += qDot * delta_t;

    // Normalise quaternion
    recipNorm = invSqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q *= recipNorm;

    return q;
}

glm::quat Floyd(glm::quat q, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float bx, float by, float bz, float delta_t, float beta)
{
    float recipNorm;
    //float q0 = q.w, q1 = q.x, q2 = q.y, q3 = q.z;

    std::vector<float> a_base = { 0, 1, 0 };
    std::vector<float> mag_base = { bx, by, bz };
    std::vector<float> a_reading = { ax / (float)9.80665, ay / (float)9.80665, az / (float)9.80665 };
    std::vector<float> mag_reading = { mx, my, mz};

    //First, the current rotation quaternion is updated with Gyroscope information
    //convert Gyroscope readings to rad/s
    gx *= 3.14159 / 180.0;
    gy *= 3.14159 / 180.0;
    gz *= 3.14159 / 180.0;

    // Rate of change of quaternion from gyroscope
    glm::quat qDot = QuaternionMultiply(q, { 0, gx, gy, gz });
    qDot *= 0.5f;
    q += qDot * delta_t;

    // Normalise quaternion
    recipNorm = invSqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q *= recipNorm;

    //Magnetometer only correctly reads heading when it is level, need to apply current pitch and roll numbers from Accelerometer
    //to make the Magnetometer think that it's on level ground

    float pitch = atan2(a_reading[0], sqrt(a_reading[1] * a_reading[1] + a_reading[2] * a_reading[2]));
    float roll = atan2(a_reading[2], sqrt(a_reading[0] * a_reading[0] + a_reading[1] * a_reading[1]));

    if (a_reading[1] < 0)
    {
        pitch = 3.14159 - pitch;
        //roll = 3.14159 - roll;
    }

    if (pitch < 0) pitch += 2 * 3.14159;
    if (roll < 0) roll += 2 * 3.14159;

    float X = mag_reading[0] * cos(pitch) + mag_reading[2] * sin(roll) * sin(pitch) - mag_reading[1] * cos(roll) * sin(pitch);
    float Z = mag_reading[2] * cos(roll) - mag_reading[1] * sin(roll);

    float base = atan2(bx, bz);
    float heading = atan2(X, Z) - base;

    if (heading < 0) heading += 2 * 3.14159;

    //std::cout << "Pitch = " << pitch * 180 / 3.14159 << ", Roll = " << roll * 180 / 3.14159 << ", X = " << X << ", Z = " << Z << ", Heading = " << heading * 180 / 3.14159 << std::endl;

    glm::quat Qa = GetRotationQuaternion(a_base, a_reading);
    Qa.z = -Qa.z; //uninvert the z-axis
    glm::quat Qm = glm::quat(cos(heading / 2), 0, sin(heading / 2), 0);

    //Qa shouldn't have any rotation in the y-axis, create a vector pointing along the z-axis and rotate it by Qa
    //if it's pointing in the x-axis at all, rotate Qa so that it no longer is
    std::vector<float> zee = { 0, 0, 1 };
    QuatRotate(Qa, zee);
    float deg = atan2(zee[0], zee[2]);
    if (deg < 0) deg += 2 * 3.14159;
    Qa = QuaternionMultiply({ cos(-deg / 2), 0, sin(-deg / 2), 0 }, Qa);

    //Now add rotation in the y-axis from the magnetometer reading
    Qa = QuaternionMultiply(Qm, Qa);

    //Make sure that the two quaternions are "close" to eachother before combining them, otherwise there will be an unintended 180 degree rotation
    if (DotProduct({ Qa.w, Qa.x, Qa.y, Qa.z }, { q.w, q.x, q.y, q.z }) < 0)  Qa *= -1;

    float gamma = .0035; //represents what percentage of the rotation quaternion comes from acc. + mag. data
    q = (1 - gamma) * q + gamma * Qa;

    // Normalise rotation quaternion quaternion
    recipNorm = invSqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q *= recipNorm;

    //Need to set q0-q4 equal to weighted quaternion for next time algorithm is run
    /*
    q0 = q.w;
    q1 = q.x;
    q2 = q.y;
    q3 = q.z;
    */
}

float invSqrt(float x)
{
    //close-to-optimal method with low cost from http://pizer.wordpress.com/2008/10/12/fast-inverse-square-root
    unsigned int i = 0x5F1F1412 - (*(unsigned int*)&x >> 1);
    float tmp = *(float*)&i;
    return tmp * (1.69000231f - 0.714158168f * x * tmp * tmp);

}

/*
//Original working Madgwick code

void BLEDevice::Madgwick()
{
    if (!data_available) return;
    float recipNorm;
    float s0, s1, s2, s3;
    float hx, hy, hz;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2by, _2bz, _4bx, _4by, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
    float q0 = Quaternion.w, q1 = Quaternion.x, q2 = Quaternion.y, q3 = Quaternion.z;

    //set up current time information
    last_time_stamp = time_stamp;
    time_stamp += 1000.0 / sampleFreq; //only add to time if new data is available

    //convert Gyroscope readings to rad/s
    gx_c = gx[current_sample] * 3.14159 / 180.0;
    gy_c = gy[current_sample] * 3.14159 / 180.0;
    gz_c = gz[current_sample] * 3.14159 / 180.0;

    //convert Accelerometer readings to g, also create new variables
    ax_c = ax[current_sample];
    ay_c = ay[current_sample];
    az_c = az[current_sample];

    mx_c = mx[current_sample];
    my_c = my[current_sample];
    mz_c = mz[current_sample];

    // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
    if ((mx_c == 0.0f) && ((my_c == 0.0f) && (mz_c == 0.0f))) return;

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if ((ax_c == 0.0f) && ((ay_c == 0.0f) && (az_c == 0.0f))) return;

    // Normalise accelerometer measurement
    recipNorm = invSqrt(ax_c * ax_c + ay_c * ay_c + az_c * az_c);
    ax_c *= recipNorm;
    ay_c *= recipNorm;
    az_c *= recipNorm;

    // Normalise magnetometer measurement
    recipNorm = invSqrt(mx_c * mx_c + my_c * my_c + mz_c * mz_c);
    mx_c *= recipNorm;
    my_c *= recipNorm;
    mz_c *= recipNorm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0mx = 2.0f * q0 * mx_c;
    _2q0my = 2.0f * q0 * my_c;
    _2q0mz = 2.0f * q0 * mz_c;
    _2q1mx = 2.0f * q1 * mx_c;
    _2q0 = 2.0f * q0;
    _2q1 = 2.0f * q1;
    _2q2 = 2.0f * q2;
    _2q3 = 2.0f * q3;
    _2q0q2 = 2.0f * q0 * q2;
    _2q2q3 = 2.0f * q2 * q3;
    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q0q3 = q0 * q3;
    q1q1 = q1 * q1;
    q1q2 = q1 * q2;
    q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;

    hx = mx_c * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx_c * q1q1 + _2q1 * my_c * q2 + _2q1 * mz_c * q3 - mx_c * q2q2 - mx_c * q3q3;
    hy = _2q0mx * q3 + my_c * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my_c * q1q1 + my_c * q2q2 + _2q2 * mz_c * q3 - my_c * q3q3;
    _2bx = sqrt(hx * hx + hy * hy);
    _2bz = -_2q0mx * q2 + _2q0my * q1 + mz_c * q0q0 + _2q1mx * q3 - mz_c * q1q1 + _2q2 * my_c * q3 - mz_c * q2q2 + mz_c * q3q3;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;

    // Gradient decent algorithm corrective step
    s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay_c) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx_c) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my_c) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
    s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay_c) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az_c) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx_c) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
    s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay_c) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az_c) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx_c) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
    s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay_c) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx_c) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my_c) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);

    recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
    s0 *= recipNorm;
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;

    // Rate of change of quaternion from gyroscope
    glm::quat qDot = QuaternionMultiply({ q0, q1, q2, q3 }, { 0, gx_c, gy_c, gz_c });
    qDot *= 0.5f;

    // Apply feedback step
    qDot.w -= beta * s0;
    qDot.x -= beta * s1;
    qDot.y -= beta * s2;
    qDot.z -= beta * s3;

    // Integrate rate of change of quaternion to yield quaternion
    float delta_t = (float)((time_stamp - last_time_stamp) / 1000.0);

    q0 += qDot.w * delta_t;
    q1 += qDot.x * delta_t;
    q2 += qDot.y * delta_t;
    q3 += qDot.z * delta_t;

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    Quaternion.w = q0; Quaternion.x = q1; Quaternion.y = q2; Quaternion.z = q3;

    current_sample++; //if the end of current data set has been reached and there's no new data, keep applying filter on last piece of data

    if (current_sample >= number_of_samples)
    {
        data_available = false; //there is no more new data to look at, setting this variable to false would prevent Madgwick filter from running
        current_sample--;
    }
}
*/
