#include <ArduinoBLE.h>
#include <Wire.h>
#include "Bobby_Adafruit_FXOS8700.h"
#include "Bobby_Adafruit_FXAS21002C.h"
#include "Bobby_Adafruit_Sensor.h"

/* Assign a unique ID to the sensors at the same time */
Adafruit_FXOS8700 accelmag = Adafruit_FXOS8700(0x8700A, 0x8700B);
Adafruit_FXAS21002C gyro = Adafruit_FXAS21002C(0x0021002C);

/* Create a timer just in case */
int32_t timestamp;

/* Create array to store all data*/
int16_t data[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

/* Create new BLE Service and Characteristic */
BLEService DataService("180C");
BLECharacteristic RawDataCharacteristic("2A58", BLERead | BLENotify, 18); //There are 9 pieces of data each at 2 bytes, so characteristic size is 18 bytes

void displaySensorDetails(void)
{
  sensor_t accel, mag, gyr;
  accelmag.getSensor(&accel, &mag);
  gyro.getSensor(&gyr);
  Serial.println("------------------------------------");
  Serial.println("ACCELEROMETER");
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(accel.name);
  Serial.print("Driver Ver:   ");
  Serial.println(accel.version);
  Serial.print("Unique ID:    0x");
  Serial.println(accel.sensor_id, HEX);
  Serial.print("Min Delay:    ");
  Serial.print(accel.min_delay);
  Serial.println(" s");
  Serial.print("Max Value:    ");
  Serial.print(accel.max_value, 4);
  Serial.println(" m/s^2");
  Serial.print("Min Value:    ");
  Serial.print(accel.min_value, 4);
  Serial.println(" m/s^2");
  Serial.print("Resolution:   ");
  Serial.print(accel.resolution, 8);
  Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");

  Serial.println("------------------------------------");
  Serial.println("GYROSCOPE");
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(gyr.name);
  Serial.print("Driver Ver:   ");
  Serial.println(gyr.version);
  Serial.print("Unique ID:    0x");
  Serial.println(gyr.sensor_id, HEX);
  Serial.print("Max Value:    ");
  Serial.print(gyr.max_value);
  Serial.println(" rad/s");
  Serial.print("Min Value:    ");
  Serial.print(gyr.min_value);
  Serial.println(" rad/s");
  Serial.print("Resolution:   ");
  Serial.print(gyr.resolution);
  Serial.println(" rad/s");
  Serial.println("------------------------------------");
  Serial.println("");
  
  Serial.println("------------------------------------");
  Serial.println("MAGNETOMETER");
  Serial.println("------------------------------------");
  Serial.print("Sensor:       ");
  Serial.println(mag.name);
  Serial.print("Driver Ver:   ");
  Serial.println(mag.version);
  Serial.print("Unique ID:    0x");
  Serial.println(mag.sensor_id, HEX);
  Serial.print("Min Delay:    ");
  Serial.print(accel.min_delay);
  Serial.println(" s");
  Serial.print("Max Value:    ");
  Serial.print(mag.max_value);
  Serial.println(" uT");
  Serial.print("Min Value:    ");
  Serial.print(mag.min_value);
  Serial.println(" uT");
  Serial.print("Resolution:   ");
  Serial.print(mag.resolution);
  Serial.println(" uT");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void setup(void)
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); //Setup onboard LED to show when BLE connection is succesful

  /* Wait for the Serial Monitor */
  //while (!Serial) delay(1);

  Serial.println("FXOS8700 Test");
  Serial.println("");

  /* Initialise the accelerometer and magnetometer */
  if (!accelmag.begin(ACCEL_RANGE_4G))
  {
    /* There was a problem detecting the FXOS8700 ... check your connections */
    Serial.println("Ooops, no FXOS8700 detected ... Check your wiring!");
    while (1);
  }

  /* Initialise the gyroscope */
  if (!gyro.begin(GYRO_RANGE_500DPS))
  {
    /* There was a problem detecting the FXAS21002C ... check your connections */
    Serial.println("Ooops, no FXAS21002C detected ... Check your wiring!");
    while (1);
  }

  /* Initialise the BLE chip */
  if (!BLE.begin()) //initialize BLE and go to infinite loop if fail
  {
    Serial.println("Starting BLE Failed!");
    while(1);
  }
  else Serial.println("Starting BLE Succeeded!");

  /* Display some basic information on this sensor */
  displaySensorDetails();

  /* Initialize user made BLE services and characteristics */
  BLE.setLocalName("Nano33BLE");
  BLE.setAdvertisedService(DataService);
  DataService.addCharacteristic(RawDataCharacteristic);
  BLE.addService(DataService);
  RawDataCharacteristic.writeValue(data, 18);
}

void loop(void)
{
  BLE.advertise(); //start advertising
  bool maintain_connection = 1;
  while (maintain_connection)
  {
    BLEDevice central = BLE.central(); //Wait for BLE central to connect
    if (central)
    {
      Serial.println("The computer has connected to the BLE 33 Nano");
      timestamp = millis(); //start a timer
        
      digitalWrite(LED_BUILTIN, HIGH); //turn on the yellow LED to indicate a connection has been made
      while (central.connected()) //keep looping while connected
      {
        if (accelmag.getRawData(&data[0]))
        {
          if (gyro.getRawData(&data[0]))
          {
            RawDataCharacteristic.writeValue(data, 18);
            timestamp = millis();
          }
        }
      }
      
      //when the central disconnects, turn off the LED:
      Serial.println("The computer has disconnected from the BLE 33 Nano\n");
      digitalWrite(LED_BUILTIN, LOW);
      maintain_connection = 0;
    }
  }
}

/*
 Print information to be used if necessary
      //With an ODR of 400 it should take roughly 3ms to get each new reading
      Serial.print("A ");
      Serial.print("X: ");
      Serial.print(data[0] * 0.000488 * 9.80665);
      Serial.print("  ");
      Serial.print("Y: ");
      Serial.print(data[1] * 0.000488 * 9.80665);
      Serial.print("  ");
      Serial.print("Z: ");
      Serial.print(data[2] * 0.000488 * 9.80665);
      Serial.print("  ");
      Serial.println("deg/s ");
      Serial.print("Took ");
      Serial.print(millis() - timestamp);
      Serial.println(" milliseconds to read data.");
      Serial.println("");
 */
