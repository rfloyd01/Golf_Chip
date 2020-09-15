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

//Functions related to creating and maintaining BLE connection
BLEDevice::BLEDevice(guid ServiceUUID, float freq)
{
    service_UUID = ServiceUUID;
    sampleFreq = freq;
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
                is_connected = true;
            }
        }));

    bleAdvertisementsWatcher.Start();
    std::cout << "Device Watcher has Started! Type any key and then hit enter to stop." << std::endl;
}

void BLEDevice::UpdateData()
{
    

    //Updates raw data with calibration numbers and update ax, ay ,... mz variables
    //normally the order should be ax, ay, az, however, the axis of sensors on chip are different than expected
    //if there are errors look into swapping back to normal order
    az = ((raw_sensor_data[0] - acc_off[0][0]) * acc_gain[0][0]) - ((acc_off[0][1] * abs(raw_sensor_data[1] / gravity)) + (acc_gain[0][1] * raw_sensor_data[1])) - ((acc_off[0][2] * abs(raw_sensor_data[2] / gravity)) + (acc_gain[0][2] * raw_sensor_data[2]));
    ax = -((raw_sensor_data[1] - acc_off[1][1]) * acc_gain[1][1]) - ((acc_off[1][0] * abs(raw_sensor_data[0] / gravity)) + (acc_gain[1][0] * raw_sensor_data[0])) - ((acc_off[1][2] * abs(raw_sensor_data[2] / gravity)) + (acc_gain[1][2] * raw_sensor_data[2])); //chip y-axis is OpenGL -x-axis
    ay = ((raw_sensor_data[2] - acc_off[2][2]) * acc_gain[2][2]) - ((acc_off[2][0] * abs(raw_sensor_data[0] / gravity)) + (acc_gain[2][0] * raw_sensor_data[0])) - ((acc_off[2][1] * abs(raw_sensor_data[1] / gravity)) + (acc_gain[2][1] * raw_sensor_data[1]));

    gz = (raw_sensor_data[3] - gyr_off[0]) * gyr_gain[0] * -1;
    gx = (raw_sensor_data[4] - gyr_off[1]) * gyr_gain[1] * -1;
    gy = (raw_sensor_data[5] - gyr_off[2]) * gyr_gain[2]; //the * -1 here is because OpenGL reads rotations in the opposite way as the FXOS does in this axis, i.e. clockwise = negative value vs. positive value

    mz = -(raw_sensor_data[6] - mag_off[0]) * mag_gain[0]; //the negative sign is used here because the LSM9DS1 mag x-axis is reversed on the chip
    mx = (raw_sensor_data[7] - mag_off[1]) * mag_gain[1];
    my = -(raw_sensor_data[8] - mag_off[2]) * mag_gain[2];

    //Third thing, in original code there was a section where raw data could be saved in separate variables
    //not sure if this is actually needed or not
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
        //To set up a characteristic for notification, first the Client Characteristic Configuration Descriptor (CCCD) must be written to
        GattCommunicationStatus status = co_await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Notify);
        if (status == GattCommunicationStatus::Success)
        {
            timer = std::chrono::steady_clock::now(); //start the clock to measure time between readings

            //Now need to implement the characteristic value changed event handler
            std::cout << std::dec;
            characteristic.ValueChanged(Windows::Foundation::TypedEventHandler<GattCharacteristic, GattValueChangedEventArgs>([this](GattCharacteristic car, GattValueChangedEventArgs eventArgs)
                {
                    float naynay = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - timer).count() / 1000000.0;
                    timer = std::chrono::steady_clock::now(); //reset timer

                    time_average *= (float)cycle_count / (float)(cycle_count + 1);
                    cycle_count++;
                    //std::cout << "Current time to read data is " << naynay << " milliseconds." << std::endl;
                    time_average += naynay / cycle_count;

                    //First thing is to record all of the raw data coming straight from the chip and put it into raw_data vector
                    ///TODO: Figure why this function isn't working
                    auto read = Windows::Storage::Streams::DataReader::FromBuffer(eventArgs.CharacteristicValue());
                    //reader.ByteOrder(Windows::Storage::Streams::ByteOrder::BigEndian); //BLE uses little Endian which is annoying so switch it

                    //First read acceleration data
                    for (int i = 0; i < 3; i++)
                    {
                        int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                        raw_sensor_data[i] = ayyy * acc_conversion;

                        //if (i == 0) std::cout << "Ax = " << ayyy * acc_conversion << std::endl;
                        //if (i == 1) std::cout << "Ay = " << ayyy * acc_conversion << std::endl;
                        //if (i == 2) std::cout << "Az = " << ayyy * acc_conversion << std::endl;
                    }

                    //Second is gyroscope data
                    for (int i = 0; i < 3; i++)
                    {
                        int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                        raw_sensor_data[i + 3] = ayyy * gyr_conversion;

                        //if (i == 0) std::cout << "Gx = " << ayyy * gyr_conversion << std::endl;
                        //if (i == 1) std::cout << "Gy = " << ayyy * gyr_conversion << std::endl;
                        //if (i == 2) std::cout << "Gz = " << ayyy * gyr_conversion << std::endl;
                    }

                    //Third is magnet data
                    for (int i = 0; i < 3; i++)
                    {
                        int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                        raw_sensor_data[i + 6] = ayyy * mag_conversion;

                        //if (i == 0) std::cout << "Mx = " << ayyy * mag_conversion << std::endl;
                        //if (i == 1) std::cout << "My = " << ayyy * mag_conversion << std::endl;
                        //if (i == 2) std::cout << "Mz = " << ayyy * mag_conversion << std::endl;
                    }
                    //std::cout << std::endl;
                    UpdateData(); //applies calibration information to raw data

                    //std::cout << "Average time to read data so far is " << time_average << " milliseconds." << std::endl;
                }));
        }
        else std::cout << "Was not able to properly set up notification event handling." << std::endl;
    }

    is_connected = true;
    UpdateCalibrationNumbers(); //update calibration numbers with the most current

    //the below infinite loop can have the rest of the code for whatever I want to do when the time comes
    //TODO: If I try to remove this infinte loop then the connection with the BLE device seems to falter, why is that the case?
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
            bleDevice.Close();
            break;
        }
    }

    std::cout << "Made it to end of connect function, now disconnecting from the BLE Device." << std::endl; 
}

