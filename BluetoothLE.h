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

	void SetUpDeviceWatcher();
	concurrency::task<void> connectToBLEDevice(unsigned long long bluetoothAddress);

	guid service_UUID, characteristic_UUID;
	unsigned long long address;

	std::wstring formatBluetoothAddress(unsigned long long BluetoothAddress);
	std::chrono::steady_clock::time_point timer;

	int data_counter = 0;

	Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher bleAdvertisementsWatcher;
	//Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic;
};
