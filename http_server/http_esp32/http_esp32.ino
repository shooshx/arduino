
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Time.h>
#include <TimeLib.h>
#include <LittleFS.h>


// Replace with your network credentials
const char* SSID = "TheCatsMew";
const char* PASSW = "abcdeg123458";

//int LED_BUILTIN = 2;

WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String date;
String t;
unsigned long epochTime;
int tzOffset = +3;

const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"}; 

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

void listAllFilesInDir(String dir_path)
{
  File dir = LittleFS.open(dir_path);
  File f = dir.openNextFile();
  while(f) {
    if (!f.isDirectory()) {
      // print file names
      Serial.print("File: ");
      Serial.println(dir_path + f.name() + "  : " + f.size());
    }
    else {
      // print directory names
      Serial.print("Dir: ");
      Serial.println(dir_path + f.name() + "/");
      // recursive file listing inside new directory
      listAllFilesInDir(dir_path + f.name() + "/");
    }
    f = dir.openNextFile();
  }
  dir.close();
}

void exposeFiles(String dir_path)
{
  File dir = LittleFS.open(dir_path);
  File f = dir.openNextFile();
  while(f) {
    if (!f.isDirectory()) {
      // print file names
      Serial.print("Adding handler: ");
      String name = dir_path + f.name();
      Serial.println(name);
      server.serveStatic(name.c_str(), LittleFS, name.c_str());
    }
    else {
      // print directory names
      Serial.print("Dir: ");
      Serial.println(dir_path + f.name() + "/");
      // recursive file listing inside new directory
      exposeFiles(dir_path + f.name() + "/");
    }
    f = dir.openNextFile();
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


void connectWifi()
{
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if(!MDNS.begin("myesp32"))
    Serial.println("Error starting mDNS");
  else
    Serial.println("mDNS responder started");
  
}

void setupNtp()
{
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  //timeClient.setTimeOffset(3600);
}

void handle_hello() {
  Serial.println("got hello");
  server.send(200, "text/plain", "hello world"); 
}

void handle_NotFound(){
  Serial.println("got not found");
  server.send(404, "text/plain", "Not found");
}

void handle_led();
void handle_getTime();

void setupWeb()
{
  server.on("/hello", handle_hello);
  server.on("/led", handle_led);
  server.on("/gettime", handle_getTime);
  server.onNotFound(handle_NotFound);
  exposeFiles("/");

  server.begin();
  Serial.println("HTTP server started");
}

void setup(void)
{
  setupPins();
  setupSerial();
  setupFs();
  connectWifi();

  setupNtp();
  setupWeb();
}

void updateTime()
{
  if (WiFi.status() != WL_CONNECTED)
    return;
  timeClient.update();
  epochTime =  timeClient.getEpochTime();    
}

void loop(void){
  // MDNS.update(); not needed?
  server.handleClient();
  updateTime();

  /*
  digitalWrite(LED_BUILTIN, LOW);  
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(500);
  */
}



void handle_led() 
{
  if (server.arg(0) == "1")
    digitalWrite(LED_BUILTIN, 1);  
  else
    digitalWrite(LED_BUILTIN, 0);  

  server.send(200, "text/plain", String("ok ") + server.arg(0));
}

void handle_getTime()
{
    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;


    local = utc + tzOffset * 3600;

    date = "";  // clear the variables
    t = "";
/*
    t += utc;
    t += "\n";
    t += local;
    t += "\n";
  */  
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