std::vector<float> BLEDevice::GetData()
{
    return raw_sensor_data;
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

//functions from old Connection.h file
void BLEDevice::UpdateCalibrationNumbers()
{
    int line_count = 0;
    std::fstream inFile;
    inFile.open("C:/Users/Bobby/Documents/Coding/C++/BLE_33/BLE_33/Resources/calibration.txt");
    char name[256];

    while (!inFile.eof())
    {
        inFile.getline(name, 256);
        ///TODO: this if statement is crappy, code up something better later
        if (line_count == 0)      acc_off[0][0] = std::stof(name);
        else if (line_count == 1) acc_off[0][1] = std::stof(name);
        else if (line_count == 2) acc_off[0][2] = std::stof(name);
        else if (line_count == 3) acc_off[1][0] = std::stof(name);
        else if (line_count == 4) acc_off[1][1] = std::stof(name);
        else if (line_count == 5) acc_off[1][2] = std::stof(name);
        else if (line_count == 6) acc_off[2][0] = std::stof(name);
        else if (line_count == 7) acc_off[2][1] = std::stof(name);
        else if (line_count == 8) acc_off[2][2] = std::stof(name);

        else if (line_count == 9) acc_gain[0][0] = std::stof(name);
        else if (line_count == 10) acc_gain[0][1] = std::stof(name);
        else if (line_count == 11) acc_gain[0][2] = std::stof(name);
        else if (line_count == 12) acc_gain[1][0] = std::stof(name);
        else if (line_count == 13) acc_gain[1][1] = std::stof(name);
        else if (line_count == 14) acc_gain[1][2] = std::stof(name);
        else if (line_count == 15) acc_gain[2][0] = std::stof(name);
        else if (line_count == 16) acc_gain[2][1] = std::stof(name);
        else if (line_count == 17) acc_gain[2][2] = std::stof(name);

        else if (line_count == 18) gyr_off[0] = std::stof(name);
        else if (line_count == 19) gyr_off[1] = std::stof(name);
        else if (line_count == 20) gyr_off[2] = std::stof(name);

        else if (line_count == 21) gyr_gain[0] = std::stof(name);
        else if (line_count == 22) gyr_gain[1] = std::stof(name);
        else if (line_count == 23) gyr_gain[2] = std::stof(name);

        else if (line_count == 24) mag_off[0] = std::stof(name);
        else if (line_count == 25) mag_off[1] = std::stof(name);
        else if (line_count == 26) mag_off[2] = std::stof(name);

        else if (line_count == 27) mag_gain[0] = std::stof(name);
        else if (line_count == 28) mag_gain[1] = std::stof(name);
        else if (line_count == 29) mag_gain[2] = std::stof(name);

        line_count++;
    }

    if (line_count < 30) std::cout << "Some calibration information wasn't updated." << std::endl;

    inFile.close();
}

void BLEDevice::ToggleCalNumbers()
{
    if (cal_on) cal_on = 0;
    else cal_on = 1;
}

//Madgwick related functions
void BLEDevice::Madgwick()
{
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float hx, hy, hz;
    float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2by, _2bz, _4bx, _4by, _4bz, _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

    //convert Gyroscope readings to rad/s
    gx_c = gx * 3.14159 / 180.0;
    gy_c = gy * 3.14159 / 180.0;
    gz_c = gz * 3.14159 / 180.0;

    //convert Accelerometer readings to g, also create new variables
    ax_c = ax / 9.80665;
    ay_c = ay / 9.80665;
    az_c = az / 9.80665;

    mx_c = -my;
    my_c = -mx;
    mz_c = mz;

    // Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
    if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))
    {
        MadgwickIMU();
        return;
    }

    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5f * (-q1 * gx_c - q2 * gy_c - q3 * gz_c);
    qDot2 = 0.5f * (q0 * gx_c + q2 * gz_c - q3 * gy_c);
    qDot3 = 0.5f * (q0 * gy_c - q1 * gz_c + q3 * gx_c);
    qDot4 = 0.5f * (q0 * gz_c + q1 * gy_c - q2 * gx_c);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax_c * ax_c + ay_c * ay_c + az_c * az_c);
        ax_c *= recipNorm;
        ay_c *= recipNorm;
        az_c *= recipNorm;

        // Normalise magnetometer measurement
        recipNorm = invSqrt(mx_c * mx_c + my_c * my_c + mz_c * mz_c);
        mx_c *= recipNorm;
        my_c *= recipNorm;
        mz_c *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0mx = 2.0f * q0 * mx_c;
        _2q0my = 2.0f * q0 * my_c;
        _2q0mz = 2.0f * q0 * mz_c;
        _2q1mx = 2.0f * q1 * mx_c;
        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _2q0q2 = 2.0f * q0 * q2;
        _2q2q3 = 2.0f * q2 * q3;
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;

        //Reference direction of Earth's magnetic field
        //This is the original code, however, by not including Earth's magnetic field in the y-direction, OpenGL will rotate the image of the chip so that it alines with the Earth field
        //i.e. the chip will always be skewed a little bit left or right when it is rendered

        //before using magnetic values, rotate them from magnetic frame to Earth frame
        /*
        glm::quat Qm = GetRotationQuaternion({ bx, by, bz }, { 0, 0, 1 });
        std::vector<float> yoo = { mx, my, mx };
        QuatRotate(Qm, yoo);
        mx = yoo[0]; my = yoo[1]; mz = yoo[2];
        */

        hx = mx_c * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx_c * q1q1 + _2q1 * my_c * q2 + _2q1 * mz_c * q3 - mx_c * q2q2 - mx_c * q3q3;
        hy = _2q0mx * q3 + my_c * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my_c * q1q1 + my_c * q2q2 + _2q2 * mz_c * q3 - my_c * q3q3;
        _2bx = sqrt(hx * hx + hy * hy);
        _2bz = -_2q0mx * q2 + _2q0my * q1 + mz_c * q0q0 + _2q1mx * q3 - mz_c * q1q1 + _2q2 * my_c * q3 - mz_c * q2q2 + mz_c * q3q3;
        _4bx = 2.0f * _2bx;
        _4bz = 2.0f * _2bz;

        //using hz and hx to calculate bz (horizontal component) and using hy for vertical component. Also, by and bz are actually multiplied by two, in the original code bx and bz weren't
        /*
        hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx * q2q2 - mx * q3q3;
        hz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz * q2q2 + mz * q3q3;
        _2by = 2 * (_2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3);
        _2bz = 2 * sqrt(hx * hx + hz * hz);
        _4by = 2.0f * _2by;
        _4bz = 2.0f * _2bz;
        */

        /*
        _2bx = 2 * bx;
        _2by = 2 * by;
        _2bz = 2 * bz;
        _4bx = 2.0f * _2bx;
        _4by = 2.0f * _2by;
        _4bz = 2.0f * _2bz;
        */

        std::cout << "Bx = " << _2bx / 2 << " , Bz = " << _2bz / 2 << std::endl;

        // Gradient decent algorithm corrective step
        //Original code
        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my);// +(_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);

        //only 2 magnetic directions and calculations compensated for y-axis being vertical
        /*
        s0 = _2q3 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) - _2q1 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (_2by * q3 - _2bz * q2) * (_2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx) + _2bz * q1 * (_2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my) - _2by * q1 * (_2by * (q2q3 - q0q1) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 = _2q2 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q3q3 - ay_c) - _2q0 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (_2by * q2 + _2bz * q3) * (_2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx) + (-_4by * q1 + _2bz * q0) * (_2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my) + (-_2by * q0 - _4bz * q1) * (_2by * (q2q3 - q0q1) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = _2q1 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) + _2q3 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (_2by * q1 - _2bz * q0) * (_2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx) + _2bz * q3 * (_2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my) + (_2by * q3 - _4bz * q2) * (_2by * (q2q3 - q0q1) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 = _2q0 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) - 4.0f * q3 * (1 - 2.0f * q1q1 - 2.0f * q3q3 - ay_c) + _2q2 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (_2by * q0 + _2bz * q1) * (_2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx) + (-_4by * q3 + _2bz * q2) * (_2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my) + _2by * q2 * (_2by * (q2q3 - q0q1) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        */

        ///Gradient decent calculation with magnetic field included in all three directions
        /*
        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay_c) + (_2by * q3 - _2bz * q2) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q2 - _2by * q1) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay_c) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az_c) + (_2by * q2 + _2bz * q3) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (_2bx * q2 - _4by * q1 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q3 - _2by * q0 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay_c) - 4.0f * q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az_c) + (-_4bx * q2 + _2by * q1 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q0 + _2by * q3 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax_c) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay_c) + (-_4bx * q3 + _2by * q0 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (-_2bx * q0 - _4by * q3 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q1 + _2by * q2) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        */

        //Gradient decent calculation where Earth's gravity field is [0, 0, 1, 0] instead of [0, 0, 0, 1] (because in OpenGL gravity runs along the y-axis instead of the z-axis and mag field in all three directions
        /*
        s0 = _2q3 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) - _2q1 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (_2by * q3 - _2bz * q2) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q2 - _2by * q1) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        s1 = _2q2 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) - 4.0f * q1 * (1 - 2.0f * q1q1 - 2.0f * q3q3 - ay_c) - _2q0 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (_2by * q2 + _2bz * q3) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (_2bx * q2 - _4by * q1 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q3 - _2by * q0 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        s2 = _2q1 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) + _2q3 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (-_4bx * q2 + _2by * q1 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q0 + _2by * q3 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        s3 = _2q0 * (2.0f * q0q3 + 2.0f * q1q2 - ax_c) - 4.0f * q3 * (1 - 2.0f * q1q1 - 2.0f * q3q3 - ay_c) + _2q2 * (2.0f * q2q3 - 2.0f * q0q1 - az_c) + (-_4bx * q3 + _2by * q0 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2by * (q0q3 + q1q2) + _2bz * (q1q3 - q0q2) - mx_c) + (-_2bx * q0 - _4by * q3 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2by * (0.5f - q1q1 - q3q3) + _2bz * (q0q1 + q2q3) - my_c) + (_2bx * q1 + _2by * q2) * (_2bx * (q0q2 + q1q3) + _2by * (q2q3 - q0q2) + _2bz * (0.5f - q1q1 - q2q2) - mz_c);
        */

        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        // Apply feedback step
        qDot1 -= beta * s0;
        qDot2 -= beta * s1;
        qDot3 -= beta * s2;
        qDot4 -= beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    q0 += qDot1 * (1.0f / sampleFreq);
    q1 += qDot2 * (1.0f / sampleFreq);
    q2 += qDot3 * (1.0f / sampleFreq);
    q3 += qDot4 * (1.0f / sampleFreq);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    q.w = q0; q.x = q1; q.y = q2; q.z = q3;
}

