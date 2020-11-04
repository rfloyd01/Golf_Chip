#pragma once

#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include <Header_Files/gnuplot.h>

void Test1();
void Test2();
void Test3();
void Test4();
void TestGraph();
void MakeFile(std::vector<float>& ax, std::vector<float>& ay, std::vector<float>& az, std::vector<float>& gx, std::vector<float>& gy, std::vector<float>& gz, std::vector<float>& mx, std::vector<float>& my, std::vector<float>& mz, std::vector<float>& t);

void Test1()
{
	//simulate the sensor sitting float on the table for 5 seconds while facing north

	std::vector<float> ax, ay, az;
	std::vector<float> gx, gy, gz;
	std::vector<float> mx, my, mz;
	std::vector<float> time;

	float t = 0;
	while (t <= 5.1)
	{
		//go to t = 5.1 to ensure floating point errors don't prevent loop from reaching 5
		time.push_back(t);

		ax.push_back(0.001); //acceleration values of zero will cause Madgwick filter to not run so just pick a small number
		ay.push_back(0.001);
		az.push_back(1);

		gx.push_back(0);
		gy.push_back(0);
		gz.push_back(0); //gyroscope value needs to be in radians, corresponds to counter clockwise spin of 360 deg / 8 seconds

		mx.push_back(21);
		my.push_back(0);
		mz.push_back(-42);

		t += .0025;
	}

	MakeFile(ax, ay, az, gx, gy, gz, mx, my, mz, time);
	TestGraph();

	//Test went as thought, image just shows sensor sitting there, no movement at all
}

void Test2()
{
	//simulate the sensor sitting float on the table for 10 seconds while facing south

	std::vector<float> ax, ay, az;
	std::vector<float> gx, gy, gz;
	std::vector<float> mx, my, mz;
	std::vector<float> time;

	float t = 0;
	while (t <= 10.1)
	{
		//go to t = 10.1 to ensure floating point errors don't prevent loop from reaching 10
		time.push_back(t);

		ax.push_back(0.001); //acceleration values of zero will cause Madgwick filter to not run so just pick a small number
		ay.push_back(0.001);
		az.push_back(1);

		gx.push_back(0);
		gy.push_back(0);
		gz.push_back(0); //gyroscope value needs to be in radians, corresponds to counter clockwise spin of 360 deg / 8 seconds

		mx.push_back(-21);
		my.push_back(0);
		mz.push_back(-42);

		t += .0025;
	}

	MakeFile(ax, ay, az, gx, gy, gz, mx, my, mz, time);
	TestGraph();

	//Cause sensor to tilt at an about 20 degree angle and then rotate 90ish degrees about z axis
	//However, if Madgwick filter quaternion starts out rotated by 180 degrees about z-axis (i.e. {0, 0, 0, 1} instead of {1, 0, 0, 0}) the same effect
	//as test 1 is achieved
}

void Test3()
{
	//simulate the sensor spinning flat on a table, 360 degrees over the span of 8 seconds
	//simulate a sesnor recording data at 400Hz

	std::vector<float> ax, ay, az;
	std::vector<float> gx, gy, gz;
	std::vector<float> mx, my, mz;
	std::vector<float> time;

	float t = 0;
	while (t <= 8.1)
	{
		//go to t = 8.1 to ensure floating point errors don't prevent loop from reaching 8
		time.push_back(t);

		ax.push_back(0.001); //acceleration values of zero will cause Madgwick filter to not run so just pick a small number
		ay.push_back(0.001);
		az.push_back(1);

		gx.push_back(0);
		gy.push_back(0);
		gz.push_back(3.14159 / 4); //gyroscope value needs to be in radians, corresponds to counter clockwise spin of 360 deg / 8 seconds

		mx.push_back(21 * cos(2 * 3.14159 / 8 * t));
		my.push_back(-21 * sin(2 * 3.14159 / 8 * t));
		mz.push_back(-42);

		t += .0025;
	}

	MakeFile(ax, ay, az, gx, gy, gz, mx, my, mz, time);
	TestGraph();
}

