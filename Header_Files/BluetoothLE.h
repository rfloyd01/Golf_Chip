#pragma once

#include <ppltasks.h> //library for creating parallel asynchronus tasks
#include <pplawait.h> //library for using the co_await command
#include <chrono>

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
	X,
	Y,
	Z
};

class BLEDevice
{
public:
	//PUBLIC FUNCTIONS
	//Constructors
	BLEDevice(guid ServiceUUID, float freq);

	//Connection
	void Connect();

	//Data Updating
	void Update(); //master update function
	void MadgwickUpdate();
	void UpdateData();
	void UpdateCalibrationNumbers();
	void ToggleCalNumbers();
	void UpdatePosition();
	void ResetPosition();
	void resetTime();

	//Variable Setting Functions
	void SetSampleFrequency(float freq);
	void SetMagField();
	void SetPositionTimer();

	//Data Passing Functions
	//These functions are for outside classes that need to access private BLEDevice variables
	//TODO - Is it better to return the physical variables or just a reference to the variables? Figure this out
	float GetData(int index);
	std::vector<float>* GetData(DataType dt, Axis a);
	void GetLinearAcceleration();
	glm::quat GetOpenGLQuaternion();
	int getCurrentSample();

	//Other Functions
	void DisplayLongUUID(guid yo);

	//PUBLIC VARIABLES
	//Position and Orientation variables
	glm::quat Quaternion = { 1, 0, 0, 0 }; //represents the current orientation of the sensor

	//Timing variables
	float time_stamp = 0; //the time in milliseconds from when the program connected to the BLE device
	float last_time_stamp = 0; 

	//Bool variables
	//had issues with these bool variables not updating so turned them into volatile variables. Apparently it had something to do with being read by multiple threads.
	//more on the subject can be found here https://stackoverflow.com/questions/25425130/loop-doesnt-see-value-changed-by-other-thread-without-a-print-statement
	volatile bool is_connected = false;
	volatile bool data_available = false;
	
private:
	//PRIVATE FUNCTIONS
	//Connection
	void SetUpDeviceWatcher();
	concurrency::task<void> connectToBLEDevice(unsigned long long bluetoothAddress);

	//Util Functions
	float ConvertInt32toFloat(int32_t num);
	float Integrate(float one, float two, float dt);

	//Other Private Functions
	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);

	//PRIVATE VARIABLES
	//Sample Variables
	//TODO: Look into a good way to read number_of_samples from BLE device upon initialization
	const int number_of_samples = 10; //number of sensor samples stored in the BLE characteristic at a given time. Due to the time associated with reading BLE broadcasts, its more efficient to store multiple data points at a single time then try to read each individual point
	int current_sample = 0; //when updating rotation quaternion with Madgwick filter, need to know which data point is currently being looked at

	//Conversion Variables
	//TODO - Add a way to update conversion variables based on what the current sensor settings are
	float acc_conversion = 9.80665 * 0.000488; //using conversion for 4G FXOS8700
	float gyr_conversion = 0.015625; //using conversion for +/- 500deg/s FXAS21002C
	float mag_conversion = 0.1; //using standard conversion for FXOS8700

	//Sensor specific variables
	guid service_UUID, characteristic_UUID;
	unsigned long long address;

	//Variables that store sensor data
	std::vector<std::vector<float> > accelerometer = { {}, {}, {} }; //ax, ay, az; //vectors of size number_of_samples which hold current calibrated acceleration readings
	std::vector<std::vector<float> > gyroscope = { {}, {}, {} };
	std::vector<std::vector<float> > magnetometer = { {}, {}, {} };
	std::vector<std::vector<float> > linear_acceleration = { {}, {}, {} };
	std::vector<std::vector<float> > velocity = { {}, {}, {} };
	std::vector<std::vector<float> > location = { {}, {}, {} };
	//std::vector<float> gx, gy, gz; //vectors of size number_of_samples which hold current calibrated gyroscope readings
	//std::vector<float> mx, my, mz; //vectors of size number_of_samples which hold current calibrated magnetometer readings
	std::vector<float> raw_sensor_data; //holds all raw data that comes in from the sensor
	double temperature; //not currently using temperature but should consider it

	float gravity = 9.80665;
	float lin_acc_threshold = 0.025; //linear acceleration will be set to zero unless it exceeds this threshold. This will help with location drift with time. This number was obtained experimentally as most white noise falls within +/- .025 of actual readings
	float movement_scale = 1; //This number is to make sure that distance traveled looks accurate relative to how far sensor is from the camera

	int data_counter = 0;

	bool acceleration_event = 0; //when an acceleration above the threshold is detected, starts collecting velocity and position data. When acceleration has stopped in all directions, the event is set back to zero and velocity is halted
	bool just_stopped = 0;

	glm::quat g_to_m, m_to_g; //quaternion that rotates the gravity frame to magnetic frame
	glm::quat m_to_mprime = { 1, 0, 0, 0 };

	//Timing Variables
	double position_timer, end_timer;
	float time_average = 0;
	int cycle_count = 0;
	double data_timer = glfwGetTime();
	double Madgwick_timer;
	std::chrono::steady_clock::time_point timer;

	//Madgwick items
	float sampleFreq, beta = 0.1; //beta changes how reliant the Madgwick filter is on acc and mag data, good value is 0.035
	float bx, by, bz; //consider setting these values to value of current location by default

	//calibration numbers
	double acc_off[3][3];
	double acc_gain[3][3];
	double gyr_off[3];
	double gyr_gain[3];
	double mag_off[3];
	double mag_gain[3];
	bool cal_on = 1; //disabled calibration numbers so that raw data from sensor is obtained

	Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher bleAdvertisementsWatcher;
};
