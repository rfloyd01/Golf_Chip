#include "pch.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Header_Files/BluetoothLE.h>
#include <Header_Files/print.h>
#include <Header_Files/sensor_fusion.h>
#include <Header_Files/quaternion_functions.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

//PUBLIC FUNCTIONS
//Constructors
BLEDevice::BLEDevice(guid ServiceUUID, float freq)
{
    service_UUID = ServiceUUID;
    sampleFreq = freq;

    for (int i = 0; i < number_of_samples; i++)
    {
        //Initialize vectors that hold calibrated and raw sensor data to 0
        //TODO: Replace this with a create zero vector function at some point
        accelerometer[X].push_back(0); accelerometer[Y].push_back(0); accelerometer[Z].push_back(0);
        gyroscope[X].push_back(0); gyroscope[Y].push_back(0); gyroscope[Z].push_back(0);
        magnetometer[X].push_back(0); magnetometer[Y].push_back(0); magnetometer[Z].push_back(0);
        raw_sensor_data.push_back(0); raw_sensor_data.push_back(0); raw_sensor_data.push_back(0);
        raw_sensor_data.push_back(0); raw_sensor_data.push_back(0); raw_sensor_data.push_back(0);
        raw_sensor_data.push_back(0); raw_sensor_data.push_back(0); raw_sensor_data.push_back(0);
    }
}

//Connection
void BLEDevice::Connect()
{
    SetUpDeviceWatcher();
}

