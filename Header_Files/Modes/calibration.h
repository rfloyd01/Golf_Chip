#pragma once

#include <Header_Files/Modes/mode.h>

#define gravity 9.80665

//Classes, structs and enums defined in other header files
class GL;
class Text;

//Class Definition
class Calibration : public Mode
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	Calibration(GL& graphics);

	//Updating and Advancement Functions
	void update(); //virtual allows a sub-class to overwrite the base class' implementation of the function
	void processInput();
	void modeStart();
	void modeEnd();

private:
	//PRIVATE FUNCTIONS
	//Setup Functions
	void initializeText();

	//Data Gathering Functions
	void addRawData(DataType dt, int current_sample); //adds raw data from sensor to necessary data vectors

	//Advancement Functions
	void checkForNextStep();
	void accTest();
	void accNextStep();
	void gyroTest();
	void gyroNextStep();
	void magTest();
	void magNextStep();
	void MagGraph();

	//Calibration Data Functions
	void getCurrentCalibrationNumbers();
	void updateCalibrationNumbers(); //this function changes the values of the cal numbers saved in this class
	void setCalibrationNumbers();    //this function updates the text file Calibration.txt so the rest of the program will have the new cal numbers

	//Utilization Functions
	void invertAccMatrix();
	float integrateData(float& p1, float& p2, float t);

	//PRIVATE VARIABLES
	//State Variables
	int cal_mode; //this variable keeps track of what type of calibration is occuring, 1 = accelerometer, 2 = gyroscope, 3 = magnetometer, 0 = calibration main menu
	int cal_stage; //keeps track of what stage of the calibration is currently active. This number will have different meanings for acc, gyr and mag

	//Bool Variables
	bool collecting; //this bool object is true when actively collecting data, and false if not currently collecting data from chip
	bool changes_made = 0; //tracks whether or not any calibration has taken place, if so, set to true so program knows to update calibration text file

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

	//Other Calibration Variables
	float acc_cal[3][6] = { 0 }; //needed to isolate data from all six portions of the acc. tumble calibration: x1, x2, x3, x4, x5, x6, y1, y2... z6
	float gyr_cal[3] = { 0 }; //integrated gyro data will go in here to figure out how close to 90 degrees the sensor was rotated
	float mag_max[3] = { 0 }; //stores maximum magnetometer readings for x, y and z
	float mag_min[3] = { 0 }; //stores minimum magnetometer readings for x, y and z
	int avg_count = 0; //counts the number of data points in a set and then used to average sum of all data points

	//Timing Variables
	double cal_time, data_time;
};