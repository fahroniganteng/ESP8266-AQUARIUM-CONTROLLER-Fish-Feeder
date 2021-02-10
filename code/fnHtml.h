/*
   FN SPIFFS
 * **************************************************************************
   Load data (file html and friend) from flash memory
   Use arduino-esp8266fs-plugin for upload HTML file to ESP8266 board
   library ==> https://github.com/esp8266/arduino-esp8266fs-plugin
*/

bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.html"; //redirect http://localhost/ ==> http://localhost/index.html

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";

  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    //nothing
  }

  dataFile.close();
  return true;
}





/*
   HTML Request handle
 * *****************************************************************************************************
   this function for handle HTML request (from web browser)
*/

// fn make json
String jsonData = "";
void newJson() {
  jsonData = "";
}
void toJson(String key, String val, boolean isJson = false) {
  jsonData += isJson ?
              "\"" + key + "\"" + ":" + val + "," :
              "\"" + key + "\"" + ":" + "\"" + val + "\"" + ",";
}
String getJson() {
  String buf = "";
  if (jsonData.length() < 1)
    buf = "{}";
  else {
    buf = "{" + jsonData;
    buf[buf.length() - 1] = '}';
  }
  jsonData = "";
  return buf;
}
void jsonFeedback(bool sukses, String msg) { //Feedback to html (JSON)
  String json = "{\"success\":\"" + String(sukses) + "\", \"data\": " + getJson() + ", \"message\":\"" + msg + "\"}";
  jsonData = "";
  //Serial.println(json);
  server.send(200, "application/json", json);
}