void BLEDevice::Floyd()
{
    float qDot1, qDot2, qDot3, qDot4;
    float recipNorm;

    std::vector<float> a_base = { 0, 1, 0 };
    std::vector<float> mag_base = { bx, by, bz };
    std::vector<float> a_reading = { ax / gravity, ay / gravity, az / gravity };
    std::vector<float> mag_reading = { mx, my, mz };

    //First, the current rotation quaternion is updated with Gyroscope information
    //Gyroscope readings are converted to radians/sec from deg/sec
    gx_c = gx * 3.14159 / 180.0;
    gy_c = gy * 3.14159 / 180.0;
    gz_c = gz * 3.14159 / 180.0;

    qDot1 = 0.5f * (-q1 * gx_c - q2 * gy_c - q3 * gz_c);
    qDot2 = 0.5f * (q0 * gx_c + q2 * gz_c - q3 * gy_c);
    qDot3 = 0.5f * (q0 * gy_c - q1 * gz_c + q3 * gx_c);
    qDot4 = 0.5f * (q0 * gz_c + q1 * gy_c - q2 * gx_c);

    q0 += qDot1 * (1.0f / sampleFreq);
    q1 += qDot2 * (1.0f / sampleFreq);
    q2 += qDot3 * (1.0f / sampleFreq);
    q3 += qDot4 * (1.0f / sampleFreq);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    //Magnetometer only correctly reads heading when it is level, need to apply current pitch and roll numbers from Accelerometer
    //to make the Magnetometer think that it's on level ground

    float pitch = atan2(a_reading[0], sqrt(a_reading[1] * a_reading[1] + a_reading[2] * a_reading[2]));
    float roll = atan2(a_reading[2], sqrt(a_reading[0] * a_reading[0] + a_reading[1] * a_reading[1]));

    if (a_reading[1] < 0)
    {
        pitch = 3.14159 - pitch;
        //roll = 3.14159 - roll;
    }

    if (pitch < 0) pitch += 2 * 3.14159;
    if (roll < 0) roll += 2 * 3.14159;

    float X = mag_reading[0] * cos(pitch) + mag_reading[2] * sin(roll) * sin(pitch) - mag_reading[1] * cos(roll) * sin(pitch);
    float Z = mag_reading[2] * cos(roll) - mag_reading[1] * sin(roll);

    float base = atan2(bx, bz);
    float heading = atan2(X, Z) - base;

    if (heading < 0) heading += 2 * 3.14159;

    //std::cout << "Pitch = " << pitch * 180 / 3.14159 << ", Roll = " << roll * 180 / 3.14159 << ", X = " << X << ", Z = " << Z << ", Heading = " << heading * 180 / 3.14159 << std::endl;

    glm::quat Qa = GetRotationQuaternion(a_base, a_reading);
    Qa.z = -Qa.z; //uninvert the z-axis
    glm::quat Qm = glm::quat(cos(heading / 2), 0, sin(heading / 2), 0);

    //Qa shouldn't have any rotation in the y-axis, create a vector pointing along the z-axis and rotate it by Qa
    //if it's pointing in the x-axis at all, rotate Qa so that it no longer is
    std::vector<float> zee = { 0, 0, 1 };
    QuatRotate(Qa, zee);
    float deg = atan2(zee[0], zee[2]);
    if (deg < 0) deg += 2 * 3.14159;
    Qa = QuaternionMultiply({ cos(-deg / 2), 0, sin(-deg / 2), 0 }, Qa);

    //Now add rotation in the y-axis from the magnetometer reading
    Qa = QuaternionMultiply(Qm, Qa);

    //Make sure that the two quaternions are "close" to eachother before combining them, otherwise there will be an unintended 180 degree rotation
    if (DotProduct({ Qa.w, Qa.x, Qa.y, Qa.z }, { q0, q1, q2, q3 }) < 0)
    {
        Qa.w *= -1;
        Qa.x *= -1;
        Qa.y *= -1;
        Qa.z *= -1;
    }

    float gamma = .035; //represents what percentage of the rotation quaternion comes from acc. + mag. data
    q.w = (1 - gamma) * q0 + gamma * Qa.w;
    q.x = (1 - gamma) * q1 + gamma * Qa.x;
    q.y = (1 - gamma) * q2 + gamma * Qa.y;
    q.z = (1 - gamma) * q3 + gamma * Qa.z;

    // Normalise rotation quaternion quaternion
    recipNorm = invSqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q.w *= recipNorm;
    q.x *= recipNorm;
    q.y *= recipNorm;
    q.z *= recipNorm;

    //Need to set q0-q4 equal to weighted quaternion for next time algorithm is run
    q0 = q.w;
    q1 = q.x;
    q2 = q.y;
    q3 = q.z;
}

