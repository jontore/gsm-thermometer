// libraries
#include <SoftwareSerial.h>
#include "inetGSM.h"
#include <DHT.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h>
#include <Sleep_n0m1.h>


// GSM settings
#define PINNUMBER "AT+CPIN=0000"
#define APN ""
#define USER ""
#define PSW  ""

//POST settings
#define URL ""
#define PATH ""

//DHT settings
#define DHTPIN 7
#define DHTTYPE DHT11   // DHT 11 

#define SLEEPTIME 300000

DHT dht(DHTPIN, DHTTYPE);
Sleep sleep;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
InetGSM inet;

String errortext = "ERROR";

void setup()
{
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

char msg[50];
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
      printToScreen(errortext);
    }
    
    delay(1000);
    
    char *temp = readTemp();
    printToScreen("Updating server");
    
    inet.httpPOST("homebrewer.herokuapp.com", 80, "/temperatures", temp, msg, 50);
    free(temp);
    
    delay(500);
    printToScreen("disconnecting.");
    
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
  lcd.print(dht.readTemperature());
}


char* readTemp() {
  float t = dht.readTemperature();
  char tmp[20];    
  char *output = (char*)malloc(sizeof(char) * 100);
  char *preTmp = "temperature%5Btemperature%5D=";
  dtostrf(t, 4, 3, tmp);
  strcpy(output, preTmp);
  strcat(output, tmp);
  return output;
};
