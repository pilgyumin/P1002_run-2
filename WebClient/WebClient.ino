/*
 WiFiEsp example: WebClientRepeating

 This sketch connects to a web server and makes an HTTP request
 using an Arduino ESP8266 module.
 It repeats the HTTP call each 10 seconds.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp.html
*/

#include "WiFiEsp.h"
#include <Wire.h>
#include <HDC1080.h>
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
SoftwareSerial mySerial(5, 11); //RX, TX
#endif

char ssid[] = "pleasebreath";            // your network SSID (name)
char pass[] = "tnawhatnlwk1";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "192.168.0.2";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

HDC1080 hdcSensor;
float tc, tf, h;
unsigned char  pms[32]; 

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
 Wire.begin();
  hdcSensor.turnOnHeater(true);
  hdcSensor.setTemperatureRes(HDC1080::T_RES_14);
  hdcSensor.setHumidityRes(HDC1080::H_RES_8);
  hdcSensor.updateConfigRegister();
  
  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  mySerial.begin(9600);

  Serial1.listen();
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  
  printWifiStatus();
}

void loop()
{
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only

  /*
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }*/

  // if 10 seconds have passed since your last connection,
  // then connect again and send data

  
  mySerial.listen();
  
  if (millis() - lastConnectionTime > postingInterval) {
  if(mySerial.available()>=32){
     for(int j=0; j<32 ; j++){ 
      pms[j]=mySerial.read(); 
    } 

     int PM1_0=(pms[10]<<8)  | pms[11]; 
    int PM2_5=(pms[12]<<8)  | pms[13];
    int PM10 =(pms[14]<<8)  | pms[15];
    /*
    Serial.print("PM1.0 : "); 
    Serial.print(PM1_0);
    Serial.print(" PM2.5 : "); 
    Serial.print(PM2_5);
    Serial.print(" PM1.0 : "); 
    Serial.println(PM10);
    */
    tf = hdcSensor.getTemperatureHumidity(tc, h);
  
  /*
    Serial.print(tc);
    Serial.print("C ");
    Serial.print(h);
    Serial.println("H");
    */

    Serial1.listen();
    
  String str = "GET http://192.168.0.2:3000?tempInner=";
    str+=tc;
    str+="&humidInner=";
    str+=h;
    str+="&pm10Inner=";
    str+=PM10;
    str+="&pm25Inner=";
    str+=PM2_5;
    
    httpRequest(str);
  }
  }
}

// this method makes a HTTP connection to the server
void httpRequest(String str)
{ 
    
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();
  // if there's a successful connection
  if (client.connect(server, 3000)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    
    client.println(str);
    Serial.println(str);
    //client.println(F("GET ?tempInner=777 HTTP/1.1"));
    //client.println(F("Host: arduino.cc"));
    //client.println("Connection: close");
    client.println();

    // note the time that the connection was made
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}