void BLEDevice::MadgwickIMU()
{
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

    //convert Gyroscope readings to rad/s
    gx_c = gx * 3.14159 / 180.0;
    gy_c = gy * 3.14159 / 180.0;
    gz_c = gz * 3.14159 / 180.0;

    //convert Accelerometer readings to g, also create new variables
    ax_c = ax;
    ay_c = az;
    az_c = ay;

    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5f * (-q1 * gx_c - q2 * gy_c - q3 * gz_c);
    qDot2 = 0.5f * (q0 * gx_c + q2 * gz_c - q3 * gy_c);
    qDot3 = 0.5f * (q0 * gy_c - q1 * gz_c + q3 * gx_c);
    qDot4 = 0.5f * (q0 * gz_c + q1 * gy_c - q2 * gx_c);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax_c *= recipNorm;
        ay_c *= recipNorm;
        az_c *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _4q0 = 4.0f * q0;
        _4q1 = 4.0f * q1;
        _4q2 = 4.0f * q2;
        _8q1 = 8.0f * q1;
        _8q2 = 8.0f * q2;
        q0q0 = q0 * q0;
        q1q1 = q1 * q1;
        q2q2 = q2 * q2;
        q3q3 = q3 * q3;

        // Gradient decent algorithm corrective step
        s0 = _4q0 * q2q2 + _2q2 * ax_c + _4q0 * q1q1 - _2q1 * ay_c;
        s1 = _4q1 * q3q3 - _2q3 * ax_c + 4.0f * q0q0 * q1 - _2q0 * ay_c - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az_c;
        s2 = 4.0f * q0q0 * q2 + _2q0 * ax_c + _4q2 * q3q3 - _2q3 * ay_c - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az_c;
        s3 = 4.0f * q1q1 * q3 - _2q1 * ax_c + 4.0f * q2q2 * q3 - _2q2 * ay_c;
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        //Apply feedback step
        qDot1 -= beta * s0;
        qDot2 -= beta * s1;
        qDot3 -= beta * s2;
        qDot4 -= beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    q0 += qDot1 * (1.0f / sampleFreq);
    q1 += qDot2 * (1.0f / sampleFreq);
    q2 += qDot3 * (1.0f / sampleFreq);
    q3 += qDot4 * (1.0f / sampleFreq);

    // Normalise quaternion
    recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= recipNorm;
    q1 *= recipNorm;
    q2 *= recipNorm;
    q3 *= recipNorm;

    q.w = q0; q.x = q1; q.y = q2; q.z = q3;
}

