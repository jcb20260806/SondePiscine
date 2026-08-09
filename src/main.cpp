#include <Arduino.h>

// v01 : connexion Wifi router ok - NB: limitation niveau version WiFi
// v02 : serveur fonctionne ok vers WiFI-2.4 fonctionne
// v03 : DHT Fonctionne
// v04 : OLED OK
// V05 : BMP fonctionne
// V06 : DS18B20 : eau immergé fonctionne
// v07 : improve Display
// v08 ; Router "Proximus-Home-33F8" = BBOx
// v09 : Improve Display - Finale
// v10 : Modify pour Platform IO dans VSCode
// v11 : Use Main WiFi Router
// v12 : Diverses Adaptations
// v13 : Force Local IP Address 192.168.129.200; voir WiFi.config(IPAddress(192, 168, 129, 200), IPAddress(192, 168, 129, 1), IPAddress(255, 255, 255, 0));
// V14 : change IP to 192.168.129.100:82 pour accès externe
// v15 : oaramètres réseau comme variables"
// Branch
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include <ESP8266mDNS.h>
// pour OTA
#include <ArduinoOTA.h>
// pour BMP etc 
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
// OLED SCREEN
#include "SH1106Wire.h"
// pour image
//#include <LittleFS.h>
const char* wifiStatusToString(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:     return "WL_IDLE_STATUS";
        case WL_NO_SSID_AVAIL:   return "WL_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED:  return "WL_SCAN_COMPLETED";
        case WL_CONNECTED:       return "WL_CONNECTED";
        case WL_CONNECT_FAILED:  return "WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
        case WL_DISCONNECTED:    return "WL_DISCONNECTED";
        default:                 return "UNKNOWN_STATUS";
    }
}

// ================= WIFI =================
const char* ssid = "WiFi-2.4-DBFA"; //"Proximus-Home-33F8";//"G604T_WIRELESS";// non Compatible "Proximus-Home-186132"; //"WiFi-2.4-DBFA"; //"G604T_WIRELESS";
const char* password = "alixetjc"; //"jcetalix";// "";
const IPAddress  wifipara1 = IPAddress(192, 168, 129, 100);
const IPAddress wifipara2= IPAddress(192, 168, 129, 1);
const IPAddress wifipara3 = IPAddress(255, 255, 255, 0);
const int wifiport = 82;
ESP8266WebServer server(wifiport);
  

bool LoopEntered = false;
char ipStr[16];


// ================= SENSORS =================

// BMP180
Adafruit_BMP085 bmp;
bool bmpOK = false;
float pressure_hpa = 0;
float temp_bmp = 0; 


// DHT
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

char dhtTemp[10];
char dhtHum[10];
char DS18[10];
char message[16];

// DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 14 // connecté à D5 ; Résistance de 4.7k entre data et VCC
OneWire oneWire(ONE_WIRE_BUS);
#define TEMPERATURE_PRECISION 12 // Lower resolution=9, max=12
DallasTemperature sensors(&oneWire);
int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
// OLED
SH1106Wire display(0x3C, SDA, SCL);
void drawTextFlowDemo(const char *Texte) {

  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.drawStringMaxWidth(0,0,128,Texte);
}

void jcbDisplay(const char *pTexte) {
  display.clear();

  Serial.println(pTexte);

  drawTextFlowDemo(pTexte);

  display.display();
display.display();

for (int i = 0; i < 50; i++) {
    ArduinoOTA.handle();
  
    delay(10);
}
}

float waterTemp = 0;
void initDS18B20(){
  jcbDisplay("Init DS18B20");
  // Set-up DS18B20
// Start up the library
  sensors.begin();
  
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  
  // locate devices on the bus
  Serial.print("Locating DS18B20 devices...");
  
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" DS18B20 devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
   
  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
  {
    Serial.print("Found device ");
    Serial.print(i, DEC);
    Serial.print(" with address: ");
    Serial.print("printAddress(tempDeviceAddress)");
    Serial.println();
    
    Serial.print("Setting resolution to ");
    Serial.println(TEMPERATURE_PRECISION, DEC);
    
    // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
    sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    
    Serial.print("Resolution actually set to: ");
    Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
    Serial.println();
  }else{
    Serial.print("Found ghost device at ");
    Serial.print(i, DEC);
    Serial.print(" but could not detect address. Check power and cabling");
  }
  

}


}


// ================= BMP =================
void initBMP() {
  if (!bmp.begin()) {
    jcbDisplay("BMP FAIL");
    bmpOK = false;
  } else {
    jcbDisplay("BMP OK");
    bmpOK = true;
  }
}
void makeMessage(char *message, size_t size, const char *label, float value)
{
    char valueStr[10];
    dtostrf(value, 0, 2, valueStr);

    snprintf(message, size, "%s %s", label, valueStr);
}