void Test4()
{
	//same as test 3 but uses more realistic noisy data

	/* initialize random seed: */
	srand(time(NULL));

	std::vector<float> ax, ay, az;
	std::vector<float> gx, gy, gz;
	std::vector<float> mx, my, mz;
	std::vector<float> time;

	float t = 0;
	while (t <= 8.1)
	{
		//go to t = 8.1 to ensure floating point errors don't prevent loop from reaching 8
		time.push_back(t);

		ax.push_back(rand() % 200 / 1000.0 - .1); //acceleration values of zero will cause Madgwick filter to not run so just pick a small number
		ay.push_back(rand() % 200 / 1000.0 - .1);
		az.push_back(1 + (rand() % 200 / 1000.0 - .1));

		gx.push_back(rand() % 100 / 1000.0 - .05);
		gy.push_back(rand() % 100 / 1000.0 - .05);
		gz.push_back((3.14159 / 4) + (rand() % 100 / 1000.0 - .05)); //gyroscope value needs to be in radians, corresponds to counter clockwise spin of 360 deg / 8 seconds

		mx.push_back((21 * cos(2 * 3.14159 / 8 * t)) + (rand() % 200 / 1000.0 - .1));
		my.push_back((-21 * sin(2 * 3.14159 / 8 * t)) + (rand() % 200 / 1000.0 - .1));
		mz.push_back((-42) + (rand() % 200 / 1000.0 - .1));

		t += .0025;
	}

	MakeFile(ax, ay, az, gx, gy, gz, mx, my, mz, time);
	TestGraph();

	//Works as expected, the sensor still rotates but has a slight shake to it
}

void TestGraph()
{
	graphFromFile("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/TestData.txt", 9);
}

void MakeFile(std::vector<float>& ax, std::vector<float>& ay, std::vector<float>& az, std::vector<float>& gx, std::vector<float>& gy, std::vector<float>& gz, std::vector<float>& mx, std::vector<float>& my, std::vector<float>& mz, std::vector<float>& t)
{
	std::ofstream myFile;
	myFile.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/TestData.txt");

	for (int i = 0; i < t.size(); i++)
	{
		myFile << t[i] << "    " << ax[i] << "    " << ay[i] << "    " << az[i] << "    " << gx[i] << "    " << gy[i] << "    " << gz[i] <<
			"    " << mx[i] << "    " << my[i] << "    " << mz[i] << '\n';
	}

	myFile.close();
}

