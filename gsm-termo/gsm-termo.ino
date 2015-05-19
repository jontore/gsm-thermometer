// libraries
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <inetGSM.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// GSM settings
#define PINNUMBER "AT+CPIN=0000"
#define APN "internet"
#define USER ""
#define PSW  ""

//POST settings
#define URL "homebrewer.herokuapp.com"
#define PATH "/measurements"

//DHT settings
#define DHTPIN 7

//Air quality
#define AIRQUALITY_SENSOR A0

#define SLEEP_TIME 300000

const char connectMsg[] PROGMEM = "Connecting";
const char errorMsg[] PROGMEM = "Error";
const char readingMsg[] PROGMEM = "Reading sensors";
const char updatingMsg[] PROGMEM = "Updating server";
const char disconnetingMsg[] PROGMEM = "Disconneting";
const char waitingMsg[] PROGMEM = "Waiting";

const char preTmp[] PROGMEM = "d%5Btemperature%5D=";
const char preHum[] PROGMEM = "\&d%5Bhumidity%5D=";
const char preAirQuality[] PROGMEM = "\&d%5Bair_quality%5D=";

const char* const string_table[] PROGMEM = {connectMsg, errorMsg, readingMsg, updatingMsg, disconnetingMsg, waitingMsg, preTmp, preHum, preAirQuality};

#define CONNECTING_STR 0
#define ERROR_STR 1
#define READING_STR 2
#define UPDATING_STR 3
#define DISCONNECTING_STR 4
#define WAITING_STR 5

#define AIRQUALITY_STR 8
#define TEMPERATURE_STR 6
#define HUMIDITY_STR 7

DHT dht;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
InetGSM inet;

void setup()
{
  dht.setup(DHTPIN);
  lcd.begin(20, 4);
  lcd.home();
  delay(2000);
}

void loop() {
  delay(1000);
  measureAndPost(); 
  waitAndMeasure();
}

void waitAndMeasure () {
  int i = SLEEP_TIME / dht.getMinimumSamplingPeriod();
  while (i > 0) {
    printLCDMessage(WAITING_STR);
    delay(dht.getMinimumSamplingPeriod());
    i--;
  }
};

char msg[1];
char temp[150];
int measureAndPost () {
  boolean started = false;
  printLCDMessage(CONNECTING_STR);
  if (gsm.begin(2400)) {
    gsm.SimpleWriteln(PINNUMBER);
    started = true;
  }

  delay(1000);

  if (started) {
    if (!inet.attachGPRS(APN, USER, PSW)) {
      printLCDMessage(ERROR_STR);
      delay(2000);
      return 0;
    }

    printLCDMessage(READING_STR);
    delay(1000);
  
    readSensors(temp);
    printLCDMessage(UPDATING_STR);
    delay(2000);

    inet.httpPOST(URL, 80, PATH, temp, msg, 1);

    delay(500);
    printLCDMessage(DISCONNECTING_STR);

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
  lcd.print("aq: ");
  lcd.print(analogRead(AIRQUALITY_SENSOR));
}


void readSensors(char* input) {
  float temp = dht.getTemperature();
  float hum = dht.getHumidity();
  float aq = (float)analogRead(AIRQUALITY_SENSOR);

  strcpy(input, "");
  copyAndCat(input, TEMPERATURE_STR, temp);
  copyAndCat(input, HUMIDITY_STR, hum);
  copyAndCat(input, AIRQUALITY_STR, aq);
};

void copyAndCat(char* input, int str_position, float measurement) {
  char tmp[20];
  char tempStr[50];
  dtostrf(measurement, 4, 3, tmp);
  strcpy_P(tempStr, (char*)pgm_read_word(&(string_table[str_position])));
  strcat(tempStr, tmp);
  strcat(input, tempStr);
}

char buffer[30];
char* printLCDMessage(int i) {
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[i])));
  printToScreen(buffer);
}


