// libraries
#include <SoftwareSerial.h>
#include "inetGSM.h"
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GSM settings
#define PINNUMBER "AT+CPIN=0000"
#define APN ""
#define USER ""
#define PSW  ""

//POST settings
#define URL "example.com"
#define PATH "/"

//DHT settings
#define DHTPIN 7

//Air quality
#define AIRQUALITY_SENSOR A0

#define SLEEP_TIME 3000

DHT dht;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
InetGSM inet;

void setup()
{
  dht.setup(DHTPIN);
  lcd.begin(20, 4);
  lcd.home();
  printToScreen("init");
  delay(2000);
}

void loop() {
  measureAndPost(); 
  delay(SLEEP_TIME);
}

char msg[1];
char temp[150];
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
      delay(2000);
      return 0;
    }

    printToScreen("Reading sensors");
    delay(1000);
  
    readSensors(temp);
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
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(str);
  printTemp();
}

void printTemp() {
  lcd.setCursor(0, 4);
  lcd.print("C: ");
  lcd.print(int(dht.getTemperature()));
  lcd.print(" h: ");
  lcd.print(int(dht.getHumidity()));
  lcd.print(" aq: ");
  lcd.print(analogRead(AIRQUALITY_SENSOR));
}

char *preTmp = "t%5Btemperature%5D=";
char *preHum = "\&t%5Bhumidity%5D=";
char *preAirQuality = "\&t%5Bair_quality%5D=";

void readSensors(char* input) {
  float temp = dht.getTemperature();
  float hum = dht.getHumidity();
  float aq = (float)analogRead(AIRQUALITY_SENSOR);

  strcpy(input, "");
  copyAndCat(input, preTmp, temp);
  copyAndCat(input, preHum, hum);
  copyAndCat(input, preAirQuality, aq);
};

void copyAndCat(char* input, char* argument, float measurement) {
  char tmp[20];
  char tempStr[50];
  dtostrf(measurement, 4, 3, tmp);
  strcpy(tempStr, argument);
  strcat(tempStr, tmp);
  strcat(input, tempStr);
}


