#include "pch.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
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

    for (int i = 0; i < 3; i++)
    {
        makeZeroVector(&accelerometer[i], number_of_samples);
        makeZeroVector(&gyroscope[i], number_of_samples);
        makeZeroVector(&magnetometer[i], number_of_samples);
        makeZeroVector(&r_accelerometer[i], number_of_samples);
        makeZeroVector(&r_gyroscope[i], number_of_samples);
        makeZeroVector(&r_magnetometer[i], number_of_samples);
        makeZeroVector(&linear_acceleration[i], number_of_samples);
        makeZeroVector(&velocity[i], number_of_samples);
        makeZeroVector(&location[i], number_of_samples);
    }

    updateCalibrationNumbers(); //set calibration variables with most current data from calibration.txt
}

//Connection
void BLEDevice::connect()
{
    //Created a separate function for this instead of just doing it in the constructor so that time of connection can
    //be controlled from the main program loop
    setUpDeviceWatcher();
}

//Data Updating
void BLEDevice::masterUpdate()
{
    //Master update function, calls for Madgwick filter to be run and then uses that data to call updatePosition() which calculates current lin_acc., vel. and loc.
    updateMadgwick();
    //TODO - linear acceleration updating seems to be incorrect, take a look at some point
    //updatePosition();
}
void BLEDevice::updateCalibrationNumbers()
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
void BLEDevice::resetPosition()
{
    //resets sensor lin_acc., vel. and loc. to 0 so that club will be rendered back at center of screen
    //this is helpful because small fluctuations in accelerometer data will cause image of club to drift accros screen over time, even without movement
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < number_of_samples; j++)
        {
            linear_acceleration[i][j] = 0;
            velocity[i][j] = 0;
            location[i][i] = 0;
        }
    }
}
void BLEDevice::resetTime()
{
    //reset time to be -1000 / the sample frequeny, this way when the first bit of data is processed it will start with a time stamp of 0.00
    time_stamp = -1000.0 / sampleFreq;
}

//Variable Setting Functions
void BLEDevice::setSampleFrequency(float freq)
{
    sampleFreq = freq;
}
void BLEDevice::setMagField()
{
    //TODO - Need to find a good way to rotate current magnetic reading to desired magnetic reading, this will allow the sensor to start off pointing straight when rendered
    bx = magnetometer[X].back();
    by = magnetometer[Y].back();
    bz = magnetometer[Z].back();

    float mag = Magnitude({ bx, by, bz });

    m_to_g = GetRotationQuaternion({ bx, by, bz }, { accelerometer[X].back(), accelerometer[Y].back(), accelerometer[Z].back() }); //g_to_m is the quaternion that will move things from the gravity frame to the magnetic frame
    g_to_m = Conjugate(m_to_g);
    m_to_mprime = GetRotationQuaternion({ sqrt(bx * bx + by * by), 0, bz }, { bx, by, bz }); //this quaternion will rotate current magnetic reading to desired frame (i.e. +x and -z when pointing sensor at monitor)
}

//Data Passing Functions
std::vector<float>* BLEDevice::getData(DataType dt, Axis a)
{
    if (dt == ACCELERATION) return &accelerometer[a];
    else if (dt == ROTATION) return &gyroscope[a];
    else if (dt == MAGNETIC) return &magnetometer[a];
    else if (dt == LINEAR_ACCELERATION) return &linear_acceleration[a];
    else if (dt == VELOCITY) return &velocity[a];
    else if (dt == LOCATION) return &location[a];
}
std::vector<float>* BLEDevice::getRawData(DataType dt, Axis a)
{
    if (dt == ACCELERATION) return &r_accelerometer[a];
    else if (dt == ROTATION) return &r_gyroscope[a];
    else if (dt == MAGNETIC) return &r_magnetometer[a];
}
glm::vec3 BLEDevice::getLocation()
{
    return { location[X][current_sample], location[Y][current_sample], location[Z][current_sample] };
}
glm::quat BLEDevice::getOpenGLQuaternion()
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
float BLEDevice::getCurrentTime()
{
    //returns the time that current sample was taken at in seconds
    return time_stamp / 1000.0;
}

