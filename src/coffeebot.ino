#include "Adafruit_SSD1306.h"
#define OLED_I2C_ADDRESS 0x3C
#define OLED_RESET D4
#define SOLENOID 3
#define WATER_SENSOR A0
Adafruit_SSD1306 display(OLED_RESET) ;

// declare global variables
volatile bool fill = false ;
volatile bool tankFull = false ;
int filledOunces ;
int waterSensor ;
int requestedOunces ;
String requestedCups ;

//initialize
void setup() {
  //Serial.begin(9600);
  pinMode(SOLENOID, OUTPUT) ;         //Sets the solenoid pin as an output
  Time.zone(-4);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  clearScreen(); //necessary?
  //cloud functions
  Particle.function("Stop", Stop);
  Particle.function("FillWater", FillWater);
  Particle.function("ManualFill", ManualFill);
}

//begin loop
void loop() {
  waterSensor = analogRead(WATER_SENSOR);
  //Serial.println(waterSensor);
  if (waterSensor > 400 && !tankFull && fill) {
    digitalWrite(SOLENOID, LOW) ; //Switch Solenoid OFF
    tankFull = true;
    requestedCups = "12"; //max water reservoir capacity
    clearScreen();
    stopMessage("Water Tank", "is Full");
  }
  else {
    if (!tankFull && fill && filledOunces < requestedOunces ) {
        digitalWrite(SOLENOID, HIGH) ;    //Switch Solenoid ON
        fillingMessage(requestedCups) ;
        statusBar(filledOunces, requestedOunces);
    }
    else if (tankFull || filledOunces >= requestedOunces) {
      digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
      fillingMessage(requestedCups) ;
      resetVariables() ;
    }
  }
} // end loop

//local functions (camelcase)
//LCD functions
void showMsg(int position, int font, String message) {
  display.setTextSize(font) ;   // 1 = 8 pixel tall, 2 = 16 pixel tall...
  display.setTextColor(WHITE) ;
  display.setCursor(0, position) ;
  display.println(message) ;
  display.display() ;
}
void statusBar(int filledOunces, int requestedOunces) {
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
void filledMessage(String cups) {
  clearScreen() ;
  String fillDate = Time.format(Time.now(), "%m-%d-%y") ;
  String fillTime = Time.format(Time.now(), "%I:%M %p") ;
  showMsg(0, 2, "Filled") ;
  showMsg(20, 2, cups + " Cups") ;
  showMsg(40, 1, "Last Filled on:") ;
  showMsg(50, 1, fillDate + " at " + fillTime) ;
}
void fillingMessage(String cups) {
  showMsg(0, 2, "Pouring");
  showMsg(16, 2, cups + " Cups...");
}
//flowsensor functions

//control flow functions
void resetVariables() {
  requestedOunces = 0 ;
  filledOunces = 0 ;
  fill = false ;
  tankFull = false ;
}

//cloud functions (pascalcase)
int Stop(String message) {
  digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
  resetVariables();
  stopMessage("Emergency", "Stop!");
  clearScreen();
}
int FillWater(String message) {
  if (!fill) {
    clearScreen() ;
    requestedCups = message ;
    requestedOunces = message.toInt() * 5;
    fill = true ;
  }
}
int ManualFill(String message) {
  digitalWrite(SOLENOID, HIGH) ;     //Switch Solenoid ON
}
