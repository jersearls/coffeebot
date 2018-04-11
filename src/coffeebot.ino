#include "Adafruit_SSD1306.h"
#define OLED_I2C_ADDRESS 0x3C
#define OLED_RESET D4
#define SOLENOID 0
#define WATER_SENSOR A0
Adafruit_SSD1306 display(OLED_RESET);

// declare global variables
volatile bool fill = false;
int statusPercent;
int cups;
int timeDelay;
int waterSensor;

//initialize
void setup() {
  //Serial.begin(9600);
  pinMode(SOLENOID, OUTPUT) ;           //Sets the pin as an output
  Time.zone(-4);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  ClearScreen();
  //cloud functions
  Particle.function("Stop", Stop);
  Particle.function("TwoCups", FillTwoCups);
  Particle.function("FourCups", FillFourCups);
  Particle.function("SixCups", FillSixCups);
  Particle.function("EightCups", FillEightCups);
  Particle.function("TenCups", FillTenCups);
  Particle.function("TwelveCups", FillTwelveCups);
}

//begin loop
void loop() {
  waterSensor = analogRead(WATER_SENSOR);
  //Serial.println(waterSensor);
  if (waterSensor < 1000) {
    if (fill && statusPercent < 101) {
      //digitalWrite(SOLENOID, HIGH) ;    //Switch Solenoid ON
      showMsg(0, 2, "Pouring");
      showMsg(16, 2, String(cups) + " Cups...");
      StatusBar(statusPercent);
      statusPercent += 5;
      delay(timeDelay);
    }
    else if (fill && statusPercent >= 101) {
      //digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
      ClearScreen();
      String fillDate = Time.format(Time.now(), "%m-%d-%y");
      String fillTime = Time.format(Time.now(), "%I:%M %p");
      showMsg(0, 2, "Filled");
      showMsg(20, 2, String(cups) + " Cups");
      showMsg(40, 1, "Last Filled on:");
      showMsg(50, 1, fillDate + " at " + fillTime);
      ResetVariables();
    }
  }
  else {
    //digitalWrite(SOLENOID, LOW) ;     //Switch Solenoid OFF
    ResetVariables();
    ClearScreen();
    showMsg(0, 3, "ERROR:");
    showMsg(32, 2, "Water Tank");
    showMsg(48, 2, "Full");
    delay(5000);
    ClearScreen();
  }
  delay(100);
}

//private functions
void showMsg(int position, int font, String message) {
  display.setTextSize(font);   // 1 = 8 pixel tall, 2 = 16 pixel tall...
  display.setTextColor(WHITE);
  display.setCursor(0, position);
  display.println(message);
  display.display();
}
void StatusBar(int n) {
  int percent = (n / 100.0) * 128;
  display.drawRect(0, 32, 128, 32, WHITE);
  display.fillRect(0, 32, percent, 32, WHITE);
  display.display();
}
void ClearScreen() {
  display.clearDisplay();
  display.display();
}
void ResetVariables() {
  cups = 0;
  timeDelay = 0;
  statusPercent = 0;
  fill = false;
}
//cloud functions
int Stop(String message) {
  ResetVariables();
  ClearScreen();
  showMsg(0, 3, "ERROR:");
  showMsg(30, 2, "Emergency");
  showMsg(48, 2, "Stop!");
  delay(5000);
  ClearScreen();
}
int FillTwoCups(String message) {
  ClearScreen();
  fill = true;
  cups = 2;
  timeDelay = 0;
}
int FillFourCups(String message) {
  ClearScreen();
  fill = true;
  cups = 4;
  timeDelay = 0;
}
int FillSixCups(String message) {
  ClearScreen();
  fill = true;
  cups = 6;
  timeDelay = 0;
}
int FillEightCups(String message) {
  ClearScreen();
  fill = true;
  cups = 8;
  timeDelay = 0;
}
int FillTenCups(String message) {
  ClearScreen();
  fill = true;
  cups = 10;
  timeDelay = 0;
}
int FillTwelveCups(String message) {
  ClearScreen();
  fill = true;
  cups = 12;
  timeDelay = 500;
}
