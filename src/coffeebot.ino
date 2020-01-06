//initialize OLED
#include "Adafruit_SSD1306.h"
#define OLED_I2C_ADDRESS 0x3C
//set pin positions
#define OLED_RESET D4
#define SOLENOID 3
int FLOAT_SWITCH = D6;
Adafruit_SSD1306 display(OLED_RESET);
// refactor without byte?
byte SENSOR_INTERRUPT = 4;
byte SENSOR_PIN = 4;
// declare variables
bool fill = false;
bool debugMode = false;
double calibrationFactor = 32.25;
float flowRate;
float flowRateOunces;
float filledOunces = 0;
double requestedOunces = 0;
long lastFlowReadingTimestamp;
volatile int pulseCount;
int floatState;
String requestedCups;
// begin inital setup
void setup()
{
  //Serial.begin(9600) ;
  pinMode(SENSOR_PIN, INPUT);
  pinMode(SOLENOID, OUTPUT);
  pinMode(FLOAT_SWITCH, INPUT_PULLUP);
  digitalWrite(SENSOR_PIN, HIGH);
  attachInterrupt(SENSOR_INTERRUPT, pulseCounter, FALLING);
  Time.zone(-4);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  display.setRotation(2);
  clearScreen();
  //cloud functions
  Particle.function("Calibrate", Calibrate);
  Particle.function("Stop", Stop);
  Particle.function("FillWater", FillWater);
  Particle.function("ToggleDebugMode", ToggleDebugMode);
  //cloud variables
  Particle.variable("calibrationFactor", calibrationFactor);
  Particle.variable("FloatState", floatState);
}
void loop()
{
  floatState = digitalRead(FLOAT_SWITCH);
  if (floatState == HIGH)
  {
    stopFilling();
    errorMessage("Tank is", "full");
  }
  else if (fill && (millis() - lastFlowReadingTimestamp) > 1000)
  {
    detachInterrupt(SENSOR_INTERRUPT);
    calculateFlow();
    attachInterrupt(SENSOR_INTERRUPT, pulseCounter, FALLING);
    if (floatState == LOW && filledOunces < requestedOunces)
    {
      digitalWrite(SOLENOID, HIGH);
      if (debugMode)
      {
        debugMessage();
      }
      else
      {
        fillingMessage();
        statusBar(filledOunces, requestedOunces);
      }
    }
    else if (filledOunces >= requestedOunces)
    {
      filledMessage();
      stopFilling();
    }
  }
  else if (!fill)
  {
    digitalWrite(SOLENOID, LOW); //Switch Solenoid OFF
  }
}
//LCD functions
void showMsg(int position, int font, String message)
{
  display.setTextSize(font); // 1 = 8 pixel tall, 2 = 16 pixel tall...
  display.setTextColor(WHITE);
  display.setCursor(0, position);
  display.println(message);
  display.display();
}
void statusBar(float filledOunces, float requestedOunces)
{
  int pixels = (filledOunces / requestedOunces) * 128;
  display.drawRect(0, 48, 128, 16, WHITE);
  display.fillRect(1, 49, pixels, 14, WHITE);
  display.fillRect(1, 49, pixels - 6, 14, BLACK);
  display.display();
}
void clearScreen()
{
  display.clearDisplay();
  display.display();
}
void errorMessage(String line1, String line2)
{
  clearScreen();
  showMsg(0, 3, "Error:");
  showMsg(30, 2, line1);
  showMsg(48, 2, line2);
  delay(3000);
}
void filledMessage()
{
  clearScreen();
  String fillDate = Time.format(Time.now(), "%m-%d-%y");
  String fillTime = Time.format(Time.now(), "%I:%M %p");
  showMsg(0, 2, "Filled");
  showMsg(20, 2, requestedCups + " Cups");
  showMsg(40, 1, "Last Filled on:");
  showMsg(50, 1, fillDate + " at " + fillTime);
}
void fillingMessage()
{
  showMsg(0, 2, "Pouring");
  showMsg(16, 2, requestedCups + " Cups...");
}
void debugMessage()
{
  clearScreen();
  showMsg(0, 1, "Flow Rate:" + String(flowRateOunces));
  showMsg(10, 1, "RequestOzs:" + String(requestedOunces));
  showMsg(20, 1, "Filled Ozs:" + String(filledOunces));
  showMsg(30, 1, "Pulse Count:" + String(pulseCount));
  showMsg(50, 1, "DEBUG MODE ENABLED");
}
//flow meter functions
void pulseCounter()
{
  pulseCount++;
}
void calculateFlow()
{
  flowRate = ((1000.0 / (millis() - lastFlowReadingTimestamp)) * pulseCount) / calibrationFactor;
  lastFlowReadingTimestamp = millis();
  flowRateOunces = (flowRate / 60.0) * 1000.0 * 0.033814;
  filledOunces += flowRateOunces;
  pulseCount = 0;
}
//control-flow functions
void resetVariables()
{
  requestedOunces = 0.0;
  filledOunces = 0.0;
  fill = false;
}
void stopFilling()
{
  digitalWrite(SOLENOID, LOW); //Switch Solenoid OFF
  resetVariables();
}
//cloud functions (pascalcase)
int Calibrate(String message)
{
  calibrationFactor = message.toFloat();
}
int Stop(String message)
{
  stopFilling();
}
int FillWater(String message)
{
  if (!fill)
  {
    clearScreen();
    requestedCups = message;
    requestedOunces = message.toInt() * 5;
    fill = true;
  }
}
int ToggleDebugMode(String message)
{
  if (!debugMode)
  {
    debugMode = true;
  }
  else
  {
    debugMode = false;
  }
}
