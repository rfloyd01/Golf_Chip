#include "pch.h"

#include <iostream>
#include <ppltasks.h> //library for creating parallel asynchronus tasks
#include <pplawait.h> //library for using the co_await command
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <bitset>
#include "BluetoothLE.h"
#include <chrono>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

auto serviceUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x180C);
auto characteristicUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x2A56);

int main()
{
    init_apartment();
    BLEDevice BLE_Nano(serviceUUID);
    BLE_Nano.Connect();

    int a;
    std::cin >> a;

    return 0;
}
