#include "pch.h"

#include "BluetoothLE.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

BLEDevice::BLEDevice(guid ServiceUUID)
{
	service_UUID = ServiceUUID;
}

void BLEDevice::Connect()
{
    SetUpDeviceWatcher();
}

void BLEDevice::SetUpDeviceWatcher()
{
	//This function starts a BLEDevice watcher and sets up parameters

    bleAdvertisementsWatcher = Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher();
    bleAdvertisementsWatcher.ScanningMode(Bluetooth::Advertisement::BluetoothLEScanningMode::Active);
    bleAdvertisementsWatcher.Received(Windows::Foundation::TypedEventHandler<Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher, Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs>(
        [this](Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher, Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            auto serviceUuids = eventArgs.Advertisement().ServiceUuids();
            unsigned int index = -1;

            if (serviceUuids.IndexOf(service_UUID, index))
            {
                //address = eventArgs.BluetoothAddress();
                std::wstring strAddress = formatBluetoothAddress(eventArgs.BluetoothAddress());
                std::wcout << "Target service found on device: " << strAddress << std::endl;
                std::wcout << L"Target device has " << serviceUuids.Size() << " services that it offers\n" << std::endl;
                DisplayLongUUID(serviceUuids.GetAt(0));

                bleAdvertisementsWatcher.Stop();
                auto t = connectToBLEDevice(eventArgs.BluetoothAddress());
                try
                {
                    std::cout << "Watcher has stopped, attempting to connect to device..." << std::endl;
                    t.wait();
                }
                catch (const std::exception& e)
                {
                    std::wcout << "An issue occured while connecting to BLE Device" << std::endl;
                    std::cout << e.what() << std::endl;
                }
            }
        }));

    bleAdvertisementsWatcher.Start();
    std::cout << "Device Watcher has Started! Type any key and then hit enter to stop." << std::endl;
    int a;
    std::cin >> a;
}

concurrency::task<void> BLEDevice::connectToBLEDevice(unsigned long long bluetoothAddress)
{
    auto bleDevice = co_await Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(bluetoothAddress); //this function physically connects the computer to the BLE 33 Nano
    Bluetooth::GenericAttributeProfile::GattDeviceServicesResult servicesResult = co_await bleDevice.GetGattServicesAsync();
    std::cout << "There are " << servicesResult.Services().Size() << " services that were found." << std::endl;
    for (int i = 0; i < servicesResult.Services().Size(); i++)
    {
        std::cout << std::hex << "Short UUID for service " << i + 1 << " is: " << servicesResult.Services().GetAt(i).Uuid().Data1 << std::endl;
    }

    Bluetooth::GenericAttributeProfile::GattDeviceService service = servicesResult.Services().GetAt(2);
    Bluetooth::GenericAttributeProfile::GattCharacteristicsResult characteristicResult = co_await service.GetCharacteristicsAsync();
    Bluetooth::GenericAttributeProfile::GattCharacteristic characteristic = characteristicResult.Characteristics().GetAt(0);

    std::cout << "Characteristic found! Connecting to Characteristic " << characteristic.Uuid().Data1 << std::endl;

    if ((characteristic.CharacteristicProperties() & GattCharacteristicProperties::Notify) == GattCharacteristicProperties::Notify)
    {
        std::cout << "Characteristic can be set to notify. ";

        //To set up a characteristic for notification, first the Client Characteristic Configuration Descriptor (CCCD) must be written to
        GattCommunicationStatus status = co_await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify);
        if (status == GattCommunicationStatus::Success)
        {
            timer = std::chrono::steady_clock::now(); //start the clock to measure time between readings

            //Now need to implement the characteristic value changed event handler
            std::cout << std::dec;
            characteristic.ValueChanged(Windows::Foundation::TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs>([this](GattCharacteristic car, GattValueChangedEventArgs eventArgs)
                {
                    auto reader = Windows::Storage::Streams::DataReader::FromBuffer(eventArgs.CharacteristicValue());
                    std::cout << "Time to read data was " << (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - timer).count() / 1000000.0  << " milliseconds." << std::endl;
                    timer = std::chrono::steady_clock::now(); //reset timer
                    //reader.ByteOrder(Windows::Storage::Streams::ByteOrder::BigEndian); //BLE uses little Endian which is annoying so switch it

                    for (int i = 0; i < 3; i++)
                    {
                        int32_t ayyy = reader.ReadInt32();

                        if (i == 0) std::cout << "Ax = " << ConvertInt32toFloat(ayyy) << std::endl;
                        if (i == 1) std::cout << "Ay = " << ConvertInt32toFloat(ayyy) << std::endl;
                        if (i == 2) std::cout << "Az = " << ConvertInt32toFloat(ayyy) << std::endl;
                    }
                    std::cout << std::endl;
                }));

            std::cout << "Event Handler for notifications has been set up." << std::endl;
        }
        else std::cout << "Was not able to properly set up notification event handling." << std::endl;
    }

    //the below infinite loop can have the rest of the code for whatever I want to do when the time comes
    for (;;)
    {
        auto cnnct = bleDevice.ConnectionStatus();
        if (cnnct == Bluetooth::BluetoothConnectionStatus::Connected)
        {
            //std::cout << "I'm connected!" << std::endl;
            int yoyoy = 1;
        }
        else
        {
            std::cout << "I got disconnected :(" << std::endl;
            break;
        }
    }

    std::cout << "Made it to end of connect function, now disconnecting from the BLE Device." << std::endl;
    //bleDevice.Close();
}

