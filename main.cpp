#include "pch.h"

#include <iostream>

#include <Header_Files/BluetoothLE.h>
#include <Header_Files/graphics.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices;
using namespace Bluetooth::GenericAttributeProfile;

auto serviceUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x180C);
auto characteristicUUID = Bluetooth::BluetoothUuidHelper::FromShortId(0x2A56);

int main()
{
    init_apartment();
     
    //Create BLE device and wait for it to connect to actual BLE device
    float sensor_refresh_rate = 400; //chip is set to gather all forms of data at 400Hz
    BLEDevice BLE_Nano(serviceUUID, sensor_refresh_rate);
    BLE_Nano.Connect();
    while (BLE_Nano.data_available == false) //wait until chip has connected and started to gather data before moving onto the next step
    {
    }

    BLE_Nano.SetMagField();

    //Set up calibration
    Calibration Cal(&BLE_Nano);

    //Set up OpenGL and Shaders
    GL GraphicWindow(&BLE_Nano);
    GraphicWindow.LoadTexture("FXOS8700.jpg");
    GraphicWindow.AddText({ "Press Space to enter calibration mode", 520.0f, 10.0f, 0.33f, glm::vec3(1.0f, 1.0f, 1.0f) });

    //Connect Calibration class to graphic interface
    Cal.SetGraphics(&GraphicWindow);
    GraphicWindow.setCal(&Cal);
    BLE_Nano.SetPositionTimer();

    //Disable V-Sync so that frames aren't limited by display
    glfwSwapInterval(0);
    double loop_timer = glfwGetTime();

    //for (int i = 1; i < test_times.size(); i++)
    while (!GraphicWindow.ShouldClose())
    {
        GraphicWindow.Update(); //Check for input, check for render data, etc.
        GraphicWindow.Render(); //Set matrices, render image, render text, etc.

        BLE_Nano.Update(); //Read any new sensor data and update current chip info

        //Add in a hard wait time before next loop iteration that is equal to 2/3 of the connection interval with the BLE chip divided by the number of samples
        while ((glfwGetTime() - loop_timer) * 1000 < 2.4) //2.4 milliseconds has been calculated by hand here, but put in a hard calculation in the code at some point
        {
        }
        loop_timer = glfwGetTime();
    }

    GraphicWindow.DeleteBuffers();
    GraphicWindow.Terminate();

    return 0;
}
