#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Arduino.h>

//Für Temperatursensor
#include <OneWire.h>
#include <DallasTemperature.h>


//## temperatur
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Webserver
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
ESP8266WebServer server(8080);


#include <Display.h>
Display *myDisplay = new Display();

//Wifi
#define STASSID "FRITZ!Box 7560 TZ"
#define STAPSK  "95172809322535102456"
const char* ssid = STASSID;
const char* password = STAPSK;

//Telnet
/*
uint8_t i;
bool ConnectionEstablished; // Flag for successfully handled connection
#define MAX_TELNET_CLIENTS 1
WiFiServer TelnetServer(2323);
WiFiClient TelnetClient[MAX_TELNET_CLIENTS];
*/

//NTP Timeserver
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");


//Wemo Alexa
#include <fauxmoESP.h>
fauxmoESP fauxmo;

//OTA Updater
const char* host = "Pool-Controller";

bool tempSensorNoSleep = false;



void handleTempUpdate() {
  myDisplay->currentjob = "process request";
  myDisplay->update();
  if (server.method() != HTTP_POST) {
    //digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    //digitalWrite(led, 0);
  } else {
    //digitalWrite(led, 1);
    String message = "";
    for (uint8_t i = 0; i < server.args(); i++) {
      //message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      if(server.argName(i) == "temp1") {
        myDisplay->temp1 = String(server.arg(i)).toFloat();
        myDisplay->updateTime = myDisplay->time;
        //myDisplay->tpStatus = "running";
        message = String(timeClient.getHours());
      }
      if(server.argName(i) == "temp2") {
        myDisplay->temp2 = String(server.arg(i)).toFloat();
        myDisplay->updateTime = myDisplay->time;
        message = String(timeClient.getHours());
      }
      if(server.argName(i) == "volt") {
        myDisplay->volt = String(server.arg(i)).toFloat();
        //myDisplay->tpStatus = String(myDisplay->temp1)+String("/")+String(myDisplay->temp2)+String(" ")+String(myDisplay->volt) + String("V");
        //myDisplay->update();
      }
      if(server.argName(i) == "sleeping") {
        Serial.println(F("Temp go sleeping"));
        myDisplay->tpStatus = String(server.arg(i)) + "/" + String(myDisplay->volt);
        myDisplay->currentjob = "TP go sleep " + String(server.arg(i));
        myDisplay->update();
        if(tempSensorNoSleep) {
          message = "noSleep";
        }else {
          message = "goSleep";
        }
        
        delay(1000);
      }
    }
    
    server.send(200, "text/plain", message);
    Serial.println(message);
    delay(500);
    myDisplay->currentjob = "";
    myDisplay->update();
    //digitalWrite(led, 0);
  }
}

void handleStats() {
  myDisplay->currentjob = "request stats";
  myDisplay->update();

  String message;
  message.reserve(2600);

  message = "<html><head><title>Poolcontroller Stats</title></head><body>";
  message += "<h2>Stats: </h2>";
  message += "<div>Wassertemperatur: ";
  message += String(myDisplay->temp1);
  message += "</div>";
  message += "<div>Kollektortemperatur: ";
  message += String(myDisplay->temp2);
  message += "</div>";
  message += "<div>Volt: ";
  message += String(myDisplay->volt);
  message += "</div>";
  message += "<div>TP Status: ";
  message += String(myDisplay->tpStatus);
  message += "</div>";
  message += "<div>Last Update: ";
  message += String(myDisplay->updateTime);
  message += "</div>";
  message += "<div>Time: ";
  message += String(myDisplay->time);
  message += "</div>";
  message += "<div>Pumpe: ";
  message += String(myDisplay->pumping);
  message += "</div>";
  message += "<div>Licht: ";
  message += String(myDisplay->light);
  message += "</div>";
  message += "<div>NoSleep: ";
  message += String(tempSensorNoSleep);
  message += "</div>";
  message += "<div>Heating: ";
  message += String(myDisplay->heating);
  message += "</div>";
  message += "</body></html>";

  server.send(200, "text/html", message);
  delay(500);
  myDisplay->currentjob = "";
  myDisplay->update();
}

