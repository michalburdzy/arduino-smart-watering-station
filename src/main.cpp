#include <Arduino.h>

#define MOISTURE_SENSOR_PIN_1 A1
#define MOISTURE_SENSOR_PIN_2 A2
#define MOISTURE_SENSOR_PIN_3 A3
#define MOISTURE_SENSOR_PIN_4 A4
#define PUMP_1_PIN 11
#define PUMP_2_PIN 12

enum SoilHumidity
{
  ARID,
  DRY,
  MEDIUM,
  HUMID,
  WET,
  INCORRECT_SENSOR_READ,
};

struct HumidityMeasurements
{
  SoilHumidity sensor1Value;
  SoilHumidity sensor2Value;
  SoilHumidity sensor3Value;
  SoilHumidity sensor4Value;
};

struct WateringStationConfig
{
  unsigned long lastWateringTime;
  unsigned long wateringTimeout;
  unsigned long pumpWateringTimeMillis;
  unsigned long pump1RunUntil;
  unsigned long pump2RunUntil;
};

WateringStationConfig config =
    {
        0,
        1000UL * 5, // 5s
        1000UL * 3, // 3s
        0,
        0,
};

void checkIfShouldWatering(unsigned long currentMillis);
void runPumps(unsigned long currentMillis);
void runPump(int pin, int enable);
int measureHumidity(int pin);
HumidityMeasurements measureHumidityForAllSensors();
SoilHumidity mapIntToSoliHumidity(int value);
bool isOneOfHumidityValuesDry(SoilHumidity value1, SoilHumidity value2);

void setup()
{
  Serial.begin(9600);

  pinMode(MOISTURE_SENSOR_PIN_1, INPUT);
  pinMode(MOISTURE_SENSOR_PIN_2, INPUT);
  pinMode(MOISTURE_SENSOR_PIN_3, INPUT);
  pinMode(MOISTURE_SENSOR_PIN_4, INPUT);
  pinMode(PUMP_1_PIN, OUTPUT);
  pinMode(PUMP_2_PIN, OUTPUT);
}

void loop()
{
  unsigned long currentMillis = millis();

  checkIfShouldWatering(currentMillis);

  runPumps(currentMillis);
}

void checkIfShouldWatering(unsigned long currentMillis)
{
  bool isWateringTime = currentMillis > config.lastWateringTime + config.wateringTimeout;
  if (isWateringTime)
  {
    HumidityMeasurements humiditySensorReads = measureHumidityForAllSensors();

    unsigned long runPumpUntil = currentMillis + config.pumpWateringTimeMillis;
    bool shouldRunPump1 = isOneOfHumidityValuesDry(humiditySensorReads.sensor1Value, humiditySensorReads.sensor2Value);
    bool shouldRunPump2 = isOneOfHumidityValuesDry(humiditySensorReads.sensor3Value, humiditySensorReads.sensor4Value);

    if (shouldRunPump1)
    {
      Serial.println("Running pump 1");
      config.pump1RunUntil = runPumpUntil;
    }

    if (shouldRunPump2)
    {
      Serial.println("Running pump 2");
      config.pump2RunUntil = runPumpUntil;
    }

    config.lastWateringTime = currentMillis + config.wateringTimeout;
  }
}

bool isOneOfHumidityValuesDry(SoilHumidity value1, SoilHumidity value2)
{
  return value1 == ARID || value1 == DRY || value2 == ARID || value2 == DRY;
}

void runPumps(unsigned long currentMillis)
{
  if (config.pump1RunUntil >= currentMillis)
  {
    runPump(PUMP_1_PIN, HIGH);
  }
  else
  {
    runPump(PUMP_1_PIN, LOW);
  }

  if (config.pump2RunUntil >= currentMillis)
  {
    runPump(PUMP_2_PIN, HIGH);
  }
  else
  {
    runPump(PUMP_2_PIN, LOW);
  }
}

void runPump(int pin, int enable)
{
  digitalWrite(pin, enable);
}

int measureHumidity(int pin)
{
  int sum = 0;
  int iterations = 10;
  for (int i = 0; i < iterations; i++)
  {
    sum = sum + analogRead(pin);
  }

  return round(sum / iterations);
}

HumidityMeasurements measureHumidityForAllSensors()
{

  HumidityMeasurements humiditySensorReads = {
      mapIntToSoliHumidity(measureHumidity(MOISTURE_SENSOR_PIN_1)),
      mapIntToSoliHumidity(measureHumidity(MOISTURE_SENSOR_PIN_2)),
      mapIntToSoliHumidity(measureHumidity(MOISTURE_SENSOR_PIN_3)),
      mapIntToSoliHumidity(measureHumidity(MOISTURE_SENSOR_PIN_4)),
  };

  Serial.print("humidity sensor 1: ");
  Serial.println(humiditySensorReads.sensor1Value);
  Serial.print("humidity sensor 2: ");
  Serial.println(humiditySensorReads.sensor2Value);
  Serial.print("humidity sensor 3: ");
  Serial.println(humiditySensorReads.sensor3Value);
  Serial.print("humidity sensor 4: ");
  Serial.println(humiditySensorReads.sensor4Value);

  return humiditySensorReads;
};

SoilHumidity mapIntToSoliHumidity(int value)
{
  // woda 300-320
  // mokra ziemia 350-360
  // sucha ziemia 880-1013

  if (value < 350)
  {
    return WET;
  }
  if (value < 500)
  {
    return HUMID;
  }

  if (value < 700)
  {
    return MEDIUM;
  }

  if (value < 900)
  {
    return DRY;
  }

  if (value < 1024)
  {
    return ARID;
  }

  return INCORRECT_SENSOR_READ;
};