#include "pch.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <bitset>
#include <stdlib.h> 

#include "BluetoothLE.h"
#include "graphics.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

auto serviceUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x180C);
auto characteristicUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x2A56);

void test(std::vector<float> &vec)
{
    std::cout << "[";
    for (int i = 0; i < vec.size() - 1; i++) std::cout << vec[i] << ", ";
    std::cout << vec.back() << "]" << std::endl;
}

void AddData(GL& graphics, BLEDevice& sensor);

int main()
{
    init_apartment();

    float frame_limit = 200;
    BLEDevice BLE_Nano(serviceUUID, frame_limit);
    BLE_Nano.Connect();

    while (!BLE_Nano.is_connected) //Go into this loop until the BLE device is connected
    {  
    }

    //After BLE device is connected, allow data to collect for .5 seconds to make sure chip runs smoothly, also to establish initial Magnetic field vector
    std::chrono::time_point<std::chrono::system_clock> timer = std::chrono::system_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - timer).count() <= .5) {}
    BLE_Nano.UpdateData();
    BLE_Nano.SetMagField(BLE_Nano.mx, BLE_Nano.my, BLE_Nano.mz);

    Calibration Cal(&BLE_Nano);    

    //Set up OpenGL and Shaders
    GL GraphicWindow;
    GraphicWindow.LoadTexture("FXOS8700.jpg");
    GraphicWindow.AddText({ "Press Space to enter calibration mode", 520.0f, 10.0f, 0.33f, glm::vec3(1.0f, 1.0f, 1.0f) });
    
    //Connect Calibration class to graphic interface
    Cal.SetGraphics(&GraphicWindow);
    GraphicWindow.setCal(&Cal);
    BLE_Nano.SetPositionTimer();

    //Connect Calibration class to graphic interface
    Cal.SetGraphics(&GraphicWindow);
    GraphicWindow.setCal(&Cal);
    BLE_Nano.SetPositionTimer();

    //Disable V-Sync so that frames aren't limited by display
    glfwSwapInterval(0);

    while (!GraphicWindow.ShouldClose())
    {
        double yooo = glfwGetTime();
        GraphicWindow.processInput();

        //check to see if sensor location needs to be reset
        if (GraphicWindow.reset_location == 1)
        {
            GraphicWindow.reset_location = 0;
            BLE_Nano.ResetPosition();
        }

        //Get current data from chip and update rotation quaternion
        BLE_Nano.UpdateData();
        //LSM9DS1.Madgwick();
        BLE_Nano.Floyd();
        BLE_Nano.UpdatePosition();

        if (GraphicWindow.record_data == 1) AddData(GraphicWindow, BLE_Nano);
        if (GraphicWindow.display_graph == 1)
        {
            //BLE_Nano.SendData(2); //let the sensor know to stop sending data while graph is being viewed
            GraphicWindow.DisplayGraph();
            GraphicWindow.display_graph = 0;
            //BLE_Nano.SendData(2); //let the sensor know to start sending data again
        }

        if (GraphicWindow.display_readings) GraphicWindow.LiveUpdate(BLE_Nano.ax, BLE_Nano.ay, BLE_Nano.az, BLE_Nano.gx, BLE_Nano.gy, BLE_Nano.gz, BLE_Nano.mx, BLE_Nano.my, BLE_Nano.mz, BLE_Nano.lin_ax, BLE_Nano.lin_ay, BLE_Nano.lin_az, BLE_Nano.vel_x, BLE_Nano.vel_y, BLE_Nano.vel_z, BLE_Nano.loc_x, BLE_Nano.loc_y, BLE_Nano.loc_z);

        GraphicWindow.SetClubMatrices({ 1.0, 1.0, 1.0 }, { BLE_Nano.loc_x, BLE_Nano.loc_y, BLE_Nano.loc_z });
        GraphicWindow.RenderClub(BLE_Nano.GetRotationQuaternion());
        GraphicWindow.RenderText();
        GraphicWindow.Swap();

        while (glfwGetTime() - yooo < (1.0 / frame_limit)) continue;
    }

    GraphicWindow.DeleteBuffers();
    GraphicWindow.Terminate();

    return 0;
}

void AddData(GL& graphics, BLEDevice& sensor)
{
    if (graphics.current_display == 0) graphics.AddData(sensor.ax, sensor.ay, sensor.az);
    else if (graphics.current_display == 1) graphics.AddData(sensor.gx, sensor.gy, sensor.gz);
    else if (graphics.current_display == 2) graphics.AddData(sensor.mx, sensor.my, sensor.mz);
    else if (graphics.current_display == 3) graphics.AddData(sensor.lin_ax, sensor.lin_ay, sensor.lin_az);
    else if (graphics.current_display == 4) graphics.AddData(sensor.vel_x, sensor.vel_y, sensor.vel_z);
    else if (graphics.current_display == 5) graphics.AddData(sensor.loc_x, sensor.loc_y, sensor.loc_z);
}
