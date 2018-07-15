#include "Adafruit_SSD1306.h"
#define OLED_I2C_ADDRESS 0x3C
#define OLED_RESET D4
#define SOLENOID 3
#define WATER_SENSOR A0
Adafruit_SSD1306 display(OLED_RESET) ;
// refactor without byte?
byte SENSOR_INTERRUPT = 4 ;  // digital pin 2
byte SENSOR_PIN       = 4 ;

// declare variables
bool fill = false ;
bool tankFull = false ;
bool debugMode = false ;
double calibrationFactor = 31 ;
float flowRate ;
float flowRateOunces;
float filledOunces ;
double requestedOunces = 0.01 ;
long lastFlowReadingTimestamp ;
volatile int pulseCount ;
int waterSensor ;
String requestedCups ;

//initialize
void setup() {
  //Serial.begin(9600) ;
  pinMode(SENSOR_PIN, INPUT) ;
  pinMode(SOLENOID, OUTPUT) ;
  digitalWrite(SENSOR_PIN, HIGH) ;
  attachInterrupt(SENSOR_INTERRUPT, pulseCounter, FALLING) ;
  Time.zone(-4) ;
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS) ;
  clearScreen(); //necessary?
  //cloud functions
  Particle.function("Calibrate", Calibrate) ;
  Particle.function("Stop", Stop) ;
  Particle.function("FillWater", FillWater) ;
  Particle.function("ManualFill", ManualFill) ;
  Particle.function("ToggleDebugMode", ToggleDebugMode) ;
  //cloud variables
  Particle.variable("calibrationFactor", calibrationFactor) ;
  Particle.variable("waterSensor", waterSensor) ;
}
void loop() {
  //Serial.println(waterSensor);
  if((millis() - lastFlowReadingTimestamp) > 1000) {
    waterSensor = analogRead(WATER_SENSOR) ;
    detachInterrupt(SENSOR_INTERRUPT) ;
    calculateFlow() ;
    attachInterrupt(SENSOR_INTERRUPT, pulseCounter, FALLING) ;
    if (waterSensor > 400 && !tankFull && fill) {
      digitalWrite(SOLENOID, LOW) ; //Switch Solenoid OFF
      tankFull = true ;
      requestedCups = "12" ; //max water reservoir capacity
      stopMessage("Water Tank", "is Full") ;
    }
    else if (!tankFull && fill && filledOunces < requestedOunces ) {
      digitalWrite(SOLENOID, HIGH) ;    //Switch Solenoid ON
      if (debugMode) {
        debugMessage() ;
      }
      else {
        fillingMessage() ;
        statusBar(filledOunces, requestedOunces) ;
      }
    }
    else if (tankFull || filledOunces >= requestedOunces) {
      digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
      if (!debugMode) {
        filledMessage() ;
      }
      resetVariables() ;
    }
  }
}
//local functions (camelcase)
//LCD functions
void showMsg(int position, int font, String message) {
  display.setTextSize(font) ;   // 1 = 8 pixel tall, 2 = 16 pixel tall...
  display.setTextColor(WHITE) ;
  display.setCursor(0, position) ;
  display.println(message) ;
  display.display() ;
}
void statusBar(float filledOunces, float requestedOunces) {
  int percent = (filledOunces / requestedOunces) * 128 ;
  display.drawRect(0, 32, 128, 32, WHITE) ;
  display.fillRect(0, 32, percent, 32, WHITE) ;
  display.display() ;
}
void clearScreen() {
  display.clearDisplay() ;
  display.display() ;
}
void stopMessage(String line1, String line2) {
  clearScreen() ;
  showMsg(0, 3, "ERROR:") ;
  showMsg(30, 2, line1) ;
  showMsg(48, 2, line2) ;
  delay(3000) ;
}
void filledMessage() {
  clearScreen() ;
  String fillDate = Time.format(Time.now(), "%m-%d-%y") ;
  String fillTime = Time.format(Time.now(), "%I:%M %p") ;
  showMsg(0, 2, "Filled") ;
  showMsg(20, 2, requestedCups + " Cups") ;
  showMsg(40, 1, "Last Filled on:") ;
  showMsg(50, 1, fillDate + " at " + fillTime) ;
}
void fillingMessage() {
  showMsg(0, 2, "Pouring") ;
  showMsg(16, 2, requestedCups + " Cups...") ;
}
void debugMessage() {
  clearScreen() ;
  showMsg(0, 1, "Flow Rate:" + String(flowRateOunces)) ;
  showMsg(10, 1, "Requested Oz:" + String(requestedOunces)) ;
  showMsg(20, 1, "Filled Oz:" + String(filledOunces)) ;
  showMsg(30, 1, "Pulse Count:" + String(pulseCount)) ;
  showMsg(40, 1, "Water Sensor:" + String(waterSensor)) ;
  showMsg(50, 1, "DEBUG MODE ENABLED") ;
}
//flowsensor functions
void pulseCounter()
{
  pulseCount++ ;
}
void calculateFlow() {
  flowRate = ((1000.0 / (millis() - lastFlowReadingTimestamp)) * pulseCount) / calibrationFactor ;
  lastFlowReadingTimestamp = millis() ;
  flowRateOunces = (flowRate / 60) * 1000 * 0.033814 ;
  filledOunces += flowRateOunces ;
  pulseCount = 0 ;
}
//control flow functions
void resetVariables() {
  requestedOunces = 0.01 ;
  filledOunces = 0 ;
  fill = false ;
  tankFull = false ;
}
//cloud functions (pascalcase)
int Calibrate(String message) {
  calibrationFactor = message.toFloat() ;
}
int Stop(String message) {
  digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
  resetVariables() ;
  stopMessage("Emergency", "Stop!") ;
  clearScreen() ;
}
int FillWater(String message) {
  if (!fill) {
    clearScreen() ;
    requestedCups = message ;
    requestedOunces = message.toInt() * 5 ;
    fill = true ;
  }
}
int ManualFill(String message) {
  digitalWrite(SOLENOID, HIGH) ;     //Switch Solenoid ON
}
int ToggleDebugMode(String message) {
  if (!debugMode) {
    debugMode = true ;
  }
  else {
    debugMode = false ;
  }
}