float BLEDevice::invSqrt(float x)
{
    if (instability_fix == 0)
    {
        /* original code */
        float halfx = 0.5f * x;
        float y = x;
        long i = *(long*)&y;
        i = 0x5f3759df - (i >> 1);
        y = *(float*)&i;
        y = y * (1.5f - (halfx * y * y));
        return y;
    }
    else if (instability_fix == 1)
    {
        /* close-to-optimal  method with low cost from http://pizer.wordpress.com/2008/10/12/fast-inverse-square-root */
        unsigned int i = 0x5F1F1412 - (*(unsigned int*)&x >> 1);
        float tmp = *(float*)&i;
        return tmp * (1.69000231f - 0.714158168f * x * tmp * tmp);
    }
    else
    {
        /* optimal but expensive method: */
        return 1.0f / sqrtf(x);
    }
}

glm::quat BLEDevice::GetRotationQuaternion()
{
    ///Change qnew back to q when done testing
    return q;
}

void BLEDevice::SetSampleFrequency(float freq)
{
    sampleFreq = freq;
}

void BLEDevice::SetMagField(float x, float y, float z)
{
    bx = x;
    by = y;
    bz = z;

    float mag = Magnitude({ x, y, z });

    m_to_g = GetRotationQuaternion({ x, y, z }, { ax, ay, az }); //g_to_m is the quaternion that will move things from the gravity frame to the magnetic frame
    g_to_m = Conjugate(m_to_g);
    //g_to_m = QuaternionMultiply(g_to_m, { 0, x / mag, y / mag, z / mag }); //rotates g_to_m by 180 degrees in the magnetic frame so that the X and Z axes are lined up correctly
}