void setup() {
  Serial.begin(9600);
  Serial.println("setup ...");
  myDisplay->setup();
  myDisplay->update();

  myDisplay->currentjob = "ini wifi";
  myDisplay->update();
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(F("."));
    myDisplay->currentjob = "connecting wifi";
    myDisplay->update();
  }

  myDisplay->currentjob = "";
  myDisplay->wifiStatus = true;
  myDisplay->wifiName = STASSID;
  myDisplay->ip = WiFi.localIP().toString();
  myDisplay->update();
  
  /*
  myDisplay->currentjob = "start Telnet";
  myDisplay->update();
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);
  */

  myDisplay->currentjob = "start NTP";
  myDisplay->update();
  timeClient.begin();
  timeClient.setTimeOffset(60 * 60 * 2);
  timeClient.update();
  delay(500);
  
  Serial.println("start sensors");
  sensors.begin();

  myDisplay->currentjob = "start webserver";
  myDisplay->update();
  Serial.println("start webserver");
  server.on("/", handleTempUpdate);
  server.on("/stats", handleStats);
  server.begin();
  delay(500);

  Serial.println("setup finish");
  myDisplay->currentjob = "";
  myDisplay->update();


  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);


  fauxmo.addDevice("Pool Heizung");
  fauxmo.addDevice("Pool P");
  fauxmo.addDevice("Pool Light");
  fauxmo.addDevice("Pool Temp No Sleep");
  fauxmo.setPort(80);
  fauxmo.enable(true);

  fauxmo.setState("Pool Heizung", myDisplay->heating, 254);
  fauxmo.setState("Pool P", myDisplay->pumping, 254);
  fauxmo.setState("Pool Light", myDisplay->light, 254);
  fauxmo.setState("Pool Temp No Sleep", tempSensorNoSleep, 254);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.print("Device "); Serial.print(device_name); 
        Serial.print(" state: "); Serial.print(state); 
        Serial.print(" value: "); Serial.print(value);
        if(String(device_name) == String("Pool Heizung")) {
          myDisplay->heating = state;
        }
        if(String(device_name) == String("Pool P")) {
          myDisplay->pumping = state;
          if(!myDisplay->pumping) {
            myDisplay->heating = false;
            fauxmo.setState("Pool Heizung", false, 254);
          }
        }
        if(String(device_name) == String("Pool Light")) {
          myDisplay->light = state;
        }
        if(String(device_name) == String("Pool Temp No Sleep")) {
          tempSensorNoSleep = state;
        }
    });

  //pinMode(13, OUTPUT);


  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
    myDisplay->currentjob = "Systemupdate ...";
    myDisplay->update();
  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    myDisplay->currentjob = "Update finish ...";
    myDisplay->update();
    delay(1000);
    myDisplay->currentjob = "restart ESP ...";
    myDisplay->update();
    delay(1000);
    ESP.restart();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    myDisplay->currentjob = "Update error ...";
    myDisplay->update();
    delay(1000);
    ESP.restart();
  });

  /* setup the OTA server */
  ArduinoOTA.begin();
  
}