// fn handle http req.
void serverOnHandleClient() {
  server.on("/", []() {
    server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
    server.send(302, "text/plane", "");
  });

  server.on("/homePage", []() {
    String buf[10];//buffer json
    newJson();

    // 0. CONNECTION ------------------
    toJson("Wifi", (WiFi.status() == WL_CONNECTED) ? e_ssid : "Not connect");
    buf[0] = getJson();

    // 1. TIME ------------------------
    toJson("Date", ntp_date);
    toJson("Time", ntp_time);
    toJson("NTP Server", e_ntp);

    //re-format GMT
    String zonaWaktu = ((int)timeZone < timeZone) ?
                       (timeZone > 0 ? "GMT +" : "GMT") + String((int)timeZone) + ":30" :
                       (timeZone > 0 ? "GMT +" : "GMT") + String((int)timeZone);
    toJson("Time zone", zonaWaktu);

    buf[1] = getJson();

    // 2. LAMP ------------------------
    if (WiFi.status() == WL_CONNECTED && ntp_datetime != "--no data--") {
      toJson("Schedule ON", lamp_time[1]);
      toJson("Schedule OFF", lamp_time[0]);
    }
    toJson("Brightness", "Level " + String(lamp_brig));
    //toJson("Mode","Mode "+String(lamp_mode));
    //toJson("Speed",String(lamp_speed)+" second");
    buf[2] = getJson();

    // 3. FEED ------------------------
    toJson("How much", String(feed_loop) + "x Level " + String(feed_level));
    if (WiFi.status() != WL_CONNECTED || ntp_datetime == "--no data--" ) {
      toJson("Offline mode", "Feed every " + String(feed_duration) + " hour");
      toJson("Next feed", nextFeed);
    }
    else {
      toJson("Schedule 1", feed_time[0]);
      if (feed_time[1] != "") toJson("Schedule 2", feed_time[1]);
      if (feed_time[2] != "") toJson("Schedule 3", feed_time[2]);
      if (feed_time[3] != "") toJson("Schedule 4", feed_time[3]);
      if (feed_time[4] != "") toJson("Schedule 5", feed_time[4]);
    }
    buf[3] = getJson();

    // 4. UP TIME ---------------------
    toJson("Up Time", shUpTime);
    buf[4] = getJson();

    // 5. OTHER DATA ------------------
    toJson("lamp_status", String(lamp_status));
    toJson("pump_status", String(pump_status));
    buf[5] = getJson();

    // WRITE FEEDBACK
    toJson("Fish Feeder", buf[3], true);
    toJson("Lamp", buf[2], true);
    toJson("Connection", buf[0], true);
    toJson("Date Time", buf[1], true);
    toJson("Device Info", buf[4], true);
    toJson("other", buf[5], true);
    jsonFeedback(true, "");
  });

  server.on("/ntpPage", []() {
    newJson();
    toJson("showTime", ntp_datetime);
    toJson("ntpServer", e_ntp);
    toJson("timeZone", String(timeZone));
    jsonFeedback(true, "");
  });

  server.on("/lampPage", []() {
    newJson();
    toJson("lamp_on", lamp_time[1]);
    toJson("lamp_off", lamp_time[0]);
    toJson("lamp_mode", String(lamp_mode));
    toJson("lamp_brightness", String(lamp_brig));
    toJson("lamp_speed", String(lamp_speed));
    toJson("lamp_color", lamp_color);
    jsonFeedback(true, "");
  });
  server.on("/saveLampSettings", []() {
    String buf[4];
    bool completeForm = true;
    buf[0]  = server.arg("lamp_mode");
    buf[1]  = server.arg("lamp_color");
    buf[2]  = server.arg("lamp_brightnessOut");
    buf[3]  = server.arg("lamp_speedOut");
    for (int i = 0; i < arrSize(buf); i++) {
      if (buf[i].length() == 0) {
        completeForm = false;
        break;
      }
    }

    if (completeForm) {
      lamp_mode     = buf[0].toInt();
      lamp_color    = buf[1];
      lamp_brig     = buf[2].toInt();
      lamp_speed    = buf[3].toInt();
      writeEEPROM("lamp_mode", buf[0]); delay(10);
      writeEEPROM("lamp_color", buf[1]); delay(10);
      writeEEPROM("lamp_brig", buf[2]); delay(10);
      writeEEPROM("lamp_speed", buf[3]); delay(10);
      fn_RGBstart();
      jsonFeedback(true, "Saved...");
    }
    else
      jsonFeedback(false, "Incomplete form");
  });
  server.on("/saveLampSchedule", []() {
    String buf[2];
    bool completeForm = true;
    buf[0]  = server.arg("lamp_off");
    buf[1]  = server.arg("lamp_on");
    for (int i = 0; i < arrSize(buf); i++) {
      if (buf[i].length() == 0) {
        completeForm = false;
        break;
      }
    }
    if (completeForm) {
      lamp_time[0]  = buf[0];
      lamp_time[1]  = buf[1];
      writeEEPROM("lamp_off", buf[0]); delay(10);
      writeEEPROM("lamp_on", buf[1]); delay(10);
      jsonFeedback(true, "Saved...");
    }
    else
      jsonFeedback(false, "Incomplete form");
  });




  server.on("/fishFeederPage", []() {
    newJson();
    toJson("feed_level", String(feed_level));
    toJson("feed_loop", String(feed_loop));
    toJson("feed1", feed_time[0]);
    toJson("feed2", feed_time[1]);
    toJson("feed3", feed_time[2]);
    toJson("feed4", feed_time[3]);
    toJson("feed5", feed_time[4]);
    toJson("feed_duration", String(feed_duration));
    jsonFeedback(true, "");
  });
  server.on("/saveFeederSettings", []() {
    String dt_feed_level = server.arg("feed_levelOut");
    String dt_feed_loop  = server.arg("feed_loopOut");
    if (dt_feed_level.length() > 0 && dt_feed_loop.length() > 0) {
      Serial.print("writing eeprom feed_level:");
      Serial.println(dt_feed_level);
      writeEEPROM("feed_level", dt_feed_level);
      feed_level = dt_feed_level.toInt();
      delay(10);
      Serial.print("writing eeprom feed_loop:");
      Serial.println(dt_feed_loop);
      writeEEPROM("feed_loop", dt_feed_loop);
      feed_loop = dt_feed_loop.toInt();
      delay(10);
      jsonFeedback(true, "Saved...");
    }
    else
      jsonFeedback(false, "Incomplete form");
  });
  server.on("/saveOnlineFeedSchedule", []() {
    String buf;
    bool success = true;
    for (int i = 0; i < 5; i++) {
      buf  = server.arg("feed" + String(i + 1));
      if (i == 0 && buf.length() != 5) { //schedule 1 required
        success = false;
        break;
      }
      buf = buf.length() != 5 ? "" : buf;
      if (feed_time[i] != buf) {
        Serial.println("writing eeprom feed" + String(i + 1) + ":" + buf);
        writeEEPROM("feed" + String(i + 1), buf);
        feed_time[i] = buf;
        delay(10);
      }
    }
    success ?
    jsonFeedback(true, "Saved...") :
    jsonFeedback(false, "Incomplete form");
  });
  server.on("/saveOfflinesetting", []() {
    String dt_feed_duration = server.arg("feed_durationOut");
    if (dt_feed_duration.length() > 0) {
      Serial.print("writing eeprom feed_duration:");
      Serial.println(dt_feed_duration);
      writeEEPROM("feed_duration", dt_feed_duration);
      feed_duration = dt_feed_duration.toInt();
      delay(10);
      jsonFeedback(true, "Saved...");
    }
    else
      jsonFeedback(false, "Incomplete form");
  });


  server.on("/wifiPage", []() {
    scanWifi();
    toJson("apName", String(ssid));
    toJson("apPass", String(passphrase));
    toJson("wifiAvailable", String(listWifi));
    toJson("currentConnection", (WiFi.status() == WL_CONNECTED) ? e_ssid : "Not Connected to wifi..");
    jsonFeedback(true, "");
  });

  server.on("/forceUpdateNtp", []() {
    timeClient.forceUpdate();
    jsonFeedback(true, "Request send.");
  });

  server.on("/saveNTPSetting", []() {
    String dt_ntpServer   = server.arg("ntpServer");
    String dt_timeZone    = server.arg("timeZone");
    if (dt_ntpServer.length() > 0 && dt_timeZone.length() > 0) {
      Serial.print("ntpServer : ");
      Serial.println(dt_ntpServer);
      Serial.print("timeZones : ");
      Serial.println(dt_timeZone);

      Serial.println("writing eeprom ntp:");
      writeEEPROM("ntp", dt_ntpServer);
      e_ntp = dt_ntpServer;
      delay(10);
      Serial.println("writing eeprom gmt:");
      writeEEPROM("gmt", dt_timeZone);
      e_gmt = dt_timeZone;
      delay(10);
      ntpInit();
      jsonFeedback(true, "Saved...");
    }
    else
      jsonFeedback(false, "Incomplete form");
  });

  server.on("/btnPUMP", []() {
    pump_status = pump_status == 1 ? 0 : 1;
    digitalWrite(PIN_PUMP, pump_status);
    jsonFeedback(true, (pump_status == 1 ? "Pump started" : "Pump stopped"));
  });
  server.on("/btnLAMP", []() {
    lamp_status = lamp_status == 1 ? 0 : 1;
    if (lamp_status == 1) {
      fn_RGBstart();
      jsonFeedback(true, "Turn ON lamp");
    }
    else {
      fn_RGBstop();
      jsonFeedback(true, "Turn OFF lamp");
    }

  });
  server.on("/btnFEED", []() {
    feedFish();
    jsonFeedback(true, "Feed the fish");
  });

  server.on("/saveConnection", []() {
    String qssid   = server.arg("ssid");
    String qpass  = server.arg("pass");
    if (qssid.length() > 0) {
      Serial.print("SSID :");
      Serial.println(qssid);
      Serial.print("Password : ");
      Serial.println(qpass);

      Serial.println("writing eeprom ssid:");
      writeEEPROM("ssid", qssid);
      e_ssid = qssid;
      delay(10);
      Serial.println("writing eeprom pass:");
      writeEEPROM("pass", qpass);
      e_pass = qpass;
      delay(10);

      jsonFeedback(true, "Saved, to connect<br>Please reboot the device manually if fails");
      delay(100);
      connectWifi();
      delay(10);
    }
    else {
      jsonFeedback(false, "Incomplete form");
    }
  });


  server.on("/reConnect", []() {
    jsonFeedback(true, "Try to connect<br>Please reboot the device manually if fails");
    delay(100);
    connectWifi();
  });
  server.on("/resetWifiSetting", []() {
    Serial.println("clearing eeprom");
    for (int i = 0; i < 96; ++i) {
      EEPROM.write(i, 0);
    }
    e_ssid          = "__DEFAULT_WIFI__";
    Serial.println("writing eeprom default ssid:");
    writeEEPROM("ssid", e_ssid);
    delay(10);
    Serial.println("Remove wifi pass (EEPROM)");
    writeEEPROM("pass", "********");
    jsonFeedback(true, "settings have been reset, try rebooting<br>Please reboot the device manually if fails");
    delay(100);
    //connectWifi();

    delay(1000);
    //ESP.reset();  // register saved
    ESP.restart();  // clean reboot
  });
  server.on("/rebootBoard", []() {
    jsonFeedback(true, "Try rebooting<br>Please reboot the device manually if fails");
    delay(1000);
    //ESP.reset();  // register saved
    ESP.restart();  // clean reboot
  });

  server.onNotFound([]() {
    if (loadFromSpiffs(server.uri())) return;
    //writeWebServer();
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    Serial.println(message);
    server.send(404, "text/plain", message);
  });
}