void BLEDevice::GetLinearAcceleration()
{
    ///TODO: There's probably a quicker way to get vector than doing three quaternion rotations, update this at some point
    std::vector<float> x_vector = { gravity, 0, 0 };
    std::vector<float> y_vector = { 0, gravity, 0 };
    std::vector<float> z_vector = { 0, 0, -gravity };

    QuatRotate(q, x_vector);
    QuatRotate(q, y_vector);
    QuatRotate(q, z_vector);

    lin_ax = x_vector[1] - ax;
    lin_ay = y_vector[1] - ay;
    lin_az = z_vector[1] - az;

    //std::cout << "[" << lin_ax << ", " << lin_ay << ", " << lin_az << "]" << std::endl;
}

void BLEDevice::UpdatePosition()
{
    GetLinearAcceleration();
    if (acceleration_event)
    {
        float delta_t = glfwGetTime() - position_timer;
        position_timer = glfwGetTime();

        float ax0 = lin_ax;
        float ay0 = lin_ay;
        float az0 = lin_az;

        //Set threshold on Linear Acceleration to help with drift
        if (lin_ax < lin_acc_threshold && lin_ax > -lin_acc_threshold) lin_ax = 0;
        if (lin_ay < lin_acc_threshold && lin_ay > -lin_acc_threshold) lin_ay = 0;
        if (lin_az < lin_acc_threshold && lin_az > -lin_acc_threshold) lin_az = 0;

        if (lin_ax == 0 && lin_ay == 0 && lin_az == 0)
        {
            if (just_stopped)
            {
                if (glfwGetTime() - end_timer > .01) //if there's been no acceleration for .1 seconds, set velocity to zero to eliminate drift
                {
                    vel_x = 0;
                    vel_y = 0;
                    vel_z = 0;
                    acceleration_event = 0;
                    just_stopped = 0;
                }
            }
            else
            {
                just_stopped = 1;
                end_timer = glfwGetTime();
            }
        }
        else
        {
            if (just_stopped)
            {
                just_stopped = 0;
            }
        }

        float ax1 = lin_ax;
        float ay1 = lin_ay;
        float az1 = lin_az;

        float vx0 = vel_x;
        float vy0 = vel_y;
        float vz0 = vel_z;

        vel_x += Integrate(ax0, ax1, delta_t);
        vel_y += Integrate(ay0, ay1, delta_t);
        vel_z += Integrate(az0, az1, delta_t);

        //the minus signs are because movement was in opposite direction of what was expected
        loc_x -= movement_scale * Integrate(vx0, vel_x, delta_t);
        loc_y -= movement_scale * Integrate(vy0, vel_y, delta_t);
        loc_z -= movement_scale * Integrate(vz0, vel_z, delta_t);
    }
    else
    {
        if (lin_ax > lin_acc_threshold || lin_ax < -lin_acc_threshold) acceleration_event = 1;
        else if (lin_ay > lin_acc_threshold || lin_ay < -lin_acc_threshold) acceleration_event = 1;
        else if (lin_az > lin_acc_threshold || lin_az < -lin_acc_threshold) acceleration_event = 1;

        if (acceleration_event) position_timer = glfwGetTime();
    }
}

