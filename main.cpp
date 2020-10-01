#include "pch.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <bitset>
#include <stdlib.h> 

#include <Header_Files/BluetoothLE.h>
#include <Header_Files/graphics.h>
#include <Header_Files/quaternion_functions.h>

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
class MatLabMadgwick
{
public:
    MatLabMadgwick(float sp, float b);
    void Update(float* gyro, float* acc, float* mag);
    glm::quat getCurrentQuaternion();

private:
    float sample_period, beta;
    glm::quat Quaternion;
};

int main()
{
    init_apartment();

    float* p_float;
    float hh[3] = {3, 4, 2};
    float yy[3][4] = { {13, 9, 7, 15}, {8, 7, 4, 6}, {6, 4, 0, 3} };
    float yeet[4];

    matrixMultiply(&hh[0], 1, 3, &yy[0][0], 3, 4, &yeet[0]);
    
    for (int i = 0; i < 4; i++)
    {
        std::cout << yeet[i] << " ";
    }
    std::cout << std::endl;

    float frame_limit = 60;
    BLEDevice BLE_Nano(serviceUUID, frame_limit);
    BLE_Nano.Connect();

    while (!BLE_Nano.is_connected) //Go into this loop until the BLE device is connected
    {  
    }

    //After BLE device is connected, allow data to collect for .5 seconds to make sure chip runs smoothly, also to establish initial Magnetic field vector
    std::chrono::time_point<std::chrono::system_clock> timer = std::chrono::system_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - timer).count() <= .5) {}
    BLE_Nano.UpdateData();
    BLE_Nano.SetMagField();

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
    double frame_timer = glfwGetTime(), loop_timer = glfwGetTime();

    while (!GraphicWindow.ShouldClose())
    {
        GraphicWindow.processInput();

        //check to see if sensor location needs to be reset
        if (GraphicWindow.reset_location == 1)
        {
            GraphicWindow.reset_location = 0;
            BLE_Nano.ResetPosition();
        }

        //Get current data from chip and update rotation quaternion
        //BLE_Nano.UpdateData(); UpdateData should get called automatically everytime new data from chip arrives, shouldn't need to be called explicitly here

        BLE_Nano.SetSampleFrequency(1.0 / (glfwGetTime() - loop_timer)); loop_timer = glfwGetTime(); //need to set the sample frequency every loop to get real time measurements
        BLE_Nano.MadgwickIMU();
        //BLE_Nano.Madgwick();
        //BLE_Nano.MadgwickModified();
        //BLE_Nano.Floyd();
        //BLE_Nano.UpdatePosition();

        if (GraphicWindow.record_data == 1) AddData(GraphicWindow, BLE_Nano);
        if (GraphicWindow.display_graph == 1)
        {
            //BLE_Nano.SendData(2); //let the sensor know to stop sending data while graph is being viewed
            GraphicWindow.DisplayGraph();
            GraphicWindow.display_graph = 0;
            //BLE_Nano.SendData(2); //let the sensor know to start sending data again
        }

        if (GraphicWindow.display_readings)
        {
            int sc = BLE_Nano.current_sample;
            GraphicWindow.LiveUpdate(BLE_Nano.ax[sc], BLE_Nano.ay[sc], BLE_Nano.az[sc], BLE_Nano.gx[sc], BLE_Nano.gy[sc], BLE_Nano.gz[sc], BLE_Nano.mx[sc], BLE_Nano.my[sc], BLE_Nano.mz[sc], BLE_Nano.lin_ax, BLE_Nano.lin_ay, BLE_Nano.lin_az, BLE_Nano.vel_x, BLE_Nano.vel_y, BLE_Nano.vel_z, BLE_Nano.loc_x, BLE_Nano.loc_y, BLE_Nano.loc_z);
        }

        //if (glfwGetTime() - frame_timer < (1.0 / frame_limit))
        if (1)
        {
            GraphicWindow.SetClubMatrices({ 1.0, 1.0, 1.0 }, { BLE_Nano.loc_x, BLE_Nano.loc_y, BLE_Nano.loc_z });
            GraphicWindow.RenderClub(BLE_Nano.GetRotationQuaternion());
            GraphicWindow.RenderText();
            GraphicWindow.Swap();
            frame_timer = glfwGetTime();
        }

        //Add in a hard wait time before next loop iteration that is equal to 2/3 of the connection interval with the BLE chip divided by the number of samples
        while ((glfwGetTime() - loop_timer) * 1000 < 2.4) //2.4 milliseconds has been calculated by hand here, but put in a hard calculation in the code at some point
        { }
    }

    GraphicWindow.DeleteBuffers();
    GraphicWindow.Terminate();

    return 0;
}

void AddData(GL& graphics, BLEDevice& sensor)
{
    int sc = sensor.current_sample;
    if (graphics.current_display == 0) graphics.AddData(sensor.ax[sc], sensor.ay[sc], sensor.az[sc]);
    else if (graphics.current_display == 1) graphics.AddData(sensor.gx[sc], sensor.gy[sc], sensor.gz[sc]);
    else if (graphics.current_display == 2) graphics.AddData(sensor.mx[sc], sensor.my[sc], sensor.mz[sc]);
    else if (graphics.current_display == 3) graphics.AddData(sensor.lin_ax, sensor.lin_ay, sensor.lin_az);
    else if (graphics.current_display == 4) graphics.AddData(sensor.vel_x, sensor.vel_y, sensor.vel_z);
    else if (graphics.current_display == 5) graphics.AddData(sensor.loc_x, sensor.loc_y, sensor.loc_z);
}

