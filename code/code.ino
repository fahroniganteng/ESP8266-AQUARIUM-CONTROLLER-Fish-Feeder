/*
 * AQUARIUM CONTROLLER
 * *********************************************************************************
 * Code by : fahroni|ganteng 
 * Contact me : fahroniganteng@gmail.com
 * License : MIT
 * --------------------------
 * 
 * PIN use on WEMOS D1 Mini
 * D1 : Pin RED led strip
 * D2 : Pin Green led strip
 * D3 : Pin Blue led strip
 * D4 = LED_BUILTIN : not connect ==> for indicator light
 * D7 : Servo
 * D8 : Relay
 * 
 */


#include "NTPClient.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "Timer.h"
#include "time.h"
#include <FS.h>   //Include File System Headers
#include <Servo.h>




//Maping PIN ESP8266 board
const int PIN[9] = {D0,D1,D2,D3,D4,D5,D6,D7,D8};// all digital pin ESP8266
const int PIN_LED[3] = {D1,D2,D3}; // to RGB PIN --> LED STRIP is common cathode (-), but if use mosfet will be inverse --> analog write 0 = off, 255 = max brightness
#define PIN_SERVO D7 // to servo
#define PIN_PUMP D8 // to relay

// speed servo ==> ms/degree
#define SERVOSPEED 2.5

// fn get size of int array
#define arrSize(x)   (sizeof(x) / sizeof(x[0]))

// fn Calculate time
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

#define getSEC(_time_) (_time_ % SECS_PER_MIN)  
#define getMIN(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define getHOUR(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define getDAY(_time_) ( _time_ / SECS_PER_DAY)

// Debuging
#define DEBUG 0   // Enable or disable ==> set 1 to enable serial debuging
#if DEBUG
  #define PRINT(s)     { Serial.print(s); }
  #define PRINTLN(s)     { Serial.println(s); }
#else
  #define PRINT(s)
  #define PRINTLN(s)
#endif

//Setting soft AP
const char* ssid = "AQUARIUM";
const char* passphrase = "";//blank = no password
const char* wifiHostName = "AQUARIUM"; //host name device (like computer name if read from network)
IPAddress local_IP(10,10,10,10);
IPAddress gateway(10,10,10,10);
IPAddress subnet(255,255,255,0);
//web server
ESP8266WebServer server(80);

// servo
Servo s;


/*
 * GLOBAL VARIABLE
 * ******************************************************************************************
 */
//Timer ---------------------------------------------------------------
Timer tmr;

//UP time -------------------------------------------------------------
long secUpTime = 0; // counter uptime in second --> default first boot = 0 second
String shUpTime;    // show UP Time (view up time)


//NTP & WIFI ----------------------------------------------------------
WiFiUDP ntpUDP;     // send request time to NTP server
NTPClient timeClient(ntpUDP); 
float timeZone;     // buffer time zona --> if in half hour like GMT -3:30 = -3.5 --> so use float
String e_ssid; // buf SID wifi name
String e_pass; 
String e_ntp; 
String e_gmt; 

// FISH FEEDER ---------------------------------------------------------
String feed_time[5]; // array buffer eeprom feed time {feed1, feed2, ... feed5)
int feed_level,feed_loop,feed_duration; // buffer feed
String nextFeed;

// LAMP ----------------------------------------------------------------
String lamp_time[2];//{off,on}
String lamp_color;
int lamp_status = 1,lamp_brig,lamp_mode,lamp_speed;

// PUMP ----------------------------------------------------------------
int pump_status = 1;

/*
 * NOTE :
 * lamp and pump always start on boot...
 */




/*
 * FUNCTION
 * *****************************************************************************************
 */
// FEEDER -------------------
int secondAfterFeeder = 0;
void feedFish(){
  int moveDegree = map(feed_level,1,10,30,180);
  int moveTime   = SERVOSPEED * moveDegree;
  moveTime       = moveTime<150?150:moveTime;//minimal 150ms
  for(int i=0;i<feed_loop;i++){
    PRINTLN("Feed fish "+String(moveDegree)+" deg");
    s.write(moveDegree);
    delay(moveTime);
    s.write(0);
    delay(moveTime);
  }
  secondAfterFeeder = 0;
}

