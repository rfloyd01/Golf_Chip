#pragma once

#include <vector>

#include <Header_Files/graphics.h>
#include <Header_Files/BluetoothLE.h>

class GL;
class BLEDevice;

class Calibration
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	Calibration(BLEDevice* sensor);

	//Setup Functions
	void SetGraphics(GL* graph);

	//Main Loop Functions
	void CalibrationLoop();
	void CheckForNextStep(); //if sensor is currently collecting data, this function will figure out the next step based on the current test being performed

	//Utilization Functions
	void processInput();

	//Get Functions
	bool getCalMode();

	//PUBLIC VARIABLES
	//Constants
	const float gravity = 9.80665;

private:
	//PRIVATE FUNCTIONS
	//Calibration Data Functions
	void GetCurrentCalibrationNumbers();
	void UpdateCalibrationNumbers(); //this function changes the values of the cal numbers saved in this class
	void SetCalibrationNumbers();    //this function updates the text file Calibration.txt so the rest of the program will have the new cal numbers

	//Data Gathering Functions
	void addRawData(DataType dt, int current_sample); //adds raw data from sensor to necessary data vectors

	//Test Functions
	void AccTest();
	void AccNextStep();
	void GyroTest();
	void GyroNextStep();
	void MagTest();
	void MagNextStep();
	void MagGraph();

	//Utilization Functions
	float IntegrateData(float& p1, float& p2, float t);
	void invertAccMatrix();

	//PRIVATE VARIABLES
	//Pointer to other entities
	GL* p_graphics;
	BLEDevice* p_sensor;

	//Arrays to hold new calibration numbers
	float acc_off[3][3] = { 0 }; //acceleration offset values
	float acc_gain[3][3] = { 0 }; //acceleration axis and cross axis gain values
	float gyr_off[3] = { 0 };
	float gyr_gain[3] = { 0 };
	float mag_off[3] = { 0 };
	float mag_gain[3] = { 0 };

	//Vectors to hold raw uncalibrated data from sensor
	std::vector<float> ax, ay, az; //vectors to hold acceleration cal. data
	std::vector<float> gx, gy, gz; //vectors to hold gyroscope cal. data
	std::vector<float> mx, my, mz; //vectors to hold gyroscope cal. data
	std::vector<float> time_data;

	//Rendering Variables
	glm::quat calibration_q = { 1, 0, 0, 0 }; //a quaternion used to render sensor in current phase of calibration, can be calculated from sensor or manually set based on current test
	//glm::vec3 scale_matrix = { 1.0, 1.0, 1.0 }, translate_matrix = { 0.0, 0.0, 0.0 };

	//Calibration State Variables
	bool record_data = 0, mode_select = 0, collecting = 0; //bool variables that tell calibration loop about current state
	bool changes_made = 0; //if any calibration numbers were changed this is set to 1 so the code knows to update Calibration.txt
	int cal_mode = -1, cal_stage = 0, avg_count = 0; //cal_stage represents how far into each physical calibration you are

	//Other Calibration Variables
	float acc_cal[3][6] = { 0 }; //needed to isolate data from all six portions of the acc. tumble calibration: x1, x2, x3, x4, x5, x6, y1, y2... z6
	float gyr_cal[3] = { 0 }; //integrated gyro data will go in here to figure out how close to 90 degrees the sensor was rotated
	float mag_max[3] = { 0 }; //stores maximum magnetometer readings for x, y and z
	float mag_min[3] = { 0 }; //stores minimum magnetometer readings for x, y and z

	//Timing Variables
	double cal_time, data_time;
};

//magnetic data for Conshohocken, PA in Gauss
	//North/South x-component (+N | -S) =  .204531 Gauss = 20.4531 uT
	//East/West   y-component (+E | -W) = -.043260 Gauss = -4.3260 uT
	//Up/Down     z-component (+D | -U) =  .468534 Gauss = 46.8534 uT
	//Magnitude = sqrt(.204531^2 + .04326^2 + .468534^2) = 0.513058 Gauss = 51.3058 uT