MatLabMadgwick::MatLabMadgwick(float sp, float b)
{
    sample_period = sp;
    beta = b;
}

void MatLabMadgwick::Update(float* gyro, float* acc, float* mag)
{
    std::vector<float> Gyroscope = { *gyro, *(gyro + 1), *(gyro + 2) };
    std::vector<float> Accelerometer = { *acc, *(acc + 1), *(acc + 2) };
    std::vector<float> Magnetometer = { *mag, *(mag + 1), *(mag + 2) };
    glm::quat q = Quaternion;

    if (Magnitude(Accelerometer) == 0) return; //break out if there are no accelerometer values to avoid division by zero
    if (Magnitude(Magnetometer) == 0) return; //break out if there are no magnetometer values to avoid division by zero

    //Normalize accelerometer and magnetometer measurements
    Normalize(Accelerometer);
    Normalize(Magnetometer);

    //Calculate the reference direction of Earth's magnetic field, b
    glm::quat h = QuaternionMultiply(q, QuaternionMultiply({ 0, Magnetometer[0], Magnetometer[1], Magnetometer[2] }, Conjugate(q)));
    glm::quat b = { 0, Magnitude({h.x, h.y}), 0, h.z };

    //Gradient Descent Algorithm corrective step
    //q1, q2, q3, q4 == q.w, q.x, q.y, q.z
    float F[6][1];
    F[0][0] = 2 * (q.x * q.z - q.w * q.y) - Accelerometer[0]; //2*(q(2)*q(4) - q(1)*q(3)) - Accelerometer(1)
    F[1][0] = 2 * (q.w * q.x + q.y * q.z) - Accelerometer[1]; //2*(q(1)*q(2) + q(3)*q(4)) - Accelerometer(2)
    F[2][0] = 2 * (0.5 - q.x * q.x - q.y * q.y) - Accelerometer[2]; //2*(0.5 - q(2)^2 - q(3)^2) - Accelerometer(3)
    F[3][0] = 2 * b.x * (0.5 - q.y * q.y - q.z * q.z) + 2 * b.z * (q.x * q.z - q.w * q.y) - Magnetometer[0]; //2*b(2)*(0.5 - q(3)^2 - q(4)^2) + 2*b(4)*(q(2)*q(4) - q(1)*q(3)) - Magnetometer(1)
    F[4][0] = 2 * b.x * (q.x * q.y - q.w * q.z) + 2 * b.z * (q.w * q.x + q.y * q.z) - Magnetometer[1]; //2*b(2)*(q(2)*q(3) - q(1)*q(4)) + 2*b(4)*(q(1)*q(2) + q(3)*q(4)) - Magnetometer(2)
    F[5][0] = 2 * b.x * (q.w * q.y + q.x * q.z) + 2 * b.z * (0.5 - q.x * q.x - q.y * q.y) - Magnetometer[2]; //2*b(2)*(q(1)*q(3) + q(2)*q(4)) + 2*b(4)*(0.5 - q(2)^2 - q(3)^2) - Magnetometer(3)]

    float J[6][4];
    J[0][0] = -2 * q.y; J[0][1] = 2 * q.z; J[0][2] = -2 * q.w; J[0][3] = 2 * q.x;
    J[1][0] = 2 * q.x;  J[1][1] = 2 * q.w; J[1][2] = -2 * q.z; J[1][3] = 2 * q.y;
    J[2][0] = 0;        J[2][1] = -4 * q.x; J[2][2] = -4 * q.y; J[2][3] = 0;

    J[3][0] = -2 * b.z * q.y; J[3][1] = 2 * b.z * q.z; J[3][2] = -4 * b.x * q.y - 2 * b.z * q.w; J[3][3] = -4 * b.x * q.z + 2 * b.z * q.x;
    J[4][0] = -2 * b.x * q.z + 2 * b.z * q.x; J[4][1] = 2 * b.x * q.y + 2 * b.z * q.w; J[4][2] = 2 * b.x * q.x + 2 * b.z * q.z; J[4][3] = -2 * b.x * q.w + 2 * b.z * q.y;
    J[5][0] = 2 * b.x * q.y; J[5][1] = 2 * b.x * q.z - 4 * b.z * q.x; J[5][2] = 2 * b.x * q.w - 4 * b.z * q.y; J[5][3] = 2 * b.x * q.x;

    //Transpose the J matrix
    float J_prime[4][6] = {
        {J[0][0], J[1][0], J[2][0], J[3][0], J[4][0], J[5][0]},
        {J[0][1], J[1][1], J[2][1], J[3][1], J[4][1], J[5][1]},
        {J[0][2], J[1][2], J[2][2], J[3][2], J[4][2], J[5][2]},
        {J[0][3], J[1][3], J[2][3], J[3][3], J[4][3], J[5][3]}
    };

    //step = J_prime * F
    float step[4][1];
    matrixMultiply(&J_prime[0][0], 4, 6, &F[0][0], 6, 1, &step[0][0]);   


}

glm::quat MatLabMadgwick::getCurrentQuaternion()
{
    return Quaternion;
}