//Data Updating
void BLEDevice::Update()
{
    MadgwickUpdate();
    //UpdatePosition(); //Leave this commented out until a better way to stabalize position is found
}
void BLEDevice::MadgwickUpdate()
{
    if (!data_available) return; //only do things if there's new data to process

    //set up current time information
    last_time_stamp = time_stamp;
    time_stamp += 1000.0 / sampleFreq;
    float delta_t = (float)((time_stamp - last_time_stamp) / 1000.0);

    //Call Madgwick function here
    int cs = current_sample; //this is only here because it became annoying to keep writing out current_sample
    Quaternion = Madgwick(Quaternion, gyroscope[X][cs], gyroscope[Y][cs], gyroscope[Z][cs], accelerometer[X][cs], accelerometer[Y][cs], accelerometer[Z][cs], magnetometer[X][cs], magnetometer[Y][cs], magnetometer[Z][cs], delta_t, beta);
    current_sample++;

    if (current_sample >= number_of_samples)
    {
        data_available = false; //there is no more new data to look at, setting this variable to false would prevent Madgwick filter from running
        current_sample--; //set current sample to last sample while waiting for more data to come in
    }
}
void BLEDevice::UpdateData()
{
    //Updates raw data with calibration numbers and update ax, ay ,... mz variables
    //since the chip x, y and z axes are different than OpenGL's x, y and z axes some of the variables are swapped around
    //The count variable counts which data point should be used as multiple data points are taken from the sensor at a time

    //chip ax = opengl -az, chip ay = opengl ax, chip az = opengl -ay
    //The following acceleration numbers are used in a different calibration, revisit this in the future
    //az = ((raw_sensor_data[0] - acc_off[0][0]) * acc_gain[0][0]) - ((acc_off[0][1] * abs(raw_sensor_data[1] / gravity)) + (acc_gain[0][1] * raw_sensor_data[1])) - ((acc_off[0][2] * abs(raw_sensor_data[2] / gravity)) + (acc_gain[0][2] * raw_sensor_data[2]));
    //ax = ((raw_sensor_data[1] - acc_off[1][1]) * acc_gain[1][1]) - ((acc_off[1][0] * abs(raw_sensor_data[0] / gravity)) + (acc_gain[1][0] * raw_sensor_data[0])) - ((acc_off[1][2] * abs(raw_sensor_data[2] / gravity)) + (acc_gain[1][2] * raw_sensor_data[2])); //chip y-axis is OpenGL -x-axis
    //ay = ((raw_sensor_data[2] - acc_off[2][2]) * acc_gain[2][2]) - ((acc_off[2][0] * abs(raw_sensor_data[0] / gravity)) + (acc_gain[2][0] * raw_sensor_data[0])) - ((acc_off[2][1] * abs(raw_sensor_data[1] / gravity)) + (acc_gain[2][1] * raw_sensor_data[1]));

    std::vector<float> mprime;
    for (int i = 0; i < number_of_samples; i++)
    {
        accelerometer[X][i] = (acc_gain[0][0] * (raw_sensor_data[0 + 9 * i] - acc_off[0][0])) + (acc_gain[0][1] * (raw_sensor_data[1 + 9 * i] - acc_off[1][1])) + (acc_gain[0][2] * (raw_sensor_data[2 + 9 * i] - acc_off[2][2]));
        accelerometer[Y][i] = (acc_gain[1][0] * (raw_sensor_data[0 + 9 * i] - acc_off[0][0])) + (acc_gain[1][1] * (raw_sensor_data[1 + 9 * i] - acc_off[1][1])) + (acc_gain[1][2] * (raw_sensor_data[2 + 9 * i] - acc_off[2][2]));
        accelerometer[Z][i] = (acc_gain[2][0] * (raw_sensor_data[0 + 9 * i] - acc_off[0][0])) + (acc_gain[2][1] * (raw_sensor_data[1 + 9 * i] - acc_off[1][1])) + (acc_gain[2][2] * (raw_sensor_data[2 + 9 * i] - acc_off[2][2]));

        gyroscope[X][i] = (raw_sensor_data[3 + 9 * i] - gyr_off[0]) * gyr_gain[0];
        gyroscope[Y][i] = (raw_sensor_data[4 + 9 * i] - gyr_off[1]) * gyr_gain[1];
        gyroscope[Z][i] = (raw_sensor_data[5 + 9 * i] - gyr_off[2]) * gyr_gain[2];

        //The negatives are only here because the magnetic field goes directly into computer screen at my desk in West Hartford, CT
        magnetometer[X][i] = (raw_sensor_data[6 + 9 * i] - mag_off[0]) * mag_gain[0];
        magnetometer[Y][i] = (raw_sensor_data[7 + 9 * i] - mag_off[1]) * mag_gain[1];
        magnetometer[Z][i] = (raw_sensor_data[8 + 9 * i] - mag_off[2]) * mag_gain[2];

        //Rotate magnet data to appropriate frame
        //mprime = { mx[i], my[i], mz[i] };
        //QuatRotate(m_to_mprime, mprime);
        //mx[i] = mprime[0]; my[i] = mprime[1]; mz[i] = mprime[2];
    }

    //time_stamp = raw_sensor_data[number_of_samples * 9];
    //std::cout << time_stamp << std::endl;
    current_sample = 0; //since a new set of data has come in, start from the beginning of it when doing Madgwick filter
    data_available = true; //main program can now run filter, render, etc.
}
void BLEDevice::UpdateCalibrationNumbers()
{
    int line_count = 0;
    std::fstream inFile;
    inFile.open("Resources/calibration.txt");
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
void BLEDevice::ResetPosition()
{
    lin_ax = 0, lin_ay = 0, lin_az = 0;
    vel_x = 0, vel_y = 0, vel_z = 0;
    loc_x = 0, loc_y = 0, loc_z = 0;
}
void BLEDevice::resetTime()
{
    //reset time to be -1000 / the sample frequeny, this way when the first bit of data is processed it will start with a time stamp of 0.00
    time_stamp = -1000.0 / sampleFreq;
}

//Variable Setting Functions
void BLEDevice::SetSampleFrequency(float freq)
{
    sampleFreq = freq;
}
void BLEDevice::SetMagField()
{
    bx = magnetometer[X].back();
    by = magnetometer[Y].back();
    bz = magnetometer[Z].back();

    float mag = Magnitude({ bx, by, bz });

    //want the most recent set of data, so use ax.back(), etc.

    m_to_g = GetRotationQuaternion({ bx, by, bz }, { accelerometer[X].back(), accelerometer[Y].back(), accelerometer[Z].back() }); //g_to_m is the quaternion that will move things from the gravity frame to the magnetic frame
    g_to_m = Conjugate(m_to_g);
    //g_to_m = QuaternionMultiply(g_to_m, { 0, x / mag, y / mag, z / mag }); //rotates g_to_m by 180 degrees in the magnetic frame so that the X and Z axes are lined up correctly
    m_to_mprime = GetRotationQuaternion({ sqrt(bx * bx + by * by), 0, bz }, { bx, by, bz }); //this quaternion will rotate current magnetic reading to desired frame (i.e. +x and -z when pointing sensor at monitor)
    //m_to_mprime = { 0, 0, 0, 1 };
    std::cout << "Magnetic rotate quaternion is: {" << m_to_mprime.w << ", " << m_to_mprime.x << ", " << m_to_mprime.y << ", " << m_to_mprime.z << '}' << std::endl;
}
void BLEDevice::SetPositionTimer()
{
    position_timer = glfwGetTime();
}

//Data Passing Functions
float BLEDevice::GetData(int index)
{
    return raw_sensor_data[index];
}
std::vector<float>* BLEDevice::GetData(DataType dt, Axis a)
{
    if (dt == ACCELERATION) return &accelerometer[a];
    else if (dt == ROTATION) return &gyroscope[a];
    else if (dt == MAGNETIC) return &magnetometer[a];
}
void BLEDevice::GetLinearAcceleration()
{
    ///TODO: There's probably a quicker way to get vector than doing three quaternion rotations, update this at some point
    std::vector<float> x_vector = { gravity, 0, 0 };
    std::vector<float> y_vector = { 0, gravity, 0 };
    std::vector<float> z_vector = { 0, 0, -gravity };

    QuatRotate(Quaternion, x_vector);
    QuatRotate(Quaternion, y_vector);
    QuatRotate(Quaternion, z_vector);

    lin_ax = x_vector[1] - accelerometer[X][current_sample];
    lin_ay = y_vector[1] - accelerometer[Y][current_sample];
    lin_az = z_vector[1] - accelerometer[Z][current_sample];

    //std::cout << "[" << lin_ax << ", " << lin_ay << ", " << lin_az << "]" << std::endl;
}
glm::quat BLEDevice::GetOpenGLQuaternion()
{
    //The purpose of this function is to return the quaternion in a form that OpenGL likes
    //The axes of the chip are different than what OpenGL expects so the axes are switched accordingly here
    //+X OpenGL = +Y Sensor, +Y OpenGL = +Z Sensor, +Z OpenGL = +X Sensor
    return { Quaternion.w, Quaternion.y, Quaternion.z, Quaternion.x };
}
int BLEDevice::getCurrentSample()
{
    return current_sample;
}

//Other Public Functions
void BLEDevice::DisplayLongUUID(guid yo)
{
    std::wcout << std::hex << std::setfill(L'0')
        << std::setw(8) << yo.Data1 << "-"
        << std::setw(4) << yo.Data2 << "-"
        << std::setw(4) << yo.Data3 << "-"
        << std::setw(4) << ((yo.Data4[0] & 0xff) & (yo.Data4[1] & 0xff)) << "-"
        << std::setw(12) << ((yo.Data4[2] & 0xff) & (yo.Data4[3] & 0xff) & (yo.Data4[4] & 0xff) & (yo.Data4[5] & 0xff) & (yo.Data4[6] & 0xff) & (yo.Data4[7] & 0xff)) << std::endl;
}

//PRIVATE FUNCTIONS
//Connection
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
                    //First thing is to record all of the raw data coming straight from the chip and put it into raw_data vector
                    ///TODO: Figure why this function isn't working
                    auto read = Windows::Storage::Streams::DataReader::FromBuffer(eventArgs.CharacteristicValue());
                    //reader.ByteOrder(Windows::Storage::Streams::ByteOrder::BigEndian); //BLE uses little Endian which is annoying so switch it

                    for (int i = 0; i < number_of_samples; i++)
                    {
                        //std::cout << i << std::endl;
                        //First read acceleration data
                        for (int j = 0; j < 3; j++)
                        {
                            int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                            raw_sensor_data[j + 9 * i] = ayyy * acc_conversion;
                        }

                        //Second is gyroscope data
                        for (int j = 0; j < 3; j++)
                        {
                            int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                            raw_sensor_data[j + 3 + 9 * i] = ayyy * gyr_conversion;
                        }

                        //Third is magnet data
                        for (int j = 0; j < 3; j++)
                        {
                            int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                            raw_sensor_data[j + 6 + 9 * i] = ayyy * mag_conversion;
                        }
                    }

                    //Last is time stamp data
                    //time is stored like Byte3 - Byte4 - Byte1 - Byte2, rearrange so it reads Byte4 - Byte3 - Byte2 - Byte1
                    //int32_t tizime = 0;
                    //tizime = tizime | (read.ReadByte() << 16);
                    //tizime = tizime | (read.ReadByte() << 24);
                    //tizime = tizime | (read.ReadByte());
                    //tizime = tizime | (read.ReadByte() << 8);
                    //time_stamp = tizime;

                    //Time gathering stuff
                    /*
                    float naynay = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - timer).count() / 1000000.0;
                    std::cout << "Currently it takes " << naynay << " milliseconds to "
                        "gather data on peripheral, send it to central and process data." << std::endl;
                    timer = std::chrono::steady_clock::now(); //reset timer

                    time_average *= (float)cycle_count / (float)(cycle_count + 1);
                    cycle_count++;

                    time_average += naynay / cycle_count;
                    std::cout << "Average time to get data is: " << time_average << " seconds." << std::endl << std::endl;
                    */

                    UpdateData(); //applies calibration information to raw data
                    //new_data = true; //by setting this value to true, calibration info will be applied to raw data in main loop of program
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

//Util Functions
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
float BLEDevice::Integrate(float one, float two, float dt)
{
    return ((one + two) / 2) * dt;
}

//Other Private Functions
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
