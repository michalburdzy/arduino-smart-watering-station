#include <Arduino.h>

#define MOISTURE_SENSOR_PIN_1 A1
#define MOISTURE_SENSOR_PIN_2 A2
#define MOISTURE_SENSOR_PIN_3 A3
#define MOISTURE_SENSOR_PIN_4 A4
#define PUMP_1_PIN 11
#define PUMP_2_PIN 12
#define HUMIDITY_THRESHOLD_LOW 400

struct HumidityMeasurements
{
  int sensor1Value;
  int sensor2Value;
  int sensor3Value;
  int sensor4Value;
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

void checkIfShouldWatering(unsigned long currentMillis);
void runPumps(unsigned long currentMillis);
void runPump(int pin, int enable);
int measureHumidity(int pin);
HumidityMeasurements measureHumidityForAllSensors();

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
    bool shouldRunPump1 = humiditySensorReads.sensor1Value < HUMIDITY_THRESHOLD_LOW && humiditySensorReads.sensor2Value < HUMIDITY_THRESHOLD_LOW;
    bool shouldRunPump2 = humiditySensorReads.sensor3Value < HUMIDITY_THRESHOLD_LOW && humiditySensorReads.sensor4Value < HUMIDITY_THRESHOLD_LOW;

    if (shouldRunPump1)
    {
      config.pump1RunUntil = runPumpUntil;
    }

    if (shouldRunPump2)
    {
      config.pump2RunUntil = runPumpUntil;
    }

    config.lastWateringTime = currentMillis + config.wateringTimeout;
  }
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
      measureHumidity(MOISTURE_SENSOR_PIN_1),
      measureHumidity(MOISTURE_SENSOR_PIN_2),
      measureHumidity(MOISTURE_SENSOR_PIN_3),
      measureHumidity(MOISTURE_SENSOR_PIN_4),
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