////Telent
/*
void TelnetMsg(String text)
{
  Serial.println(text);
  for(i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] || TelnetClient[i].connected())
    {
      TelnetClient[i].println(text);
    }
  }
  delay(10);  // to avoid strange characters left in buffer
}
      
void Telnet()
{
  // Cleanup disconnected session
  for(i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] && !TelnetClient[i].connected())
    {
      Serial.print("Client disconnected ... terminate session "); Serial.println(i+1); 
      TelnetClient[i].stop();
    }
  }

  
  // Check new client connections
  if (TelnetServer.hasClient())
  {
    ConnectionEstablished = false; // Set to false
    
    for(i = 0; i < MAX_TELNET_CLIENTS; i++)
    {
      // Serial.print("Checking telnet session "); Serial.println(i+1);
      
      // find free socket
      if (!TelnetClient[i])
      {
        TelnetClient[i] = TelnetServer.available(); 
        
        Serial.print("New Telnet client connected to session "); Serial.println(i+1);
        
        TelnetClient[i].flush();  // clear input buffer, else you get strange characters
        TelnetClient[i].println("Welcome!");
        
        TelnetClient[i].print("Millis since start: ");
        TelnetClient[i].println(millis());
        
        TelnetClient[i].print("Free Heap RAM: ");
        TelnetClient[i].println(ESP.getFreeHeap());
  
        TelnetClient[i].println("----------------------------------------------------------------");
        
        ConnectionEstablished = true; 
        
        break;
      }
      else
      {
        // Serial.println("Session is in use");
      }
    }

    if (ConnectionEstablished == false)
    {
      Serial.println("No free sessions ... drop connection");
      TelnetServer.available().stop();
      // TelnetMsg("An other user cannot connect ... MAX_TELNET_CLIENTS limit is reached!");
    }
  }

  for(i = 0; i < MAX_TELNET_CLIENTS; i++)
  {
    if (TelnetClient[i] && TelnetClient[i].connected())
    {
      if(TelnetClient[i].available())
      { 
        //get data from the telnet client
        while(TelnetClient[i].available())
        {
          Serial.write(TelnetClient[i].read());
        }
      }
    }
  }
}
*/
///Telent end



void getTemperatures(void) {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    //Serial.print("Temperature for the device 1 (index 0) is: ");
    //Serial.println(tempC);
    myDisplay->temp1 = tempC;
  } 
  else
  {
    Serial.println("Error: Could not read temperature data (index 0)");
  }

  float tempC2 = sensors.getTempCByIndex(1);
  if(tempC2 != DEVICE_DISCONNECTED_C) 
  {
    //Serial.print("Temperature for the device 2 (index 1) is: ");
    //Serial.println(tempC2);
    myDisplay->temp2 = tempC2;
  } 
  else
  {
    Serial.println("Error: Could not read temperature data (index 1)");
  }
}


void loop() {
  ArduinoOTA.handle();
  fauxmo.handle();
  getTemperatures();
  //Telnet();
  //digitalWrite(12, HIGH);
  //digitalWrite(13, LOW);


  String formattedTime = timeClient.getFormattedTime();
  //Serial.print("Formatted Time: ");
  //Serial.println(formattedTime);
  myDisplay->time = formattedTime;
  
  myDisplay->update();
  server.handleClient();

  //digitalWrite(12, LOW);
  //digitalWrite(13, HIGH);

  if( (round(myDisplay->temp2) >= round(myDisplay->temp1)+6 && myDisplay->heating) || myDisplay->pumping) {
    digitalWrite(12, LOW);
    fauxmo.setState("Pool P", true, 254);
  }else if( (round(myDisplay->temp2) <= round(myDisplay->temp1)+2 && myDisplay->heating) || (!myDisplay->heating and !myDisplay->pumping) ) {
    digitalWrite(12, HIGH);
    fauxmo.setState("Pool P", false, 254);
  }

  if(myDisplay->light) {
    digitalWrite(13, LOW);
  }else {
    digitalWrite(13, HIGH);
  }

  /*
  TelnetMsg(String("Wasser Temp: ")+myDisplay->temp1);
  TelnetMsg(String("Kollektor Temp: ")+myDisplay->temp2);
  TelnetMsg(String("Pumpe: ")+myDisplay->pumping);
  TelnetMsg(String("Licht: ")+myDisplay->light);
  TelnetMsg(String("NoSleep: ")+tempSensorNoSleep);
  TelnetMsg(String("TP Status: ")+myDisplay->tpStatus);
  TelnetMsg(String("Last Update: ")+myDisplay->updateTime);
  TelnetMsg(String("Time: ")+myDisplay->time);

  TelnetMsg("...");
  */
  delay(500);

  //Serial.println("Going to deep sleep...");
  //ESP.deepSleep(60 * 1000000);

}