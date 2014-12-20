// libraries
#include "SIM900.h"
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


// messages for serial monitor response
String oktext = "OK";
String errortext = "ERROR";

char path[] = "/";

// variable for save response obtained
String response = "";

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

    // connection and realize HTTP request
    boolean res_connected = false;

    int n_of_at = 0;
    while(n_of_at < 3){
       if(!inet.connectTCP(URL, 80)){
          n_of_at++;
        } else {
          res_connected = true;
          break;
      }
    }
    char *temp = readTemp();
    printToScreen("Updating server");
    if (res_connected)
    {
      char end_c[2];
      end_c[0]=0x1a;
      end_c[1]='\0';
      printToScreen("connected!");
      
      gsm.SimpleWrite("POST ");
      gsm.SimpleWrite(PATH);
      gsm.SimpleWrite(" HTTP/1.1\n");
      gsm.SimpleWrite("Host: ");
      gsm.SimpleWrite(URL);
      gsm.SimpleWrite("\n");
      gsm.SimpleWrite("User-Agent: Arduino\n");
      gsm.SimpleWrite("Content-Type: application/json\n");
      gsm.SimpleWrite("Content-Length: ");
      gsm.SimpleWrite(strlen(temp));
      gsm.SimpleWrite("\n\n");
      gsm.SimpleWrite(temp);
      gsm.SimpleWrite("\n\n");
      gsm.SimpleWrite(end_c);
      free(temp);  

      switch (gsm.WaitResp(10000, 10, "SEND OK")) {
        case RX_TMOUT_ERR:
          return 0;
          break;
        case RX_FINISHED_STR_NOT_RECV:
          return 0;
          break;
      }
    }
    else {
      // if you didn't get a connection to the server
      printToScreen(errortext);
    }
    
    delay(500);
    printToScreen("disconnecting.");
    
    inet.disconnectTCP();
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

  char *output = (char*)malloc(sizeof(char) * 100);
  char tmp[20];
  char *preTmp = "{\"temperature\": {\"temperature\": \"";
  char *postTmp = "\"}}";
  dtostrf(t, 4, 3, tmp);
  strcpy(output, preTmp);
  strcat(output, tmp);
  strcat(output, postTmp);
  return output;
};