/*
The below code was used to try and debug Madgwick filter. It shouldn't be necessary anymore but it took awhile to code it all up and implement so I want to keep it in case it's
ever needed again.

void test(std::vector<float>& vec)
{
	std::cout << "[";
	for (int i = 0; i < vec.size() - 1; i++) std::cout << vec[i] << ", ";
	std::cout << vec.back() << "]" << std::endl;
}

class MatLabMadgwick
{
public:
	MatLabMadgwick(float sp, float b);
	void Update(float* gyro, float* acc, float* mag, float t, vector<glm::quat> &investigate, ofstream& file);
	void BadUpdate(float* gyro, float* acc, float* mag, float t);
	glm::quat getCurrentQuaternion();
	float invSqrt(float x);
	float IntegrateData(float& p1, float& p2, float t);

private:
	float sample_period, beta;
	glm::quat Quaternion;
	int instability_fix = 1;
};


MatLabMadgwick::MatLabMadgwick(float sp, float b)
{
    sample_period = sp;
    beta = b;
    Quaternion = { 1, 0, 0, 0 }; //Quaternion needs to be initialized so that it's near the sensors current starting position
    //Quaternion = { 0, 0, 0, 1 };
}

void MatLabMadgwick::BadUpdate(float* gyro, float* acc, float* mag, float t)
{
    float recipNorm;
    float s0, s1, s2, s3;
    float hx, hy, hz;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2by, _2bz, _4bx, _4by, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
    float q0 = Quaternion.w, q1 = Quaternion.x, q2 = Quaternion.y, q3 = Quaternion.z;
    glm::quat qDot;

    //convert Gyroscope readings to rad/s
    float gx_c = *gyro;
    float gy_c = *(gyro + 1);
    float gz_c = *(gyro + 2);

    //convert Accelerometer readings to g, also create new variables
    float ax_c = *acc;
    float ay_c = *(acc + 1);
    float az_c = *(acc + 2);

    float mx_c = *mag;
    float my_c = *(mag + 1);
    float mz_c = *(mag + 2);

    // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
    if ((mx_c == 0.0f) && (my_c == 0.0f) && (mz_c == 0.0f)) return;

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if (!((ax_c == 0.0f) && (ay_c == 0.0f) && (az_c == 0.0f)))
    {

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

        //std::cout << "Bx = " << _2bx / 2 << ", Bz = " << _2bz / 2 << std::endl;

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
        qDot = QuaternionMultiply({ q0, q1, q2, q3 }, { 0, gx_c, gy_c, gz_c });
        qDot *= 0.5f;

        // Apply feedback step
        qDot.w -= beta * s0;
        qDot.x -= beta * s1;
        qDot.y -= beta * s2;
        qDot.z -= beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    q0 += qDot.w * t;
    q1 += qDot.x * t;
    q2 += qDot.y * t;
    q3 += qDot.z * t;

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    Quaternion.w = q0; Quaternion.x = q1; Quaternion.y = q2; Quaternion.z = q3;
    //std::cout << Quaternion.w << " " << Quaternion.x << " " << Quaternion.y << " " << Quaternion.z << std::endl;
}
void MatLabMadgwick::Update(float* gyro, float* acc, float* mag, float t, vector<glm::quat>& investigate, ofstream& file)
{
    std::vector<float> Gyroscope = { *gyro, *(gyro + 1), *(gyro + 2) }; ///stays the same each time
    std::vector<float> Accelerometer = { *acc, *(acc + 1), *(acc + 2) }; ///stays the same each time
    std::vector<float> Magnetometer = { *mag, *(mag + 1), *(mag + 2) }; ///stays the same each time
    glm::quat q = Quaternion;

    file << Gyroscope[0] << "  " << Gyroscope[1] << "  " << Gyroscope[2] << "  ";
    file << Accelerometer[0] << "  " << Accelerometer[1] << "  " << Accelerometer[2] << "  ";
    file << Magnetometer[0] << "  " << Magnetometer[1] << "  " << Magnetometer[2] << "  ";
    file << q.w << "  " << q.x << "  " << q.y << "  " << q.z << "  ";

    if (Magnitude(Accelerometer) == 0) return; //break out if there are no accelerometer values to avoid division by zero
    if (Magnitude(Magnetometer) == 0) return; //break out if there are no magnetometer values to avoid division by zero

    //Normalize accelerometer and magnetometer measurements
    Normalize(Accelerometer); ///normalizing doesn't change data between iterations
    Normalize(Magnetometer); ///normalizing doesn't change data between iterations

    //investigate.push_back(QuaternionMultiply({ 0, Magnetometer[0], Magnetometer[1], Magnetometer[2] }, { 1, 2, 3, 4 }));

    //Calculate the reference direction of Earth's magnetic field, b
    glm::quat h = QuaternionMultiply(q, QuaternionMultiply({ 0, Magnetometer[0], Magnetometer[1], Magnetometer[2] }, Conjugate(q)));
    glm::quat b = { 0, Magnitude({h.x, h.y}), 0, h.z };

    //std::cout << "Calculated magnetic field = [" << h.x << ", " << h.y << ", " << b.z << ']' << std::endl;
    //std::cout << "Earth's magnetic field = [" << b.x << ", 0, " << b.z << ']' << std::endl << std::endl;

    file << h.w << "  " << h.x << "  " << h.y << "  " << h.z << "  ";
    file << b.w << "  " << b.x << "  " << b.y << "  " << b.z << "  ";
    //std::cout << b.w << "  " << b.x << "  " << b.y << "  " << b.z << std::endl;

    //Gradient Descent Algorithm corrective step
    //q1, q2, q3, q4 == q.w, q.x, q.y, q.z

    float F[6][1];
    F[0][0] = 2 * (q.x * q.z - q.w * q.y) - Accelerometer[0]; //2*(q(2)*q(4) - q(1)*q(3)) - Accelerometer(1)
    F[1][0] = 2 * (q.w * q.x + q.y * q.z) - Accelerometer[1]; //2*(q(1)*q(2) + q(3)*q(4)) - Accelerometer(2)
    F[2][0] = 2 * (0.5 - q.x * q.x - q.y * q.y) - Accelerometer[2]; //2*(0.5 - q(2)^2 - q(3)^2) - Accelerometer(3)
    F[3][0] = 2 * b.x * (0.5 - q.y * q.y - q.z * q.z) + 2 * b.z * (q.x * q.z - q.w * q.y) - Magnetometer[0]; //2*b(2)*(0.5 - q(3)^2 - q(4)^2) + 2*b(4)*(q(2)*q(4) - q(1)*q(3)) - Magnetometer(1)
    F[4][0] = 2 * b.x * (q.x * q.y - q.w * q.z) + 2 * b.z * (q.w * q.x + q.y * q.z) - Magnetometer[1]; //2*b(2)*(q(2)*q(3) - q(1)*q(4)) + 2*b(4)*(q(1)*q(2) + q(3)*q(4)) - Magnetometer(2)
    F[5][0] = 2 * b.x * (q.w * q.y + q.x * q.z) + 2 * b.z * (0.5 - q.x * q.x - q.y * q.y) - Magnetometer[2]; //2*b(2)*(q(1)*q(3) + q(2)*q(4)) + 2*b(4)*(0.5 - q(2)^2 - q(3)^2) - Magnetometer(3)]

    file << F[0][0] << "  " << F[1][0] << "  " << F[2][0] << "  " << F[3][0] << "  " << F[4][0] << "  " << F[5][0] << "  ";
    //std::cout << F[0][0] << "  " << F[1][0] << "  " << F[2][0] << "  " << F[3][0] << "  " << F[4][0] << "  " << F[5][0] << std::endl;

    float J[6][4];
    J[0][0] = -2 * q.y; J[0][1] = 2 * q.z; J[0][2] = -2 * q.w; J[0][3] = 2 * q.x;
    J[1][0] = 2 * q.x;  J[1][1] = 2 * q.w; J[1][2] = -2 * q.z; J[1][3] = 2 * q.y;
    J[2][0] = 0;        J[2][1] = -4 * q.x; J[2][2] = -4 * q.y; J[2][3] = 0;
    J[3][0] = -2 * b.z * q.y; J[3][1] = 2 * b.z * q.z; J[3][2] = -4 * b.x * q.y - 2 * b.z * q.w; J[3][3] = -4 * b.x * q.z + 2 * b.z * q.x;
    J[4][0] = -2 * b.x * q.z + 2 * b.z * q.x; J[4][1] = 2 * b.x * q.y + 2 * b.z * q.w; J[4][2] = 2 * b.x * q.x + 2 * b.z * q.z; J[4][3] = -2 * b.x * q.w + 2 * b.z * q.y;
    J[5][0] = 2 * b.x * q.y; J[5][1] = 2 * b.x * q.z - 4 * b.z * q.x; J[5][2] = 2 * b.x * q.w - 4 * b.z * q.y; J[5][3] = 2 * b.x * q.x;

    //std::cout << J[0][0] << "  " << J[0][1] << "  " << J[0][2] << "  " << J[0][3] << std::endl;
    //std::cout << J[1][0] << "  " << J[1][1] << "  " << J[1][2] << "  " << J[1][3] << std::endl;
    //std::cout << J[2][0] << "  " << J[2][1] << "  " << J[2][2] << "  " << J[2][3] << std::endl;
    //std::cout << J[3][0] << "  " << J[3][1] << "  " << J[3][2] << "  " << J[3][3] << std::endl;
    //std::cout << J[4][0] << "  " << J[4][1] << "  " << J[4][2] << "  " << J[4][3] << std::endl;
    //std::cout << J[5][0] << "  " << J[5][1] << "  " << J[5][2] << "  " << J[5][3] << std::endl << std::endl;

    file << J[0][0] << "  " << J[0][1] << "  " << J[0][2] << "  " << J[0][3] << "  ";
    file << J[1][0] << "  " << J[1][1] << "  " << J[1][2] << "  " << J[1][3] << "  ";
    file << J[2][0] << "  " << J[2][1] << "  " << J[2][2] << "  " << J[2][3] << "  ";
    file << J[3][0] << "  " << J[3][1] << "  " << J[3][2] << "  " << J[3][3] << "  ";
    file << J[4][0] << "  " << J[4][1] << "  " << J[4][2] << "  " << J[4][3] << "  ";
    file << J[5][0] << "  " << J[5][1] << "  " << J[5][2] << "  " << J[5][3] << "  ";

    //Transpose the J matrix
    float J_prime[4][6] =
    {
        {J[0][0], J[1][0], J[2][0], J[3][0], J[4][0], J[5][0]},
        {J[0][1], J[1][1], J[2][1], J[3][1], J[4][1], J[5][1]},
        {J[0][2], J[1][2], J[2][2], J[3][2], J[4][2], J[5][2]},
        {J[0][3], J[1][3], J[2][3], J[3][3], J[4][3], J[5][3]}
    };

    //calculate step magnitude
    float step[4][1];
    matrixMultiply(&J_prime[0][0], 4, 6, &F[0][0], 6, 1, &step[0][0]);
    //std::cout << J_prime[0][0] << ", " << J_prime[0][1] << ", " << J_prime[0][2] << ", " << J_prime[0][3] << ", " << J_prime[0][4] << ", " << J_prime[0][5] << std::endl;

    file << step[0][0] << "  " << step[1][0] << "  " << step[2][0] << "  " << step[3][0] << "  ";
    //std::cout << step[0][0] << " " << step[1][0] << " " << step[2][0] << " " << step[3][0] << std::endl;

    //convert step magnitude to a quaternion and then normalize it
    glm::quat q_step = { step[0][0], step[1][0], step[2][0], step[3][0] };
    Normalize(q_step);
    file << q_step.w << "  " << q_step.x << "  " << q_step.y << "  " << q_step.z << "  ";

    //compute rate of change of quaternion
    ///For some reason quaternion multiply is sometimes giving different values here when the inputs are identical, need to look into this
    glm::quat qDot = QuaternionMultiply(q, { 0, Gyroscope[0], Gyroscope[1], Gyroscope[2] });


    std::cout << "Input conditions:" << std::endl;
    std::cout << "           q = "; print(q);
    std::cout << "         gyr = " << '[' << Gyroscope[0] << "  " << Gyroscope[1] << "  " << Gyroscope[2] << ']' << std::endl;
    std::cout << "Output conditions:" << std::endl;
    std::cout << "        qDot = "; print(qDot); std::cout << std::endl;

file << Gyroscope[0] << "  " << Gyroscope[1] << "  " << Gyroscope[2] << "  " << 1 << "  ";
qDot *= .5;
qDot -= beta * q_step;
file << qDot.w << "  " << qDot.x << "  " << qDot.y << "  " << qDot.z << "  ";
//std::cout << qDot.w << " " << qDot.x << " " << qDot.y << " " << qDot.z << std::endl;

//Integrate to yield quaternion
q += qDot * t;
file << q.w << "  " << q.x << "  " << q.y << "  " << q.z << "  ";

//normalise quaternion
Normalize(q);
Quaternion = q;
file << Quaternion.w << "  " << Quaternion.x << "  " << Quaternion.y << "  " << Quaternion.z << "  ";

//std::cout << Accelerometer[0] << "  " << Accelerometer[1] << "  " << Accelerometer[2] << std::endl;
//std::cout << Quaternion.w << " " << Quaternion.x << " " << Quaternion.y << " " << Quaternion.z << std::endl << std::endl;;
}

glm::quat MatLabMadgwick::getCurrentQuaternion()
{
    return Quaternion;
}

float MatLabMadgwick::invSqrt(float x)
{
    if (instability_fix == 0)
    {
        //Original code
        float halfx = 0.5f * x;
        float y = x;
        long i = *(long*)&y;
        i = 0x5f3759df - (i >> 1);
        y = *(float*)&i;
        y = y * (1.5f - (halfx * y * y));
        return y;
    }
    else if (instability_fix == 1)
    {
        //close-to-optimal  method with low cost from http://pizer.wordpress.com/2008/10/12/fast-inverse-square-root
        unsigned int i = 0x5F1F1412 - (*(unsigned int*)&x >> 1);
        float tmp = *(float*)&i;
        return tmp * (1.69000231f - 0.714158168f * x * tmp * tmp);
    }
    else
    {
        // optimal but expensive method:
        return 1.0f / sqrtf(x);
    }
}

float MatLabMadgwick::IntegrateData(float& p1, float& p2, float t)
{
    return t * ((p1 + p2) / 2);
}

//first plot the matlab data
    //graphFromFile("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/MatlabData.txt", 9);

//consider running algorithm with high beta factor just until starting quaternion is fixed, then reduce to much smaller number
    MatLabMadgwick maggy(1.0 / 256, .1); //increasing the beta value or 1 / sample_rate causes the algorithm to converge quicker
    float g[3] = { 0, 0, 0 };
    float a[3] = { 0, 0, 0 };
    float m[3] = { 0, 0, 0 };
    std::vector<float> time;
    std::vector<glm::quat> quats;
    std::vector<glm::quat> inv;

    std::vector<float> dat = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::vector<std::vector<float> > test_gyro = { {}, {}, {} };
    std::vector<std::vector<float> > test_acc = { {}, {}, {} };
    std::vector<std::vector<float> > test_mag = { {}, {}, {} };
    std::vector<float> test_times;

    ifstream file;
    ofstream comp;
    file.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/TestData.txt");
    comp.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/Compare2.txt");
    /*
    float taytay = 0;

    time.push_back(-1.0 / 400);
    while (dat[0] < 8)
    {
        for (int j = 0; j < 10; j++)
        {
            file >> dat[j];
        }
        taytay = time.back();
        time.push_back(dat[0]);
        a[0] = dat[1]; a[1] = dat[2]; a[2] = dat[3];
        g[0] = dat[4]; g[1] = dat[5]; g[2] = dat[6];
        m[0] = dat[7]; m[1] = dat[8]; m[2] = dat[9];

        test_times.push_back(dat[0]);
        test_gyro[0].push_back(g[0]); test_gyro[1].push_back(g[1]); test_gyro[2].push_back(g[2]);
        test_acc[0].push_back(a[0]); test_acc[1].push_back(a[1]); test_acc[2].push_back(a[2]);
        test_mag[0].push_back(m[0]); test_mag[1].push_back(m[1]); test_mag[2].push_back(m[2]);

        //maggy.Update(&g[0], &a[0], &m[0], time.back() - taytay, inv, comp);
        maggy.BadUpdate(&g[0], &a[0], &m[0], time.back() - taytay);
        quats.push_back(maggy.getCurrentQuaternion());
    }
    time.erase(time.begin());
    file.close();
    comp.close();


    ifstream comp1, comp2;
    float c1, c2;
    comp1.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/Compare1.txt");
    comp2.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/Compare2.txt");

    for (int i = 0; i < 1000; i++)
    {
        comp1 >> c1;
        comp2 >> c2;

        if (c1 != c2)
        {
            std::cout << i % 75 + 1 << std::endl;
            std::cout << c1 << ", " << c2 << std::endl;
            break;
        }
    }

    ofstream file1;
    file1.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/QuaternionData.txt");
    for (int i = 0; i < quats.size(); i++)
    {
        file1 << time[i] << "    " << quats[i].w << "    " << quats[i].x << "    " << quats[i].y << "    " << quats[i].z << "\n";
    }
    file1.close();

std::chrono::time_point<std::chrono::system_clock> timer = std::chrono::system_clock::now();
std::cout << "Data recording starts now." << std::endl;
time.push_back(-1 / 400);
//write all data to file
ofstream myFile;
myFile.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/MyData.txt");

float theta = 0;
while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - timer).count() <= .5)
{
    if (!BLE_Nano.data_available)  continue;

    BLE_Nano.Madgwick();
    float tt = time.back();
    time.push_back(BLE_Nano.time_stamp / 1000.0);
    g[0] = BLE_Nano.gx[BLE_Nano.current_sample] * 3.14159 / 180.0; g[1] = BLE_Nano.gy[BLE_Nano.current_sample] * 3.14159 / 180.0; g[2] = BLE_Nano.gz[BLE_Nano.current_sample] * 3.14159 / 180.0;
    a[0] = BLE_Nano.ax[BLE_Nano.current_sample]; a[1] = BLE_Nano.ay[BLE_Nano.current_sample]; a[2] = BLE_Nano.az[BLE_Nano.current_sample];
    m[0] = BLE_Nano.mx[BLE_Nano.current_sample]; m[1] = BLE_Nano.my[BLE_Nano.current_sample]; m[2] = BLE_Nano.mz[BLE_Nano.current_sample];

    myFile << time.back() << "    " << g[0] * 180.0 / 3.14159 << "    " << g[1] * 180.0 / 3.14159 << "    " << g[2] * 180.0 / 3.14159 << "    " << a[0] << "    " << a[1] << "    " << a[2] <<
        "    " << m[0] / 100.0 << "    " << m[1] / 100.0 << "    " << m[2] / 100.0 << '\n';

    test_times.push_back(dat[0]);
    test_gyro[0].push_back(g[0]); test_gyro[1].push_back(g[1]); test_gyro[2].push_back(g[2]);
    test_acc[0].push_back(a[0]); test_acc[1].push_back(a[1]); test_acc[2].push_back(a[2]);
    test_mag[0].push_back(m[0]); test_mag[1].push_back(m[1]); test_mag[2].push_back(m[2]);

    maggy.Update(&g[0], &a[0], &m[0], time.back() - tt, inv, comp);
    quats.push_back(maggy.getCurrentQuaternion());

    std::chrono::time_point<std::chrono::system_clock> wait_timer = std::chrono::system_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - wait_timer).count() < 3) {}
}

myFile.close();


for (int i = 1; i < test_gyro[0].size(); i++)
{
    theta += maggy.IntegrateData(test_gyro[2][i], test_gyro[2][i - 1], time[i] - time[i - 1]);
}
std::cout << "Angle movement is " << theta * 180.0 / 3.14159 << " degrees." << std::endl;

//graphFromFile("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/Data_Sets/MyData.txt", 9);

GL TestWindow;
TestWindow.LoadTexture("FXOS8700.jpg");

//for (int i = 0; i < test_acc[0].size(); i++)
//{
//    std::cout << '[' << test_acc[0][i] << ", " << test_acc[1][i] << ", " << test_acc[2][i] << "] " << time[i] << std::endl;
//}
//for (int i = 0; i < inv.size(); i++) print(inv[i]);
//for (int i = 0; i < quats.size(); i++) print(quats[i]);

float tt = glfwGetTime();
float begin_timer = glfwGetTime();
float last_frame_rendered_time = 0, next_frame_rendered_time = 0;
//glfwSwapInterval(0);
for (int i = 0; i < quats.size(); i++)
{
    if (time[i] < next_frame_rendered_time) continue;
    else
    {
        //std::cout << time[i] << ", " << next_frame_rendered_time << std::endl;
    }

    //std::cout << "Rendering iteration " << i << std::endl;
    //std::cout << "Current actual time is " << glfwGetTime() - begin_timer << " seconds" << std::endl;
    //std::cout << "Current time according to i is  " << time[i] << " seconds" << std::endl;
    //print(quats[i]);
    //std::cout << std::endl;

    while (glfwGetTime() - begin_timer < time[i])
    {

    }

    TestWindow.processInput();
    TestWindow.RenderClub({ quats[i].w, quats[i].y, quats[i].z, quats[i].x }); //switch y and z axes for OpenGL rendering purposes, OpenGl seems to handle y rotation differently so flip value
    TestWindow.RenderText();
    TestWindow.Swap();

    //std::cout << "Rendered time stamp " << time[i] << " at actual time " << glfwGetTime() - begin_timer << std::endl;

    next_frame_rendered_time = glfwGetTime() - begin_timer;
    //std::cout << "Next rendered frame should be at " << next_frame_rendered_time << " seconds." << std::endl;
    tt = glfwGetTime();
}

TestWindow.DeleteBuffers();
TestWindow.Terminate();
*/