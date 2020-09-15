#pragma once

#include <vector>

#include "graphics.h"
#include "BluetoothLE.h"

class GL;
class BLEDevice;

class Calibration
{
public:
	Calibration(BLEDevice* sensor);
	void SetGraphics(GL* graph);
	void CalibrationLoop();

	//Accelerometer Calibration Numbers
	float acc_off[3][3] = { 0 }; //acceleration offset values
	float acc_gain[3][3] = { 0 }; //acceleration axis and cross axis gain values

	//Gyroscope Calibration Numbers
	float gyr_off[3] = { 0 };
	float gyr_gain[3] = { 0 };

	//Magnetometer Calibration Numbers
	float mag_off[3] = { 0 };
	float mag_gain[3] = { 0 };

	float gravity = 9.80665;

	void processInput();
	void CheckForNextStep(); //if sensor is currently collecting data, this function will figure out the next step based on the current test being performed

private:
	GL* p_graphics;
	BLEDevice* p_sensor;

	void GetCurrentCalibrationNumbers();
	void UpdateCalibrationNumbers(); //this function changes the values of the cal numbers saved in this class
	void SetCalibrationNumbers();    //this function updates the text file Calibration.txt so the rest of the program will have the new cal numbers
	void LiveUpdate();

	//functions for individual tests
	void AccTest();
	void AccNextStep();
	void GyroTest();
	void GyroNextStep();
	void MagTest();
	void MagNextStep();
	void MagGraph();
	float IntegrateData(float& p1, float& p2, float t);

	std::vector<float> ax, ay, az; //vectors to hold acceleration cal. data
	std::vector<float> gx, gy, gz; //vectors to hold gyroscope cal. data
	std::vector<float> mx, my, mz; //vectors to hold gyroscope cal. data
	std::vector<float> time_data;

	bool record_data = 0, can_press_key = 0, mode_select = 0, collecting = 0, live_data = 0; //can_press_key will disable keyboard presses momentarily after a key is pressed, to make sure the same key isn't pressed multiple times in one go
	bool changes_made = 0; //if any calibration numbers were changed this is set to 1 so the code knows to update Calibration.txt
	int cal_mode = 0, cal_stage = 0, text_start = 0, avg_count = 0; //cal_stage represents how far into each physical calibration you are

	float acc_cal[3][6] = { 0 }; //needed to isolate data from all six portions of the acc. tumble calibration: x1, x2, x3, x4, x5, x6, y1, y2... z6
	float gyr_cal[3] = { 0 }; //integrated gyro data will go in here to figure out how close to 90 degrees the sensor was rotated
	float mag_max[3] = { 0 };
	float mag_min[3] = { 0 };

	double cal_time, data_time;
};

//magnetic data for Conshohocken, PA in Gauss
	//North/South x-component (+N | -S) =  .204531 Gauss = 20.4531 uT
	//East/West   y-component (+E | -W) = -.043260 Gauss = -4.3260 uT
	//Up/Down     z-component (+D | -U) =  .468534 Gauss = 46.8534 uT
	//Magnitude = sqrt(.204531^2 + .04326^2 + .468534^2) = 0.513058 Gauss = 51.3058 uT
