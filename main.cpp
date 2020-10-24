#include "pch.h"

#include <iostream>

#include <Header_Files/BluetoothLE.h>
#include <Header_Files/graphics.h>
#include <Header_Files/model.h>

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
    BLE_Nano.connect();
    while (BLE_Nano.data_available == false) //wait until chip has connected and started to gather data before moving onto the next step
    {
    }

    BLE_Nano.setMagField();

    //Set up calibration
    Calibration Cal(&BLE_Nano);

    //Set up OpenGL and Shaders
    GL GraphicWindow(&BLE_Nano);
    GraphicWindow.LoadTexture("FXOS8700.jpg");
    GraphicWindow.AddText(MessageType::FOOT_NOTE, { "Press Space to enter calibration mode.", 520.0f, 10.0f, 0.33f, glm::vec3(1.0f, 1.0f, 1.0f), GraphicWindow.getScreenWidth() });

    //Connect Calibration class to graphic interface
    Cal.SetGraphics(&GraphicWindow);
    GraphicWindow.setCal(&Cal);

    //glfwSwapInterval(0); //If this is uncommented it will disable v-sync, allowing OpenGl to update faster than the monitor refresh rate (60 Hz in this case)

    //Main rendering loop
    while (!GraphicWindow.ShouldClose())
    {
        //All updates for sensor data are taking place behing the scenes, values are updated when new data comes in from a concurrent function
        //That's why (for now) the only thing in this loop has to do with the graphic window
        GraphicWindow.Update(); //Check for input, check for render data, etc.
        GraphicWindow.SetClubMatrices({ 1.0, 1.0, 1.0 }, BLE_Nano.getLocation()); //set scale and translation matrices for render, rotation matrix is handled by Madgwick filter separately
        GraphicWindow.Render(BLE_Nano.getOpenGLQuaternion()); //Set matrices, render image, render text, etc.
    }

    GraphicWindow.Terminate();

    return 0;
}
