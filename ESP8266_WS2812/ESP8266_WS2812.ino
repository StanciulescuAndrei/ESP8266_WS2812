#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#ifndef APSSID
#define APSSID "CaciulaLuiStanciu"
#define APPSK  "thereisnospoon"
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

#define PIN            2
#define NUMPIXELS      7
int ledMode = 0;
int lastTime = 0;
int animationDelay = 10;
int hue[NUMPIXELS];
int saturation = 255;
int value[NUMPIXELS];
int dir = 1;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void updatePixels(){
  if(animationDelay > millis() - lastTime)
    return;

  if(ledMode == 1 || ledMode == 2){
    for(int i=0;i<NUMPIXELS;i++){
      value[i] = 255;
      hue[i] = (hue[i] + 500) % 65535;
    }
  }
  else if(ledMode == 3 || ledMode == 4){
    if(value[0] == 255){
      dir = -1;
    }
    if(value[0] == 0 && dir == -1){
      dir = 1;
      for(int i=0;i<NUMPIXELS;i++){
        hue[i] = (hue[i] + 10000) % 65535;
      }
    }
    for(int i=0;i<NUMPIXELS;i++){
      value[i] += 3 * dir;
      if(value[i] > 255)
        value[i] = 255;
      else if(value[i] < 0)
        value[i] = 0;
    }
  }

  for(int i=0;i<NUMPIXELS;i++){
    uint32_t color = (pixels.ColorHSV(hue[i], saturation, value[i]));
    pixels.setPixelColor(i, color);
  }
  pixels.show();
  lastTime = millis();
}

ESP8266WebServer server(80); //Deschide serverul cu portul 80

void handleRoot() {
  //Pagina de pe root
  //server.send(200, "text/html", ServerMainPage);
  loadWebpage("/index.html");
}
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

void handleClick(){
  //Vede ce program e si schimba secventa, apoi redirectioneaza
  if(!server.hasArg("mode") || server.arg("mode") == NULL){
    server.send(400, "text/plain", "400: Invalid Request"); 
    return;
  }
  ledMode = server.arg("mode").toInt();
  Serial.print("LED Mode: ");
  if(ledMode == 1){
    hue[0] = 0;
    hue[1] = 65536 / 6 * 5;
    hue[2] = 65536 /3 * 2;
    hue[3] = 65536 / 2;
    hue[4] = 65536 / 3;
    hue[5] = 65536 / 6;
    hue[6] = 65536 / 2;
    value[0] = value[1] = value[2] = value[3] = value[4] = value[5] = value[6] = 255;
    animationDelay = 10;
  }
  else if(ledMode == 0){
    value[0] = value[1] = value[2] = value[3] = value[4] = value[5] = value[6] = 0;
  }
  else if (ledMode == 2){
    hue[0] = 0;
    hue[1] = 65536 / 6 * 5;
    hue[2] = 65536 /3 * 2;
    hue[3] = 65536 / 2;
    hue[4] = 65536 / 3;
    hue[5] = 65536 / 6;
    hue[6] = 65536 / 2;
    value[0] = value[1] = value[2] = value[3] = value[4] = value[5] = value[6] = 0;
    animationDelay = 5;
  }
  else if(ledMode == 3){
    hue[0] = 0;
    hue[1] = 65536 / 6 * 5;
    hue[2] = 65536 /3 * 2;
    hue[3] = 65536 / 2;
    hue[4] = 65536 / 3;
    hue[5] = 65536 / 6;
    hue[6] = 65536 / 2;
    value[0] = value[1] = value[2] = value[3] = value[4] = value[5] = value[6] = 0;
    animationDelay = 10;
  }
   else if(ledMode == 4){
    hue[0] =  hue[1] =  hue[2] =  hue[3] =  hue[4] =  hue[5] =  hue[6] = 0;
    value[0] = value[1] = value[2] = value[3] = value[4] = value[5] = value[6] = 0;
    animationDelay = 10;
  }
  Serial.println(ledMode);
  server.send(303);
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

void loadWebpage(String path){
  if (SPIFFS.exists(path)) {    
    String contentType = getContentType(path);                        
    File file = SPIFFS.open(path, "r");                 
    size_t sent = server.streamFile(file, contentType); 
    file.close();
    return;
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  SPIFFS.begin();
  
  pixels.begin();
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);  
  server.on("/LED", HTTP_POST, handleClick);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  updatePixels();
}
