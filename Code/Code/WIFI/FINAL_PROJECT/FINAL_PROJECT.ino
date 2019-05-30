
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <Ticker.h> 
Ticker delay_in_sec;
int Level_set=0;
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
void handleRequest();
void handleLEDON();
void handleLEDOFF();
void handleLEDDIM();
void handleFANON();
void handleFANOFF();
void handleFANSPEED();
void handleREFRESH();

const char *ssid = "EMBED21G"; // The name of the Wi-Fi network that will be created
const char *password = "itisfine";   // The password required to connect to it, leave blank for an open network

const int out = 16; // GPIO16 (or) D0
String dimlevel = "50";
String ledstatus = "F";
String fanstatus = "F";
String fanspeed = "25";
//int Delay_set =0;///////////////////////////////
//int Level_set =0;///////////////////////////////

void ICACHE_RAM_ATTR onTimerISR(){
 
  if (Level_set < 0) dimlevel = "0";
  else if(Level_set > 100) dimlevel = "100";
  else dimlevel = String(Level_set);
  handleRequest();
    //timer1_write(600000);//
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(10);

 
  
  pinMode(out, OUTPUT);

  WiFi.softAP(ssid, password);             // Start the access point

  SPIFFS.begin();                           // Start the SPI Flash Files System
  
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.on("/LEDON", HTTP_POST, handleLEDON);  // Call the 'handleLEDON' function when a POST request is made to URI "/LEDON"
  server.on("/LEDOFF", HTTP_POST, handleLEDOFF);  // Call the 'handleLEDOFF' function when a POST request is made to URI "/LEDOFF"
  server.on("/LEDDIM", HTTP_POST, handleLEDDIM);  // Call the 'handleLEDDIM' function when a POST request is made to URI "/LEDDIM"
  server.on("/FANON", HTTP_POST, handleFANON);  // Call the 'handleLEDON' function when a POST request is made to URI "/LEDON"
  server.on("/FANOFF", HTTP_POST, handleFANOFF);  // Call the 'handleLEDOFF' function when a POST request is made to URI "/LEDOFF"
  server.on("/FANSPEED", HTTP_POST, handleFANSPEED);  // Call the 'handleLEDDIM' function when a POST request is made to URI "/LEDDIM"
  server.on("/REFRESH", HTTP_POST, handleREFRESH);  // Call the 'handleREFRESH' function when a POST request is made to URI "/REFRESH"
  server.on("/DELAYSET", HTTP_POST, handleDelaySet);  ///////////////////////////////////////
  //server.on("/LEVELSET", HTTP_POST, handleLevelSet);  ///////////////////////////////////////

  server.begin();                           // Actually start the server

  Serial.flush();
  Serial1.flush();
  delay(500);
  Serial.println("Device Ready");
  Serial.println(WiFi.softAPIP());
  delay(500);
  Serial1.print(ledstatus+"_"+dimlevel+"_"+fanstatus+"_"+fanspeed+"_"+ipToString(WiFi.softAPIP())+"#");
  digitalWrite(out,LOW);
}

void loop(void) {
  server.handleClient();
  delay(5);
}

void handleLEDON() {                          // If a POST request is made to URI /LEDON
  ledstatus = "O";
  handleRequest();
}

void handleLEDOFF() {                          // If a POST request is made to URI /LEDOFF
  ledstatus = "F";
  handleRequest();
}

void handleFANON() {                          // If a POST request is made to URI /FANON
  fanstatus = "O";
  handleRequest();
}

void handleFANOFF() {                          // If a POST request is made to URI /FANOFF
  fanstatus = "F";
  handleRequest();
}

void handleLEDDIM(){
  
  int level = (server.arg("LEDDIMLevel")).toInt();
  if (level < 0) dimlevel = "0";
  else if(level > 100) dimlevel = "100";
  else dimlevel = String(level);
  handleRequest();
}
//void handleLevelSet(){
//  int Level_set = (server.arg("LEDDIMLevel_Set")).toInt();///////////////////////////////////
//  if (Level_set < 0) dimlevel = "0";
//  else if(Level_set > 100) dimlevel = "100";
// else dimlevel = String(Level_set);
//  handleRequest();
//}


void handleDelaySet(){/////////////////////////////////////////////////
  int Delay_set = (server.arg("LEDDIMDelay_Set")).toInt();////////////////////////////////////
   Level_set = (server.arg("LEDDIMLevel_Set")).toInt();///////////////////////////////////
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(1200000*Delay_set); //1s multiple
  //delay_in_sec.attach(Delay_set, handleLEDDIM);               //Handler/////////////////////////////
  handleRequest();
}

void handleFANSPEED(){
  int level = (server.arg("FANSppedLevel")).toInt();
  if (level < 0) fanspeed = "0";
  else if(level > 100) fanspeed = "100";
  else fanspeed = String(level);
  handleRequest();
}

void handleREFRESH(){
  handleRequest();
}

String ipToString(IPAddress ip){
  String s = "";
  for (int i=0; i<4; i++){
    s += i ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

void handleRequest() {                          // If a POST request is made to URI /LED
  Serial1.print(ledstatus+"_"+dimlevel+"_"+fanstatus+"_"+fanspeed+"_"+ipToString(WiFi.softAPIP())+"#");
  Serial.println(ledstatus+"_"+dimlevel+"_"+fanstatus+"_"+fanspeed+"_"+ipToString(WiFi.softAPIP())+"#");
  server.send(200, "text/plain", ledstatus+"_"+dimlevel+"_"+fanstatus+"_"+fanspeed); 
  Serial1.flush();
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  return false;                                         // If the file doesn't exist, return false
}
