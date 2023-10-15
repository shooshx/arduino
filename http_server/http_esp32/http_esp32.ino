#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Time.h>
#include <TimeLib.h>
#include <LittleFS.h>

#include <PxMatrix.h>
#include "my_fonts/helvetica_11.h"
#include "my_fonts/diamond_12.h"


// Replace with your network credentials
const char* SSID = "TheCatsMew";
const char* PASSW = "abcdeg123458";

//int LED_BUILTIN = 2;

WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String cur_date;
String cur_time;
unsigned long epochTime;
int tzOffset = +3;

const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"}; 

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);

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


void IRAM_ATTR display_updater(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(70);
  //display.displayTestPattern(70);
  portEXIT_CRITICAL_ISR(&timerMux);
}



uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myRED = display.color565(255, 0, 0);

void setupDisplay() 
{
  display.begin(8);
  
  display.setPanelsWidth(2);

  display.clearDisplay();
  display.flushDisplay();
  
  display.setTextColor(myCYAN);
  display.setCursor(2,0);
  display.print("Pixel");

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &display_updater, true);
  timerAlarmWrite(timer, 4000, true);
  timerAlarmEnable(timer);

  Serial.println("initialized display");


  //delay(1000);
}

void setup(void)
{
  setupPins();
  setupSerial();
  setupFs();
  
  connectWifi();
  setupNtp();
  setupWeb();

  setupDisplay();
}

bool updateTime()
{
  if (WiFi.status() != WL_CONNECTED)
    return false;
  timeClient.update();
  unsigned long prevEpochTime = epochTime;
  epochTime =  timeClient.getEpochTime();
  return (epochTime != prevEpochTime);
}


void timeToStrings()
{
      // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;
    local = utc + tzOffset * 3600;

    cur_date = "";  // clear the variables
    cur_time = "";

    // now format the Time variables into strings with proper names for month, day etc
    cur_date += days[weekday(local)-1];
    cur_date += ", ";
    cur_date += months[month(local)-1];
    cur_date += " ";
    cur_date += day(local);
    cur_date += ", ";
    cur_date += year(local);

    // format the time to 12-hour format with AM/PM and no seconds
    cur_time += hourFormat12(local);
    cur_time += ":";
    int mn = minute(local);
    if(mn < 10)  // add a zero if minute is under 10
      cur_time += "0";
    cur_time += mn;
    cur_time += ":";
    int sec = second(local);
    if(sec < 10)  // add a zero if minute is under 10
      cur_time += "0";
    cur_time += sec;

    cur_time += " ";
    cur_time += ampm[isPM(local)];
}


void printTime()
{
  display.clearDisplay();

  timeToStrings();

  display.setFont(&DiamondRegularRNormal12);
  display.setTextColor(myCYAN);
  display.setCursor(2,2);
  display.print(cur_time);

  display.setFont(&HelveticaRegularRNormal11);
  display.setTextColor(myRED);
  display.setCursor(2,16);
  display.print(cur_date);
}

void loop(void){
  // MDNS.update(); not needed?
  
  server.handleClient();
  bool timeChanged = updateTime();
  
  if (timeChanged)
  {
    printTime();
  }

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
    timeToStrings();

    server.send(200, "text/plain", cur_time + "\n" + cur_date);

}
