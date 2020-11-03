#include "pch.h"

#include <iostream>

#include <Header_Files/BluetoothLE.h>
#include <Header_Files/graphics.h>
#include <Header_Files/Modes/modes.h>

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

    /*
    BLE_Nano.connect();
    while (BLE_Nano.data_available == false) //wait until chip has connected and started to gather data before moving onto the next step
    {
    }
    */
    BLE_Nano.setMagField();

    //Set up OpenGL and Shaders
    GL GraphicWindow(&BLE_Nano);

    //Add all proper modes to the Graphic Interface
    MainMenu mm(GraphicWindow); GraphicWindow.addMode(&mm);
    FreeSwing fs(GraphicWindow); GraphicWindow.addMode(&fs);
    Calibration cc(GraphicWindow); GraphicWindow.addMode(&cc);
    Training tt(GraphicWindow); GraphicWindow.addMode(&tt);

    GraphicWindow.setCurrentMode(ModeType::MAIN_MENU); //start off with the main menu, ultimately want to move this Mode setup into the graphic intialization

    //Main rendering loop
    while (!GraphicWindow.ShouldClose())
    {
        //All updates for sensor data are taking place behing the scenes, values are updated when new data comes in from a concurrent function
        //That's why (for now) the only thing in this loop has to do with the graphic window

        GraphicWindow.masterUpdate();
        GraphicWindow.masterRender();
    }

    GraphicWindow.Terminate();

    return 0;
}