// UP TIME -------------------
void uptime(){
  char buf[50];//buffer
  secUpTime++; // max long datatype = 2,147,483,647 second = 69 year
  //secUpTime = secUpTime<0?0:secUpTime; // after 2,147,483,647 + 1 = -2147483648 ==> impossible run until 69 year ^___^
  
  snprintf(buf, 50, "%ld Day %02d:%02d:%02d",getDAY(secUpTime),getHOUR(secUpTime),getMIN(secUpTime),getSEC(secUpTime));
  shUpTime = String(buf);
}

// MOVE TO OTHER FILE --------------------------
#include "fnEEPROM.h" // mapping eeprom
#include "fnWifi.h"   // function conection wifi
#include "fnNTP.h"    // get time from NTP
#include "fnLamp.h"   // Lamp handle
#include "fnHtml.h"   // for handle HTML request (from web browser)



// CHECK EVERY 1 SECOND -----------------------
int countApMode = 0;

void check_1s(){
  getNtpDate(); //check NTP
  uptime(); // check uptime
  secondAfterFeeder++;
  
  // IF OFFLINE OR not connect NTP server
  if (WiFi.status() != WL_CONNECTED || ntp_datetime == "--no data--" ){ 
    // run offline fish feeder
    if(feed_duration * 60 * 60 < secondAfterFeeder)
      feedFish();

    // Offline next feed
    char buf[50];//buffer
    int secNextFeed = feed_duration * 60 * 60 - secondAfterFeeder;
    snprintf(buf, 50, "%02d:%02d:%02d",getHOUR(secNextFeed),getMIN(secNextFeed),getSEC(secNextFeed));
    nextFeed = buf;

    // try connect every 60 second, if SSID in EEPROM is set. ==> not set = "__DEFAULT_WIFI__"
    if(e_ssid!="__DEFAULT_WIFI__" && countApMode > 60){
      connectWifi();
      countApMode=0; // counting form zero again, until 60 second try connect again...
    }
    else 
      countApMode++;
  }
  else {
    //check every minutes (ONLINE MODE)
    if(arr_time[2]=="00"){
      String buf = arr_time[0] + ":" + arr_time[1];
      // check feed schedule 
      for(int i=0; i<5; i++){
        if(feed_time[i] == buf){
          feedFish();
          break;
        }
      }
      //check lamp schedule ==> if schedule ON same with OFF ==> lamp always ON
      if(lamp_time[1]==buf)fn_RGBstart();
      else if(lamp_time[0]==buf)fn_RGBstop();
    }
    countApMode=0;
  }
}











void setup() {
  //for debug
  Serial.begin(115200);

  //read EEPROM data
  EEPROM.begin(512);
  readAllDataEEPROM();

  // for file system (html and friends on flash memory)
  SPIFFS.begin();

  // set all digital pin OUTPUT
  for(int i=0; i<arrSize(PIN); i++){
    pinMode(PIN[i], OUTPUT);
    digitalWrite(PIN[i], LOW);
  }

  // Servo PIN
  s.attach(PIN_SERVO);
  s.write(0);
  
  // Turn ON Pump
  digitalWrite(PIN_PUMP,HIGH);

  // Turn ON Lamp
  fn_RGBstart();
  
  //Wifi start
  WiFi.persistent(false);
  PRINTLN("Connecting wifi...");
  connectWifi();
  
  //start http server
  server.begin();
  serverOnHandleClient();
  PRINTLN("HTTP server started");
  
  //turn off LED_BUILTIN; --> LED_BUILTIN = D4 (WEMOS D1 MINI)
  digitalWrite(LED_BUILTIN,HIGH);// HIGH = off --> i dont know why... :D
  
  //NTP
  ntpInit();
  delay(1000);
  tmr.every(3600000,[](){//force update NTP every 1 hour
    timeClient.forceUpdate();
  });

  // Check every 1 second
  delay(100);
  tmr.every(1000,check_1s);
}
void loop() {
  tmr.update(); // update timer
  server.handleClient(); // check http client request
}
