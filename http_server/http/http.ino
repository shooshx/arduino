//#define DEBUG_ESP_HTTP_SERVER 
//#define DEBUG_ESP_PORT Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>   // Include the WebServer library

#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

//#include <ESPAsyncWebSrv.h>
#include <LittleFS.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

//AsyncWebServer server(80);

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "il.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

String date;
String t;
unsigned long epochTime;

const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"}; 


void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();
void handleLed();
void handleGetTime();

void setupPins()
{
  pinMode(LED_BUILTIN, OUTPUT);
}

void setupSerial()
{
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
}

void connectWifi()
{
  wifiMulti.addAP("TheCatsMew", "abcdeg123458");   // add Wi-Fi networks you want to connect to
  //wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
 //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
}

void listAllFilesInDir(String dir_path)
{
  Dir dir = LittleFS.openDir(dir_path);
  while(dir.next()) {
    if (dir.isFile()) {
      // print file names
      Serial.print("File: ");
      Serial.println(dir_path + dir.fileName() + "  : " + dir.fileSize());
    }
    if (dir.isDirectory()) {
      // print directory names
      Serial.print("Dir: ");
      Serial.println(dir_path + dir.fileName() + "/");
      // recursive file listing inside new directory
      listAllFilesInDir(dir_path + dir.fileName() + "/");
    }
  }
}

void setupFs()
{
    if(!LittleFS.begin()){
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
    }
    listAllFilesInDir("/");
}

void exposeFiles(String dir_path)
{
  Dir dir = LittleFS.openDir(dir_path);
  while(dir.next()) {
    if (dir.isFile()) {
      // print file names
      Serial.print("Adding handler: ");
      String name = dir_path + dir.fileName();
      Serial.println(name);
      server.serveStatic(name.c_str(), LittleFS, name.c_str());
    }
    if (dir.isDirectory()) {
      // print directory names
      Serial.print("Dir: ");
      Serial.println(dir_path + dir.fileName() + "/");
      // recursive file listing inside new directory
      exposeFiles(dir_path + dir.fileName() + "/");
    }
  }
}

void setup(void)
{
  setupPins();
  setupSerial();
  setupFs();

  connectWifi();
  timeClient.begin();   // Start the NTP UDP client
 
  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/led", handleLed);
  server.on("/gettime", handleGetTime);
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  exposeFiles("/");

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void updateTime()
{
  if (WiFi.status() != WL_CONNECTED)
    return;
  timeClient.update();
  epochTime =  timeClient.getEpochTime();    
}

void loop(void){
  MDNS.update();
  server.handleClient();                    // Listen for HTTP requests from clients
  updateTime();
}


void handleRoot() {
  server.send(200, "text/plain", "Hello world!");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleLed() 
{
  if (server.arg(0) == "1")
    digitalWrite(LED_BUILTIN, 1);  
  else
    digitalWrite(LED_BUILTIN, 0);  

  server.send(200, "text/plain", String("ok ") + server.arg(0));
}

void handleGetTime()
{
    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;

    TimeChangeRule ilIST = {"IST", Last, Fri, Mar, 2, +120};  //UTC + 2 hours - change this as needed
    TimeChangeRule isIDT = {"IDT", Last, Sun, Oct, 2, +180};   //UTC + 3 hours - change this as needed
    Timezone usEastern(ilIST, isIDT);
    local = usEastern.toLocal(utc);

    date = "";  // clear the variables
    t = "";
    // now format the Time variables into strings with proper names for month, day etc
    date += days[weekday(local)-1];
    date += ", ";
    date += months[month(local)-1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    t += hourFormat12(local);
    t += ":";
    int mn = minute(local);
    if(mn < 10)  // add a zero if minute is under 10
      t += "0";
    t += mn;
    t += ":";
    int sec = second(local);
    if(sec < 10)  // add a zero if minute is under 10
      t += "0";
    t += sec;

    t += " ";
    t += ampm[isPM(local)];

    server.send(200, "text/plain", t + "\n" + date);

}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
