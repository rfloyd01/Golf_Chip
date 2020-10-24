#pragma once

#include <ppltasks.h> //library for creating parallel asynchronus tasks
#include <pplawait.h> //library for using the co_await command

#include <Header_Files/calibration.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

//Some Enums that make code a little more readable
enum DataType
{
	ACCELERATION,
	ROTATION,
	MAGNETIC,
	LINEAR_ACCELERATION,
	VELOCITY,
	LOCATION
};
enum Axis
{
	//x, y, and z correlate to 0, 1 and 2 respectively. It's a little easier to keep track this way then having numbers denote everything
	X = 0,
	Y = 1,
	Z = 2
};

class BLEDevice
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	BLEDevice(guid ServiceUUID, float freq);

	//Connection
	void connect();

	//Data Updating
	void masterUpdate(); //master update function
	void updateCalibrationNumbers();
	void resetPosition(); //resets sensor lin_acc., vel. and loc. to 0 so that club will be rendered back at center of screen
	void resetTime(); //resets time_stamp variable to 0 seconds, useful when looking at graphs of data so that they start at t = 0

	//Variable Setting Functions
	void setSampleFrequency(float freq); //currently don't need but if way to manipulate chip settings via program is added this will be necessary
	void setMagField();

	//Data Passing Functions
	//These functions are for outside classes that need to access private BLEDevice variables
	//TODO - Is it better to return the physical variables or just a reference to the variables? Figure this out
	std::vector<float>* getData(DataType dt, Axis a);
	std::vector<float>* getRawData(DataType dt, Axis a);
	glm::vec3 getLocation();
	glm::quat getOpenGLQuaternion();
	int getCurrentSample();
	float getCurrentTime(); //returns the time stamp of the current data reading in seconds

	//Other Functions
	void displayLongUUID(guid yo);

	//PUBLIC VARIABLES
	//Bool variables
	//had issues with these bool variables not updating so turned them into volatile variables. Apparently it had something to do with being read by multiple threads.
	//more on the subject can be found here https://stackoverflow.com/questions/25425130/loop-doesnt-see-value-changed-by-other-thread-without-a-print-statement
	volatile bool is_connected = false;
	volatile bool data_available = false;

private:
	//PRIVATE FUNCTIONS
	//Connection
	void setUpDeviceWatcher();
	concurrency::task<void> connectToBLEDevice(unsigned long long bluetoothAddress);

	//Internal Updating Functions
	void updateSensorData();
	void updateMadgwick();
	void updateLinearAcceleration();
	void updatePosition();

	//Util Functions
	float convertInt32toFloat(int32_t num);
	float integrate(float one, float two, float dt);
	void makeZeroVector(std::vector<float>* vec, int number_of_zeros);

	//Other Private Functions
	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);

	//PRIVATE VARIABLES
	//Sample Variables
	//TODO: Look into a good way to read number_of_samples from BLE device upon initialization
	const int number_of_samples = 10; //number of sensor samples stored in the BLE characteristic at a given time. Due to the time associated with reading BLE broadcasts, its more efficient to store multiple data points at a single time then try to read each individual point
	int current_sample = 0; //when updating rotation quaternion with Madgwick filter, need to know which data point is currently being looked at

	//Conversion Variables
	//TODO - Add a way to update conversion variables based on what the current sensor settings are
	//convert binary readings from chip into actual numbers with these variables, taken from sensor mfg.
	//float acc_conversion = 9.80665 * 0.000488; //using conversion for 4G FXOS8700
	float acc_conversion = 9.80665 * 4.0 / 32768.0; //using conversion for 4G LSM9DS1
	//float gyr_conversion = 0.015625; //using conversion for +/- 500deg/s FXAS21002C
	float gyr_conversion = 2000.0 / 32768.0; //using conversion for +/- 500deg/s LSM9DS1
	//float mag_conversion = 0.1; //using standard conversion for FXOS8700
	float mag_conversion = 4.0 * 100.0 / 32768.0; //using standard conversion for LSM9DS1

	//Sensor specific variables
	guid service_UUID, characteristic_UUID;

	//Position and Orientation variables
	glm::quat Quaternion = { 1, 0, 0, 0 }; //represents the current orientation of the sensor
	std::vector<std::vector<float> > accelerometer = { {}, {}, {} }; //vectors of size number_of_samples which hold current calibrated acceleration readings
	std::vector<std::vector<float> > gyroscope = { {}, {}, {} };
	std::vector<std::vector<float> > magnetometer = { {}, {}, {} };
	std::vector<std::vector<float> > linear_acceleration = { {}, {}, {} };
	std::vector<std::vector<float> > velocity = { {}, {}, {} };
	std::vector<std::vector<float> > location = { {}, {}, {} };

	//Variables that store sensor data
	std::vector<std::vector<float> > r_accelerometer = { {}, {}, {} }; //vectors of size number_of_samples which hold current calibrated acceleration readings
	std::vector<std::vector<float> > r_gyroscope = { {}, {}, {} };
	std::vector<std::vector<float> > r_magnetometer = { {}, {}, {} };
	double temperature; //not currently using temperature but should consider it

	float gravity = 9.80665;

	//Movement variables
	float lin_acc_threshold = 0.025; //linear acceleration will be set to zero unless it exceeds this threshold. This will help with location drift with time. This number was obtained experimentally as most white noise falls within +/- .025 of actual readings
	bool acceleration_event = 0; //when an acceleration above the threshold is detected, starts collecting velocity and position data. When acceleration has stopped in all directions, the event is set back to zero and velocity is halted
	float movement_scale = 1; //This number is to make sure that distance traveled looks accurate relative to how far sensor is from the camera
	bool just_stopped = 0; //when acceleration gets low enough, this variable is used to stop velocity and location from trickling forwards

	int data_counter = 0;

	//Frame conversion quaternions
	//These quaternions are used to rotate sensor data from one from to another
	glm::quat g_to_m, m_to_g; //quaternion that rotates the gravity frame to magnetic frame
	glm::quat m_to_mprime = { 1, 0, 0, 0 }; //The purpose of this quaternion is to rotate magnet data from current reading to 'desired' reading, i.e. magnetic north into computer screen so sensor lines up with computer screen

	//Timing Variables
	double position_timer = 0, end_timer = 0; //used for tracking start and stop times of accerleation events, to known if the club should actually move or not
	float time_stamp = 0; //the time in milliseconds from when the program connected to the BLE device, can be reset to 0 when looking at graphs
	float last_time_stamp = 0; //holds the time of the last measured sample, used to find delta_t for integration purposes

	//Madgwick items
	float sampleFreq, beta = 0.1; //beta changes how reliant the Madgwick filter is on acc and mag data, good value is 0.035
	float bx = 21, by = 0, bz = -42; //consider setting these values to value of current location by default. Currently set to Conshohocken values

	//calibration numbers
	//These variables will change when a calibration is carried out and are initialized based on data in calibration.txt in Resources
	double acc_off[3][3] = { 0 };
	double acc_gain[3][3] = { 0 };
	double gyr_off[3] = { 0 };
	double gyr_gain[3] = { 0 };
	double mag_off[3] = { 0 };
	double mag_gain[3] = { 0 };

	Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher bleAdvertisementsWatcher;
};