void BLEDevice::SetPositionTimer()
{
    position_timer = glfwGetTime();
}

void BLEDevice::ResetPosition()
{
    lin_ax = 0, lin_ay = 0, lin_az = 0;
    vel_x = 0, vel_y = 0, vel_z = 0;
    loc_x = 0, loc_y = 0, loc_z = 0;
}

//Below are all function related to quaternion manipulation and other math
float BLEDevice::DotProduct(std::vector<float> vec1, std::vector<float> vec2)
{
    //returns the dot product of two vectors, the vectors need to be the same size
    if (vec1.size() != vec2.size())
    {
        std::cout << "Vectors must be of the same length." << std::endl;
        return 0;
    }

    float answer = 0;
    for (int i = 0; i < vec1.size(); i++) answer += vec1[i] * vec2[i];
    return answer;
}

std::vector<float> BLEDevice::CrossProduct(std::vector<float> vec1, std::vector<float> vec2)
{
    std::vector<float> answer;
    if (vec1.size() != vec2.size())
    {
        std::cout << "Vectors must be of the same length." << std::endl;
        return answer;
    }

    answer.push_back(vec1[1] * vec2[2] - vec1[2] * vec2[1]);
    answer.push_back(vec1[2] * vec2[0] - vec1[0] * vec2[2]);
    answer.push_back(vec1[0] * vec2[1] - vec1[1] * vec2[0]);
    return answer;
}

float BLEDevice::Magnitude(std::vector<float> vec)
{
    //returns the magnitude of vec
    float answer = 0;
    for (int i = 0; i < vec.size(); i++) answer += vec[i] * vec[i];
    return sqrt(answer);
}

