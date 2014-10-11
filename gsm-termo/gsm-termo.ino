// libraries
#include <GSM.h>

// GSM settings
#define PINNUMBER ""
#define APN ""
#define USER ""
#define PSW  ""

//POST settings
#define URL ""
#define PATH ""

// initialize the library instance
GSM gsmAccess;   // GSM access: include a 'true' parameter for debug enabled
GPRS gprsAccess;  // GPRS access
GSMClient client;  // Client service for TCP connection

// messages for serial monitor response
String oktext = "OK";
String errortext = "ERROR";

char path[] = "/";

// variable for save response obtained
String response = "";

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const double degreePrVolt = 25/0.75;

void setup()
{
  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
}

void loop()
{

  // start GSM shield
  // if your SIM has PIN, pass it as a parameter of begin() in quotes
  Serial.print("Connecting GSM network...");
  if (gsmAccess.begin(PINNUMBER) != GSM_READY)
  {
    Serial.println(errortext);
    while (true);
  }
  Serial.println(oktext);

  Serial.println("Attaching to GPRS with your APN...");
  
  delay(1000);
  
  if (gprsAccess.attachGPRS(APN, USER, PSW) != GPRS_READY)
  {
    Serial.println(errortext);
  }
  else {
    // connection and realize HTTP request
    Serial.println("Connecting and sending POST");
    int res_connect;

    res_connect = client.connect(URL, 80);
    String temp = readTemp();
    Serial.println(temp);
    
    if (res_connect)
    { 
      Serial.println("connected!");
      client.print("POST ");
      client.print(PATH);
      client.println(" HTTP/1.1");
      client.println("Content-Type: application/json");
      client.print("Content-Length: ");
      client.println(temp.length());
      client.println("Host: postcatcher.in");
      client.println();  
      client.print(temp);
      client.println();      

      Serial.println(oktext);
    }
    else
    {
      // if you didn't get a connection to the server
      Serial.println(errortext);
    }
    Serial.print("Receiving response...");

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
          Serial.println(oktext);
          Serial.println("TEST COMPLETE!");
          test = false;
        }
      }

      // if the server's disconnected, stop the client:
      if (!client.connected())
      {
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
        test = false;
      }
    }
  }
}

String readTemp() {
  double sensorValue = analogRead(analogInPin);
  
  double tempVoltage = sensorValue * 0.0049;
  double tempCel = tempVoltage * degreePrVolt;

  char tmp[20];
  String preTmp = "{\"temp\": \"";
  String postTemp = "\"}";
  dtostrf(tempCel, 4, 3, tmp);
  preTmp.concat(tmp);
  preTmp.concat(postTemp);
  
  return preTmp;
};
