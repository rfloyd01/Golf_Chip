#pragma once

#include <ppltasks.h> //library for creating parallel asynchronus tasks
#include <pplawait.h> //library for using the co_await command
#include <chrono>

#include <Header_Files/calibration.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

//TODO: A lot of stuff in here was just randomly copied from a previous project
//Should consider breaking apart Madgwick items, quaternion algebra and BLE functions
//into separate files for readability

class BLEDevice
{
public:
	BLEDevice(guid ServiceUUID, float freq);

	void DisplayLongUUID(guid yo);
	void Connect();

	volatile bool is_connected = false;
	//had issues with this bool variable not updating. Apparently it had something to do with being read by multiple threads
	//more on the subject can be found here https://stackoverflow.com/questions/25425130/loop-doesnt-see-value-changed-by-other-thread-without-a-print-statement

	//functions from old Connection.h file
	void UpdateData();
	void UpdateCalibrationNumbers();
	void ToggleCalNumbers();

	//Madgwick related functions
	void Floyd();
	void Madgwick();
	void MadgwickModified();
	void MadgwickIMU();
	float invSqrt(float x);
	glm::quat GetRotationQuaternion();
	void SetSampleFrequency(float freq);
	void SetMagField(float x, float y, float z);

	//Data Passing Functions
	float GetData(int index);
	std::vector<float> GetData();
	void GetLinearAcceleration();

	void UpdatePosition();
	void ResetPosition();

	void SetPositionTimer();

	float ax, ay, az, r_ax, r_ay, r_az; //TODO: Do I need separate variables for raw data? I don't think so
	float gx, gy, gz, r_gx, r_gy, r_gz;
	float mx, my, mz, r_mx, r_my, r_mz;
	double temperature; //not currently using temperature but should consider it

	//position data
	float lin_ax = 0, lin_ay = 0, lin_az = 0;
	float vel_x = 0, vel_y = 0, vel_z = 0;
	float loc_x = 0, loc_y = 0, loc_z = 0;

private:
	float ConvertInt32toFloat(int32_t num);
	float acc_conversion = 9.80665 * 0.000488; //using conversion for 4G FXOS8700
	float gyr_conversion = 0.015625; //using conversion for +/- 500deg/s FXAS21002C
	float mag_conversion = 0.1; //using standard conversion for FXOS8700

	double data_timer = glfwGetTime();
	double connection_timer = glfwGetTime();

	void SetUpDeviceWatcher();
	concurrency::task<void> connectToBLEDevice(unsigned long long bluetoothAddress);

	guid service_UUID, characteristic_UUID;
	unsigned long long address;
	float time_average = 0;
	int cycle_count = 0;
	std::vector<float> raw_sensor_data = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);
	std::chrono::steady_clock::time_point timer;

	double position_timer, end_timer;
	float gravity = 9.80665;
	float lin_acc_threshold = 0.025; //linear acceleration will be set to zero unless it exceeds this threshold. This will help with location drift with time. This number was obtained experimentally as most white noise falls within +/- .025 of actual readings
	float movement_scale = 1; //This number is to make sure that distance traveled looks accurate relative to how far sensor is from the camera

	int data_counter = 0;

	bool acceleration_event = 0; //when an acceleration above the threshold is detected, starts collecting velocity and position data. When acceleration has stopped in all directions, the event is set back to zero and velocity is halted
	bool just_stopped = 0;

	float ax_c, ay_c, az_c; //create copy variables so Madgwick filter doesn't alter original values from sensor
	float gx_c, gy_c, gz_c; //create copy variables so Madgwick filter doesn't alter original values from sensor
	float mx_c, my_c, mz_c; //create copy variables so Madgwick filter doesn't alter original values from sensor

	glm::quat g_to_m, m_to_g; //quaternion that rotates the gravity frame to magnetic frame

	//Madgwick items
	float q0 = 1, q1 = 0, q2 = 0, q3 = 0;
	float sampleFreq, beta = 0.035; //beta changes how reliant the Madgwick filter is on acc and mag data, good value is 0.035
	glm::quat q = glm::quat(q0, q2, q3, q1), qnew = glm::quat(1, 0, 0, 0);
	int instability_fix = 1;
	float bx, by, bz; //consider setting these values to value of current location by default

	//Quaternion arithmatic functions
	float DotProduct(std::vector<float> vec1, std::vector<float> vec2);
	std::vector<float> CrossProduct(std::vector<float> vec1, std::vector<float> vec2);
	float Magnitude(std::vector<float> vec);
	void Normalize(glm::quat& q);
	void QuatRotate(glm::quat q, std::vector<float>& data);
	void QuatRotate(glm::quat q1, glm::quat& q2);
	glm::quat QuaternionMultiply(glm::quat q1, glm::quat q2);
	glm::quat GetRotationQuaternion(std::vector<float> vec1, std::vector<float> vec2);
	glm::quat GetRotationQuaternion(float angle, std::vector<float> vec);
	glm::quat Conjugate(glm::quat q);

	float Integrate(float one, float two, float dt);

	//calibration numbers
	double acc_off[3][3];
	double acc_gain[3][3];
	double gyr_off[3];
	double gyr_gain[3];
	double mag_off[3];
	double mag_gain[3];
	bool cal_on = 1; //disabled calibration numbers so that raw data from sensor is obtained

	//Calibration* p_cal;

	Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher bleAdvertisementsWatcher;
	//Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic;
};
