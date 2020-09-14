#pragma once

#include <ppltasks.h> //library for creating parallel asynchronus tasks
#include <pplawait.h> //library for using the co_await command
#include <chrono>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

class BLEDevice
{
public:
	BLEDevice(guid ServiceUUID);

	void DisplayLongUUID(guid yo);
	void Connect();

private:
	float ConvertInt32toFloat(int32_t num);
	float accl_conversion = 9.81 * 4.0 / 32768.0;
	float gyr_conversion = 2000.0 / 32768.0;
	float mag_conversion = 4.0 * 100.0 / 32768.0;

	void SetUpDeviceWatcher();
	concurrency::task<void> connectToBLEDevice(unsigned long long bluetoothAddress);

	guid service_UUID, characteristic_UUID;
	unsigned long long address;
	float time_average = 0;
	int cycle_count = 0;

	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);
	std::chrono::steady_clock::time_point timer;

	int data_counter = 0;

	Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher bleAdvertisementsWatcher;
	//Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic;
};
