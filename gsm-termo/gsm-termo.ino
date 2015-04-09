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
}

void loop() {
  lcd.display();
  measureAndPost();
  lcd.clear();
  lcd.noDisplay();
  delay(SLEEP_TIME);
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
      delay(2000);
      return 0;
    }

    delay(1000);

    char temp[50];
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
  int airQuality = analogRead(AIRQUALITY_SENSOR);
  lcd.print(" aq: ");
  lcd.print(airQuality);
}

void readSensors(char* input) {
  float temp = dht.getTemperature();
  float humidity = dht.getHumidity();
  int airQuality = analogRead(AIRQUALITY_SENSOR);

  char tmp[20];
  char *preTmp = "temperature%5Btemperature%5D=";
  char *preHum = "&temperature%5Bhumidity%5D=";
  char *preAirQuality = "&temperature%5Bair_quality%5D=";
  dtostrf(temp, 4, 3, tmp);
  strcpy(input, preTmp);
  strcat(input, tmp);
  dtostrf(humidity, 4, 3, tmp);
  strcat(input, preHum);
  strcat(input, tmp);
  dtostrf(airQuality, 4, 3, tmp);
  strcat(input, preAirQuality);
  strcat(input, tmp);
};