void BLEDevice::Normalize(glm::quat& q)
{
    float magnitude = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    q.w /= magnitude;
    q.x /= magnitude;
    q.y /= magnitude;
    q.z /= magnitude;
}

void BLEDevice::QuatRotate(glm::quat q, std::vector<float>& data)
{
    //Takes the float vector data and rotates it according to quaternion q

    double w, x, y, z;

    if (data.size() == 3)
    {
        //a 3-diomensional vector is passed
        w = 0;
        x = data[0];
        y = data[1];
        z = data[2];
    }
    else
    {
        std::cout << "Need a three dimensional vector." << std::endl;
        return;
    }

    glm::quat q_star; q_star.w = q.w; q_star.x = -q.x; q_star.y = -q.y; q_star.z = -q.z;

    double temp[4] = { 0 };
    temp[0] = q.w * w - q.x * x - q.y * y - q.z * z;
    temp[1] = q.w * x + q.x * w + q.y * z - q.z * y;
    temp[2] = q.w * y - q.x * z + q.y * w + q.z * x;
    temp[3] = q.w * z + q.x * y - q.y * x + q.z * w;

    w = temp[0]; x = temp[1]; y = temp[2]; z = temp[3];

    temp[0] = w * q_star.w - x * q_star.x - y * q_star.y - z * q_star.z;
    data[0] = w * q_star.x + x * q_star.w + y * q_star.z - z * q_star.y;
    data[1] = w * q_star.y - x * q_star.z + y * q_star.w + z * q_star.x;
    data[2] = w * q_star.z + x * q_star.y - y * q_star.x + z * q_star.w;
}

void BLEDevice::QuatRotate(glm::quat q1, glm::quat& q2)
{
    //Takes the quaternion q2 and rotates it according to quaternion q1

    double w, x, y, z;
    glm::quat q_star; q_star.w = q1.w; q_star.x = -q1.x; q_star.y = -q1.y; q_star.z = -q1.z;

    double temp[4] = { 0 };
    temp[0] = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    temp[1] = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    temp[2] = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    temp[3] = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    w = temp[0]; x = temp[1]; y = temp[2]; z = temp[3];

    q2.w = w * q_star.w - x * q_star.x - y * q_star.y - z * q_star.z;
    q2.x = w * q_star.x + x * q_star.w + y * q_star.z - z * q_star.y;
    q2.y = w * q_star.y - x * q_star.z + y * q_star.w + z * q_star.x;
    q2.z = w * q_star.z + x * q_star.y - y * q_star.x + z * q_star.w;
}

glm::quat BLEDevice::QuaternionMultiply(glm::quat q1, glm::quat q2)
{
    glm::quat new_q;

    new_q.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    new_q.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    new_q.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    new_q.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;

    return new_q;
}

glm::quat BLEDevice::GetRotationQuaternion(std::vector<float> vec1, std::vector<float> vec2)
{
    //returns the Quaternion that rotates vec1 to vec2
    glm::quat q;
    std::vector<float> cross = CrossProduct(vec1, vec2);
    q.w = sqrt(Magnitude(vec1) * Magnitude(vec1) * Magnitude(vec2) * Magnitude(vec2)) + DotProduct(vec1, vec2);
    q.x = cross[0];
    q.y = cross[1];
    q.z = cross[2];

    float mag = Magnitude({ q.w, q.x, q.y, q.z });
    q.w /= mag; q.x /= mag; q.y /= mag; q.z /= mag;
    return q;
}

glm::quat BLEDevice::GetRotationQuaternion(float angle, std::vector<float> vec)
{
    //returns the Quaternion that rotates about the axis defined by vec, by and angle of angle
    glm::quat q;
    q.w = cos(angle * 3.14159 / 360);
    q.x = vec[0] * sin(angle * 3.14159 / 360);
    q.y = vec[0] * sin(angle * 3.14159 / 360);
    q.z = vec[0] * sin(angle * 3.14159 / 360);

    float mag = Magnitude({ q.w, q.x, q.y, q.z });
    q.w /= mag; q.x /= mag; q.y /= mag; q.z /= mag;
    return q;
}

glm::quat BLEDevice::Conjugate(glm::quat q)
{
    glm::quat yo;
    yo.w = q.w;
    yo.x = -q.x;
    yo.y = -q.y;
    yo.z = -q.z;

    return yo;
}

float BLEDevice::Integrate(float one, float two, float dt)
{
    return ((one + two) / 2) * dt;
}
