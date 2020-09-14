#include <ArduinoBLE.h>
#include "Bobby_LSM9DS1.h"

BLEService MyService("180C");
BLECharacteristic TestCharacteristic("2A58", BLERead | BLENotify, 18);

int16_t data[9]; //Order of data is ax, ay, az, gy, gx, gz, mx, my and mz
float ax = 1, ay = 1, az = 1;
float gx, gy, gz;
float mx, my, mz;

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

  TestCharacteristic.writeValue(data, 18);
}

void loop()
{
  BLE.advertise(); //start advertising
  bool maintain_connection = 1;
  while (maintain_connection)
  {
      BLEDevice central = BLE.central(); //Wait for BLE central to connect
      //Serial.print("Read "); Serial.print(TestCharacteristic.readValue(val, 2)); Serial.println(" bytes of data.");
      //Serial.print("Value of data read is "); Serial.println(val[0]); //returns the number of bytes of data read
      //Serial.println();
      
      //if a central is connected to the peripheral:
      if (central)
      {
        //greetingCharacteristic.broadcast(); //start broadcasting value for this LED
        Serial.println("The computer has connected to the BLE 33 Nano");
        timer = millis(); //start a timer
        
        digitalWrite(LED_BUILTIN, HIGH); //turn on the yellow LED to indicate a connection has been made
        while (central.connected()) //keep looping while connected
        {
          if (IMU.accelerationAvailable())
          {
            //IMU.readAcceleration(ax, ay, az);
            //IMU.readGyroscope(gx, gy, gz);
            //IMU.readMagneticField(mx, my, mz);

            IMU.readData(&data[0]);

            //ax *= 9.81;
            //ay *= 9.81;
            //az *= 9.81;

            Serial.print("Ax = "); Serial.println(data[0] * 4.0 * 9.81 / 32768.0);
            Serial.print("Ay = "); Serial.println(data[1] * 4.0 * 9.81 / 32768.0);
            Serial.print("Az = "); Serial.println(data[2] * 4.0 * 9.81 / 32768.0);
            Serial.print("Gx = "); Serial.println(data[3] * 2000.0 / 32768.0);
            Serial.print("Gy = "); Serial.println(data[4] * 2000.0 / 32768.0);
            Serial.print("Gz = "); Serial.println(data[5] * 2000.0 / 32768.0);
            Serial.print("Mx = "); Serial.println(data[6] * 4.0 * 100.0 / 32768.0);
            Serial.print("My = "); Serial.println(data[7] * 4.0 * 100.0 / 32768.0);
            Serial.print("Mz = "); Serial.println(data[8] * 4.0 * 100.0 / 32768.0);
            Serial.println();

            /*
            char* p = reinterpret_cast<char*>(&ax); //set ax
            for (int i = 0; i < 4; i++) acceleration[i] = p[i];

            p = reinterpret_cast<char*>(&ay); //set ay
            for (int i = 0; i < 4; i++) acceleration[i+4] = p[i];

            p = reinterpret_cast<char*>(&az); //set az
            for (int i = 0; i < 4; i++) acceleration[i+8] = p[i];
            */
            
            timer = millis();
            TestCharacteristic.writeValue(data, 18);
            
            //Serial.print("Function ran in = "); Serial.print(millis() - timer); Serial.println(" milliseconds.");
            //Serial.println("The characteristic has been updated.");
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
