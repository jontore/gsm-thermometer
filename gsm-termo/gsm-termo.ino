// libraries
#include <GSM.h>
#include <DHT.h>
#include <Wire.h> 

#include <LiquidCrystal_I2C.h>
#include <Sleep_n0m1.h>


// GSM settings
#define PINNUMBER "0000"
#define APN "internet"
#define USER ""
#define PSW  ""

//POST settings
#define URL ""
#define PATH ""

//DHT settings
#define DHTPIN 9
#define DHTTYPE DHT11   // DHT 11 

#define SLEEPTIME 300000

DHT dht(DHTPIN, DHTTYPE);
Sleep sleep;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
GSM gsmAccess;   // GSM access: include a 'true' parameter for debug enabled
GPRS gprsAccess;  // GPRS access
GSMClient client;  // Client service for TCP connection

// messages for serial monitor response
String oktext = "OK";
String errortext = "ERROR";

char path[] = "/";

// variable for save response obtained
String response = "";

void setup()
{
  lcd.begin(20,4);
  lcd.home();  
  printToScreen("init");
}

void loop() {
  lcd.display();
  measureAndPost();
  sleep.pwrDownMode();
  sleep.sleepDelay(SLEEPTIME);
  lcd.clear();
  lcd.noDisplay();
  delay(300); 
}


void measureAndPost () {
  // start GSM shield
  // if your SIM has PIN, pass it as a parameter of begin() in quotes
  printToScreen("Connecting");
  if (gsmAccess.begin(PINNUMBER) != GSM_READY)
  {
    printToScreen(errortext);
    while (true);
  }
  
  delay(1000);
  
  if (gprsAccess.attachGPRS(APN, USER, PSW) != GPRS_READY)
  {
    printToScreen(errortext);
  }
  else {
    // connection and realize HTTP request
    printToScreen("Updating server");
    int res_connect;

    res_connect = client.connect(URL, 80);
    String temp = readTemp();
    
    if (res_connect)
    { 
      printToScreen("connected!");
      client.print("POST ");
      client.print(PATH);
      client.println(" HTTP/1.1");
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(temp.length());
      client.print("Host: ");
      client.println(URL);
      client.println();  
      client.print(temp);
      client.println();      

      printToScreen(oktext);
    }
    else
    {
      // if you didn't get a connection to the server
      printToScreen(errortext);
    }
    printToScreen("Waiting..");

    boolean test = true;
    while (test)
    {
      // if there are incoming bytes available
      // from the server, read and check them
      if (client.available())
      {
        char c = client.read();
        response += c;

        // cast response obtained from string to char array
        char responsechar[response.length() + 1];
        response.toCharArray(responsechar, response.length() + 1);
        // if response includes a "200 OK" substring
        if (strstr(responsechar, "200 OK") != NULL) {
          printToScreen(oktext);
          test = false;
        }
      }

      // if the server's disconnected, stop the client:
      if (!client.connected())
      {
        printToScreen("disconnecting.");
        client.stop();
        test = false;
      }
    }
  }
}

void printToScreen(String str) {
  lcd.home();
  lcd.print(str);
  printTemp();
}

void printTemp() {
  lcd.setCursor(12, 4);
  lcd.print("C:");
  lcd.print(dht.readTemperature());
}

String readTemp() {
  float t = dht.readTemperature();

  char output[100];
  char tmp[20];
  char *preTmp = "{\"temperature\": {\"temperature\": \"";
  char *postTmp = "\"}}";
  dtostrf(t, 4, 3, tmp);
  strcpy(output, preTmp);
  strcat(output, tmp);
  strcat(output, postTmp);
  return output;
};
