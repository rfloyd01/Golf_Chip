#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

BLEService MyService("180C");
BLECharacteristic TestCharacteristic("2A58", BLERead | BLENotify, 12);

char acceleration[12];
float ax, ay, az;

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

  if (!IMU.begin()) //initialize IMU and go to infinite loop if fail
  {
    Serial.println("Starting IMU Failed!");
    while(1);
  }

  BLE.setLocalName("Nano33BLE");
  BLE.setAdvertisedService(MyService);
  MyService.addCharacteristic(TestCharacteristic);
  
  BLE.addService(MyService);

  TestCharacteristic.setValue(&acceleration[0]);
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
        //greetingCharacteristic.broadcast(); //start broadcasting value for this LED
        Serial.println("The computer has connected to the BLE 33 Nano");
        timer = millis(); //start a timer
        
        digitalWrite(LED_BUILTIN, HIGH); //turn on the yellow LED to indicate a connection has been made
        while (central.connected()) //keep looping while connected
        {
          if (IMU.accelerationAvailable())
          {
            IMU.readAcceleration(ax, ay, az);

            ax *= 9.81;
            ay *= 9.81;
            az *= 9.81;

            //Serial.print("Ax = "); Serial.println(ax);
            //Serial.print("Ay = "); Serial.println(ay);
            //Serial.print("Az = "); Serial.println(az);
            //Serial.println("");
            
            char* p = reinterpret_cast<char*>(&ax); //set ax
            for (int i = 0; i < 4; i++) acceleration[i] = p[i];

            p = reinterpret_cast<char*>(&ay); //set ay
            for (int i = 0; i < 4; i++) acceleration[i+4] = p[i];

            p = reinterpret_cast<char*>(&az); //set az
            for (int i = 0; i < 4; i++) acceleration[i+8] = p[i];
            
            timer = millis();
            TestCharacteristic.setValue(&acceleration[0]);
            //Serial.print("Function ran in = "); Serial.print(millis() - timer); Serial.println(" milliseconds.");
            Serial.println("The characteristic has been updated.");
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