void readBMP() {
  //jcbDisplay("Read BMP");
  if (!bmpOK) return;

  pressure_hpa = bmp.readPressure() / 100.0;
  temp_bmp = bmp.readTemperature();
  //char tempStr[10];
  //dtostrf(temp_bmp, 0, 2, tempStr);
  //const char *c = "BMP T°";
  
  //snprintf(message, sizeof(message), "%c BMP T° %s",c , tempStr);
  makeMessage(message, sizeof(message), "BMP T°", temp_bmp);
  jcbDisplay(message);
  makeMessage(message, sizeof(message), "hPa", pressure_hpa);
  jcbDisplay(message);
}

// ================= DHT =================
void readDHT() {
  //jcbDisplay("Read DHT");
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) return;

  dtostrf(t, 5, 1, dhtTemp);
  makeMessage(message, sizeof(message), "DHT T°", t);
  jcbDisplay(message);
  //jcbDisplay(dhtTemp);
  dtostrf(h, 5, 1, dhtHum);
  makeMessage(message, sizeof(message), "DHT H%", h);
  jcbDisplay(message);
}

// ================= DS18B20 =================
void readDS18B20() {
  
  sensors.requestTemperatures();
  waterTemp = sensors.getTempCByIndex(0);
  dtostrf(waterTemp, 5, 1, DS18);
  makeMessage(message, sizeof(message), "DS18 T°", waterTemp);
  jcbDisplay(message);
}

// ================= HTML =================
String page() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta http-equiv='refresh' content='10'>";
  html += "<style>";
html += "body { font-family: Arial, sans-serif; font-size: 28px; }";
html += "h1 { font-size: 42px; color: blue; }";
html += "h3 { font-size: 32px; margin-top: 20px; }";
html += "</style>";
  html += "</head><body>";

  html += "<h1>ESP8266 Station upDate On The Air 4-8- Ajouter ,5° pour l' eau</h1>";
  //html += "<img src='/piscine.png' width='300'><br>";

  html += "<h3>DHT</h3>";
  html += "Temp: "; html += dhtTemp; html += " °C<br>";
  html += "Hum: "; html += dhtHum; html += " %<br>";

  html += "<h3>Eau</h3>";
  html += "Temp eau: "; html += String(waterTemp); html += " °C<br>";

  if (bmpOK) {
    html += "<h3>BMP180</h3>";
    html += "Pression: "; html += String(pressure_hpa); html += " hPa<br>";
    html += "Temp BMP: "; html += String(temp_bmp); html += " °C<br>";
  }

  html += "</body></html>";
  return html;
}

// ================= HANDLER =================
void handleRoot() {
  jcbDisplay("Handle Root");
  readDHT();
  readBMP();
  readDS18B20();
  jcbDisplay(ipStr);

  

  server.send(200, "text/html", page());
}







// ================= SETUP =================
void setup() {
  /*if (!LittleFS.begin()) {
    Serial.println("Erreur LittleFS");
}
else {
    jcbDisplay("LittleFS OK");
}
    */
  Serial.begin(74880);
  Serial.println("Starting...");
  Serial.print("MAC : ");
  Serial.println(WiFi.macAddress());
  
  Wire.begin();
  Wire.setClock(400000);
  display.init();
  display.flipScreenVertically();
  display.clear();
  //display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "TEST OLED");
  display.display();
  

  dht.begin();
  sensors.begin();

  initBMP();
  jcbDisplay("ICI");
  initDS18B20();
  WiFi.config(wifipara1,wifipara2,wifipara3);
  WiFi.begin(ssid, password);
  jcbDisplay("Connexion WiFi Router ");
  jcbDisplay(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Try to Connect.");
  }
  //jcbDisplay(wifiStatusToString(WiFi.status()));
 
   IPAddress ip = WiFi.localIP();
   // IPv4 address max length: "255.255.255.255" (15 chars + null terminator);
  snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  jcbDisplay(ipStr);
    // mDNS
  if (MDNS.begin("esp8266")) {
    jcbDisplay("mDNS OK");
  }

  // Web server
  server.on("/", handleRoot);
  server.begin();

  jcbDisplay("Serveur prêt");
  // pour OTA
  ArduinoOTA.setHostname("ESP_Piscine");   // Nom visible sur le réseau

ArduinoOTA.onStart([]() {
    jcbDisplay("Début mise à jour OTA");
});

ArduinoOTA.onEnd([]() {
    jcbDisplay("Fin mise à jour OTA ");
});

ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progression : %u%%\r", (progress * 100) / total);
});

ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erreur[%u]\n", error);

    if (error == OTA_AUTH_ERROR) Serial.println("Erreur authentification");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Erreur début");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Erreur connexion");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Erreur réception");
    else if (error == OTA_END_ERROR) Serial.println("Erreur fin");
});

ArduinoOTA.begin();

jcbDisplay("OTA prêt");
jcbDisplay("Image Not Ok");
  
}

// ================= LOOP =================
void loop() {
  jcbDisplay(ipStr);
  
  ArduinoOTA.handle();
  
  
  server.handleClient();
}
