#include "Adafruit_SSD1306.h"
#define OLED_I2C_ADDRESS 0x3C
#define OLED_RESET D4
#define SOLENOID 3
#define WATER_SENSOR A0
Adafruit_SSD1306 display(OLED_RESET);

// declare global variables
volatile bool fill = false;
volatile bool tankFull = false;
float statusPercent;
int cups;
int timeDelay;
int waterSensor;
//initialize
void setup() {
  //Serial.begin(9600);
  pinMode(SOLENOID, OUTPUT) ;           //Sets the pin as an output
  Time.zone(-4);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  clearScreen();
  //cloud functions
  Particle.function("Stop", Stop);
  Particle.function("FillWater", FillWater);
}

//begin loop
void loop() {
  waterSensor = analogRead(WATER_SENSOR);
  //Serial.println(waterSensor);
  if (waterSensor > 400 && !tankFull && fill) {
    digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
    tankFull = true;
    cups = 12; //max water reservoir capacity
    clearScreen();
    showMsg(0, 3, "ERROR:");
    showMsg(30, 2, "Water Tank");
    showMsg(48, 2, "is Full");
    delay(3000);
  }
  else {
    if (!tankFull && fill && statusPercent < 101 ) {
      digitalWrite(SOLENOID, HIGH) ;    //Switch Solenoid ON
      showMsg(0, 2, "Pouring");
      showMsg(16, 2, String(cups) + " Cups...");
      statusBar(statusPercent);
      // idea, calibrate at longest fill, then increase percent increment for smaller fills based on cups
      statusPercent += 5 ;
      timeDelay = 255 * cups ;
      delay(timeDelay) ; // 400 * cups
    }
    else if (tankFull || statusPercent >= 101) {
      digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
      clearScreen();
      String fillDate = Time.format(Time.now(), "%m-%d-%y");
      String fillTime = Time.format(Time.now(), "%I:%M %p");
      showMsg(0, 2, "Filled");
      showMsg(20, 2, String(cups) + " Cups");
      showMsg(40, 1, "Last Filled on:");
      showMsg(50, 1, fillDate + " at " + fillTime);
      resetVariables();
    }
  }
} // end loop

// local functions camelcase
void showMsg(int position, int font, String message) {
  display.setTextSize(font);   // 1 = 8 pixel tall, 2 = 16 pixel tall...
  display.setTextColor(WHITE);
  display.setCursor(0, position);
  display.println(message);
  display.display();
}
void statusBar(int n) {
  int percent = (n / 100.0) * 128;
  display.drawRect(0, 32, 128, 32, WHITE);
  display.fillRect(0, 32, percent, 32, WHITE);
  display.display();
}
void clearScreen() {
  display.clearDisplay() ;
  display.display() ;
}
void resetVariables() {
  cups = 0;
  timeDelay = 0;
  statusPercent = 0;
  fill = false;
  tankFull = false;
}
//cloud functions pascalcase
int Stop(String message) {
  digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
  resetVariables();
  clearScreen();
  showMsg(0, 3, "ERROR:");
  showMsg(30, 2, "Emergency");
  showMsg(48, 2, "Stop!");
  delay(5000);
  clearScreen();
}
int FillWater(String message) {
  //idea have alexa set cups, create delay based on cups
  if (!fill) {
    clearScreen() ;
    cups = message.toInt() ;
    fill = true ;
  }
}
