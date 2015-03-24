// libraries
#include <SoftwareSerial.h>
#include "inetGSM.h"
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Sleep_n0m1.h>

// GSM settings
#define PINNUMBER "AT+CPIN=0000"
#define APN "internet"
#define USER ""
#define PSW  ""

//POST settings
#define URL "example.com"
#define PATH "/path"

//DHT settings
#define DHTPIN 7

#define SLEEPTIME 300

DHT dht;
Sleep sleep;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
InetGSM inet;

void setup()
{
  dht.setup(DHTPIN);
  lcd.begin(20, 4);
  lcd.home();
  printToScreen("init");
}

void loop() {
  lcd.display();
  measureAndPost();
  lcd.clear();
  lcd.noDisplay();
  sleep.pwrDownMode();
  sleep.sleepDelay(SLEEPTIME);
  delay(300);
}

char msg[1];
int measureAndPost () {
  boolean started = false;
  printToScreen("Connecting");
  if (gsm.begin(2400)) {
    gsm.SimpleWriteln(PINNUMBER);
    started = true;
  }

  delay(1000);

  if (started) {
    if (!inet.attachGPRS(APN, USER, PSW)) {
      printToScreen("Error");
    }
    
    delay(1000);
    
    char temp[50];
    readTemp(temp);
    printToScreen("Updating server");
    delay(2000);
    
    inet.httpPOST(URL, 80, PATH, temp, msg, 1);
    
    delay(500);
    printToScreen("disconnecting");
    
    return 1;
  }
}

void printToScreen(String str) {
  lcd.home();
  lcd.clear();
  lcd.print(str);
  printTemp();
}

void printTemp() {
  lcd.setCursor(12, 4);
  lcd.print("C:");
  lcd.print(dht.getTemperature());
}


void readTemp(char* input) {
  float t = dht.getTemperature();
  char tmp[20];    
  char *preTmp = "temperature%5Btemperature%5D=";
  dtostrf(t, 4, 3, tmp);
  strcpy(input, preTmp);
  strcat(input, tmp);
};
