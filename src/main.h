void checkIfShouldWatering(unsigned long currentMillis);
void runPumps(unsigned long currentMillis);
void runPump(int pin, int enable);
int measureHumidity(int pin);
HumidityMeasurements measureHumidityForAllSensors();