//Other Public Functions
void BLEDevice::displayLongUUID(guid yo)
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
void BLEDevice::setUpDeviceWatcher()
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
                        //First read acceleration data
                        for (int j = 0; j < 3; j++)
                        {
                            //x, y, z = 0, 1, 2 respectively
                            int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                            r_accelerometer[j][i] = ayyy * acc_conversion;
                        }

                        //Second is gyroscope data
                        for (int j = 0; j < 3; j++)
                        {
                            int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                            r_gyroscope[j][i] = ayyy * gyr_conversion;
                        }

                        //Third is magnet data
                        for (int j = 0; j < 3; j++)
                        {
                            int16_t ayyy = ((read.ReadByte()) | (read.ReadByte() << 8));
                            r_magnetometer[j][i] = ayyy * mag_conversion;
                        }
                    }

                    updateSensorData(); //applies calibration information to raw data
                }));
        }
        else std::cout << "Was not able to properly set up notification event handling." << std::endl;
    }

    is_connected = true;

    //the below infinite loop can have the rest of the code for whatever I want to do when the time comes
    //TODO: If I try to remove this infinte loop then the connection with the BLE device seems to falter, why is that the case?
    for (;;)
    {
        auto cnnct = bleDevice.ConnectionStatus();
        if (cnnct == Bluetooth::BluetoothConnectionStatus::Connected)
        {
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

//Internal Updating Functions
void BLEDevice::updateSensorData()
{
    //Updates raw data with calibration numbers and fills acceleromter, gyroscope and magnetometer vectors with updated values
    //The count variable counts which data point should be used as multiple data points are taken from the sensor at a time
    for (int i = 0; i < number_of_samples; i++)
    {
        accelerometer[X][i] = (acc_gain[0][0] * (r_accelerometer[X][i] - acc_off[0][0])) + (acc_gain[0][1] * (r_accelerometer[Y][i] - acc_off[1][1])) + (acc_gain[0][2] * (r_accelerometer[Z][i] - acc_off[2][2]));
        accelerometer[Y][i] = -((acc_gain[1][0] * (r_accelerometer[X][i] - acc_off[0][0])) + (acc_gain[1][1] * (r_accelerometer[Y][i] - acc_off[1][1])) + (acc_gain[1][2] * (r_accelerometer[Z][i] - acc_off[2][2]))); //take out negative sign when using FXOS
        accelerometer[Z][i] = (acc_gain[2][0] * (r_accelerometer[X][i] - acc_off[0][0])) + (acc_gain[2][1] * (r_accelerometer[Y][i] - acc_off[1][1])) + (acc_gain[2][2] * (r_accelerometer[Z][i] - acc_off[2][2]));

        gyroscope[X][i] = (r_gyroscope[X][i] - gyr_off[0]) * gyr_gain[0];
        gyroscope[Y][i] = (r_gyroscope[Y][i] - gyr_off[1]) * gyr_gain[1];
        gyroscope[Z][i] = (r_gyroscope[Z][i] - gyr_off[2]) * gyr_gain[2];

        magnetometer[X][i] = -(r_magnetometer[X][i] - mag_off[0]) * mag_gain[0]; //take out negative sign when using FXOS
        magnetometer[Y][i] = -(r_magnetometer[Y][i] - mag_off[1]) * mag_gain[1]; //take out negative sign when using FXOS
        magnetometer[Z][i] = (r_magnetometer[Z][i] - mag_off[2]) * mag_gain[2];

        masterUpdate(); //update rotation quaternion, lin_acc., velocity and position with every new data point that comes in

        //Each data point from the sensor is recorded at time intervals of 1/sampleFreq seconds from each other, however, this program can process them quicker than that
        //Hard code a wait time of 1/sampleFreq seconds to ensure that rendering looks smooth
        std::chrono::high_resolution_clock::time_point tt = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tt).count() <= 1000.0 / sampleFreq) {}
    }
    current_sample = 0; //since a new set of data has come in, start from the beginning of it when doing Madgwick filter
    data_available = true; //main program can now run filter, render, etc.
}
void BLEDevice::updateMadgwick()
{
    if (!data_available) return; //only do things if there's new data to process

    //set up current time information
    last_time_stamp = time_stamp;
    time_stamp += 1000.0 / sampleFreq;
    float delta_t = (float)((time_stamp - last_time_stamp) / 1000.0);

    //Call Madgwick function here
    int cs = current_sample; //this is only here because it became annoying to keep writing out current_sample
    //Quaternion = Madgwick(Quaternion, gyroscope[X][cs], gyroscope[Y][cs], gyroscope[Z][cs], accelerometer[X][cs], accelerometer[Y][cs], accelerometer[Z][cs], magnetometer[X][cs], magnetometer[Y][cs], magnetometer[Z][cs], delta_t, beta);
    Quaternion = MadgwickModified(Quaternion, gyroscope[X][cs], gyroscope[Y][cs], gyroscope[Z][cs], accelerometer[X][cs], accelerometer[Y][cs], accelerometer[Z][cs], magnetometer[X][cs], magnetometer[Y][cs], magnetometer[Z][cs], { 0, bx, by, bz }, delta_t, beta);
    current_sample++;

    if (current_sample >= number_of_samples)
    {
        data_available = false; //there is no more new data to look at, setting this variable to false would prevent Madgwick filter from running
        current_sample--; //set current sample to last sample while waiting for more data to come in
    }
}
void BLEDevice::updateLinearAcceleration()
{
    ///TODO: There's probably a quicker way to get vector than doing three quaternion rotations, update this at some point
    std::vector<float> x_vector = { gravity, 0, 0 };
    std::vector<float> y_vector = { 0, gravity, 0 };
    std::vector<float> z_vector = { 0, 0, -gravity };

    QuatRotate(Quaternion, x_vector);
    QuatRotate(Quaternion, y_vector);
    QuatRotate(Quaternion, z_vector);

    linear_acceleration[X][current_sample] = x_vector[1] - accelerometer[X][current_sample];
    linear_acceleration[Y][current_sample] = y_vector[1] - accelerometer[Y][current_sample];
    linear_acceleration[Z][current_sample] = z_vector[1] - accelerometer[Z][current_sample];

    //Set threshold on Linear Acceleration to help with drift
    if (linear_acceleration[X][current_sample] < lin_acc_threshold && linear_acceleration[X][current_sample] > -lin_acc_threshold) linear_acceleration[X][current_sample] = 0;
    if (linear_acceleration[Y][current_sample] < lin_acc_threshold && linear_acceleration[Y][current_sample] > -lin_acc_threshold) linear_acceleration[Y][current_sample] = 0;
    if (linear_acceleration[Z][current_sample] < lin_acc_threshold && linear_acceleration[Z][current_sample] > -lin_acc_threshold) linear_acceleration[Z][current_sample] = 0;
}
void BLEDevice::updatePosition()
{
    updateLinearAcceleration();
    if (acceleration_event)
    {
        //these variables build on themselves so need to utilize previous value
        //because of the way sensor data is stored, may need to wrap around to end of vector to get previous value
        int last_sample = current_sample - 1;
        if (current_sample == 0) last_sample = number_of_samples - 1; //last data point would have been end of current vector

        if (linear_acceleration[X][current_sample] == 0 && (linear_acceleration[Y][current_sample] && linear_acceleration[Z][current_sample] == 0))
        {
            if (just_stopped)
            {
                if (time_stamp - end_timer > .01) //if there's been no acceleration for .1 seconds, set velocity to zero to eliminate drift
                {
                    velocity[X][current_sample] = 0;
                    velocity[Y][current_sample] = 0;
                    velocity[Z][current_sample] = 0;
                    acceleration_event = 0;
                    just_stopped = 0;
                }
            }
            else
            {
                just_stopped = 1;
                end_timer = time_stamp;
            }
        }
        else
        {
            if (just_stopped)
            {
                just_stopped = 0;
            }
        }

        //velocity variable builds on itself so need to reference previous value
        velocity[X][current_sample] = velocity[X][last_sample] + integrate(linear_acceleration[X][last_sample], linear_acceleration[X][current_sample], 1.0 / sampleFreq);
        velocity[Y][current_sample] = velocity[Y][last_sample] + integrate(linear_acceleration[Y][last_sample], linear_acceleration[Y][current_sample], 1.0 / sampleFreq);
        velocity[Z][current_sample] = velocity[Z][last_sample] + integrate(linear_acceleration[Z][last_sample], linear_acceleration[Z][current_sample], 1.0 / sampleFreq);

        //location variables also build on themselves so need to reference previous values
        //the minus signs are because movement was in opposite direction of what was expected
        location[X][current_sample] = location[X][last_sample] + movement_scale * integrate(velocity[X][last_sample], velocity[X][current_sample], 1.0 / sampleFreq);
        location[X][current_sample] = location[Y][last_sample] + movement_scale * integrate(velocity[Y][last_sample], velocity[Y][current_sample], 1.0 / sampleFreq);
        location[X][current_sample] = location[Z][last_sample] + movement_scale * integrate(velocity[Z][last_sample], velocity[Z][current_sample], 1.0 / sampleFreq);
    }
    else
    {
        if (linear_acceleration[X][current_sample] > lin_acc_threshold || linear_acceleration[X][current_sample] < -lin_acc_threshold) acceleration_event = 1;
        else if (linear_acceleration[Y][current_sample] > lin_acc_threshold || linear_acceleration[Y][current_sample] < -lin_acc_threshold) acceleration_event = 1;
        else if (linear_acceleration[Z][current_sample] > lin_acc_threshold || linear_acceleration[Z][current_sample] < -lin_acc_threshold) acceleration_event = 1;

        if (acceleration_event) position_timer = time_stamp;
    }
}

//Util Functions
float BLEDevice::convertInt32toFloat(int32_t num)
{
    //This function was originally needed to convert bits of data coming from sensor to a readable format, not necessary any longer but keeping just in case
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
float BLEDevice::integrate(float one, float two, float dt)
{
    //Returns the area under the curve of two adjacent points on a graph
    return ((one + two) / 2) * dt;
}
void BLEDevice::makeZeroVector(std::vector<float>* vec, int number_of_zeros)
{
    for (int i = 0; i < number_of_zeros; i++) vec->push_back(0);
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
