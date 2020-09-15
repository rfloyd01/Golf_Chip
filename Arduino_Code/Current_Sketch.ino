#include <ArduinoBLE.h>
#include "Bobby_LSM9DS1.h"

BLEService MyService("180C");
BLECharacteristic TestCharacteristic("2A58", BLERead | BLENotify, 18);

int16_t data[9]; //Order of data is ax, ay, az, gy, gx, gz, mx, my and mz
unsigned long timer;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  if (!BLE.begin()) //initialize BLE and go to infinite loop if fail
  {
    Serial.println("Starting BLE Failed!");
    while(1);
  }
  else Serial.println("Starting BLE Succeeded!");

  if (!IMU.begin()) //initialize IMU and go to infinite loop if fail
  {
    Serial.println("Starting IMU Failed!");
    while(1);
  }
  else Serial.println("Starting IMU Succeeded!");

  BLE.setLocalName("Nano33BLE");
  BLE.setAdvertisedService(MyService);
  MyService.addCharacteristic(TestCharacteristic);
  
  BLE.addService(MyService);
  IMU.setContinuousMode(); //allow for simultaneous reading of accelerometer and gyroscope

  TestCharacteristic.writeValue(data, 18);
}

void loop()
{
  BLE.advertise(); //start advertising
  bool maintain_connection = 1;
  while (maintain_connection)
  {
      BLEDevice central = BLE.central(); //Wait for BLE central to connect
      
      //if a central is connected to the peripheral:
      if (central)
      {
        Serial.println("The computer has connected to the BLE 33 Nano");
        timer = millis(); //start a timer
        
        digitalWrite(LED_BUILTIN, HIGH); //turn on the yellow LED to indicate a connection has been made
        while (central.connected()) //keep looping while connected
        {
          if (IMU.accelerationAvailable())
          {
            IMU.readData(&data[0]);

            //Serial.print("Ax = "); Serial.println(data[0] * 4.0 * 9.81 / 32768.0);
            //Serial.print("Ay = "); Serial.println(data[1] * 4.0 * 9.81 / 32768.0);
            //Serial.print("Az = "); Serial.println(data[2] * 4.0 * 9.81 / 32768.0);
            //Serial.print("Gx = "); Serial.println(data[3] * 2000.0 / 32768.0);
            //Serial.print("Gy = "); Serial.println(data[4] * 2000.0 / 32768.0);
            //Serial.print("Gz = "); Serial.println(data[5] * 2000.0 / 32768.0);
            //Serial.print("Mx = "); Serial.println(data[6] * 4.0 * 100.0 / 32768.0);
            //Serial.print("My = "); Serial.println(data[7] * 4.0 * 100.0 / 32768.0);
            //Serial.print("Mz = "); Serial.println(data[8] * 4.0 * 100.0 / 32768.0);
            //Serial.println();
            
            timer = millis();
            TestCharacteristic.writeValue(data, 18);
            //delay(500); //adding a short delay here may help things run smoothly
          }
        }

        //when the central disconnects, turn off the LED:
        Serial.println("The computer has disconnected from the BLE 33 Nano\n");
        digitalWrite(LED_BUILTIN, LOW);
        maintain_connection = 0;
      }
  }
}