std::wstring BLEDevice::formatBluetoothAddress(unsigned long long BluetoothAddress)
{
    std::wostringstream ret;
    ret << std::hex << std::setfill(L'0')
        << std::setw(2) << ((BluetoothAddress >> (5 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (4 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (3 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (2 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (1 * 8)) & 0xff) << ":"
        << std::setw(2) << ((BluetoothAddress >> (0 * 8)) & 0xff);

    return ret.str();
}

float BLEDevice::ConvertInt32toFloat(int32_t num)
{
    int byte1, byte2, byte3, byte4;
    byte1 = (num & 0xff);
    byte2 = ((num >> 8) & 0xff);
    byte3 = ((num >> 16) & 0xff);
    byte4 = ((num >> 24) & 0xff);

    int sign = (byte1 & 0x80); //if sign = 1 then the number is negative
    int exponent = (((byte1 & 0x7f) << 1) | (byte2 >> 7)); //need the second 7 bits of byte1 and the very first bit of byte 2 to make a new byte [1(1), 1(2), 1(3), 1(4), 1(5), 1(6), 1(7), 2(0)]
    int32_t mantissa = ((byte2 << 16) | (byte3 << 8)) | byte4;
    float yoyo = 1;
    float power = pow(2, -23);
    for (int i = 0; i < 23; i++)
    {
        if ((mantissa & 1))
        {
            yoyo += power;
        }
        power *= 2;
        mantissa = (mantissa >> 1);
    }

    if (sign) return (-1 * pow(2, exponent - 127) * yoyo);
    else return (pow(2, exponent - 127) * yoyo);
}

void BLEDevice::DisplayLongUUID(guid yo)
{
    std::wcout << std::hex << std::setfill(L'0')
        << std::setw(8) << yo.Data1 << "-"
        << std::setw(4) << yo.Data2 << "-"
        << std::setw(4) << yo.Data3 << "-"
        << std::setw(4) << ((yo.Data4[0] & 0xff) & (yo.Data4[1] & 0xff)) << "-"
        << std::setw(12) << ((yo.Data4[2] & 0xff) & (yo.Data4[3] & 0xff) & (yo.Data4[4] & 0xff) & (yo.Data4[5] & 0xff) & (yo.Data4[6] & 0xff) & (yo.Data4[7] & 0xff)) << std::endl;
}