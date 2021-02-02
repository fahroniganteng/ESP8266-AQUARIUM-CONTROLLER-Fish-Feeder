/*
 * FN WIFI
 * *************************************************************************************
 */
String listWifi;  // for available wifi list
void scanWifi(){
  listWifi = "";
  int n = WiFi.scanNetworks();
  PRINTLN("scan done");
  if (n == 0){
    PRINTLN("no networks found");
  }
  else {
    PRINT(n);
    PRINTLN(" networks found");
    for (int i = 0; i < n; ++i){
      // Print SSID and RSSI for each network found
      PRINT(i + 1);
      PRINT(": ");
      PRINT(WiFi.SSID(i));
      PRINT(" (");
      PRINT(WiFi.RSSI(i));
      PRINT(")");
      PRINTLN((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
    for (int i = 0; i < n; ++i){
      // Print SSID and RSSI for each network found
      String signalRSSI;
      if(WiFi.RSSI(i)>=-50)  signalRSSI ="&#9679;&#9679;&#9679;&#9679;&#9679;";
      else if (WiFi.RSSI(i)>=-60)  signalRSSI ="&#9679;&#9679;&#9679;&#9679;&#9675;";
      else if (WiFi.RSSI(i)>=-67)  signalRSSI ="&#9679;&#9679;&#9679;&#9675;&#9675;";
      else if (WiFi.RSSI(i)>=-70)  signalRSSI ="&#9679;&#9679;&#9675;&#9675;&#9675;";
      else signalRSSI ="&#9679;&#9675;&#9675;&#9675;&#9675;";
      
      listWifi += "<li class='list-group-item'><span>";
      listWifi += WiFi.SSID(i);
      listWifi += "</span><sup> ";
      listWifi += signalRSSI;
      listWifi += "</sup>";
      //listWifi += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?"no password":"with password";
      listWifi += "</li>";
    }
  }
  PRINTLN("");
  delay(100);
}

void setupAP(void) {//access point MODE
  if (WiFi.status() == WL_CONNECTED) WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.hostname(wifiHostName);
  delay(10);
  //scanWifi();
  PRINT("AP Config... ");
  PRINTLN(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  //WiFi.softAP(ssid, passphrase);
  PRINT("AP Start... ");
  if(WiFi.softAP(ssid, passphrase)){
    PRINTLN("Ready" );
  }
  else {
    PRINTLN("Failed! Coba ulang...");
    setupAP();
  }
}

void connectWifi(){
  if (WiFi.status() == WL_CONNECTED) WiFi.disconnect();
  PRINT("ssid : ");
  PRINTLN(e_ssid);
  if(e_ssid=="__DEFAULT_WIFI__"){
    PRINTLN("SSID belum diisi, mode access point..");
    setupAP();
    PRINT("SoftAP IP: ");
    PRINTLN(WiFi.softAPIP());
    tmr.oscillate(LED_BUILTIN, 50, HIGH, 5);
    delay(100);
    return;
  }
  PRINT("pass : ");
  PRINTLN(e_pass);
//  PRINTLN("********");

  WiFi.mode(WIFI_STA); //wifi mode ==> WIFI_OFF, WIFI_STA, WIFI_AP, WIFI_AP_STA
  WiFi.hostname(wifiHostName);
  int connectLoop = 0;
  WiFi.begin(e_ssid.c_str(), e_pass.c_str());
  while (WiFi.status() != WL_CONNECTED && connectLoop<=50) { //20*500ms = 10 second max connect time
    delay(200);
    PRINT(".");
    connectLoop++;
  }
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    PRINTLN("Cannot connect WiFi");
    setupAP();
    PRINT("SoftAP IP: ");
    PRINTLN(WiFi.softAPIP());
    tmr.oscillate(LED_BUILTIN, 50, HIGH, 5);
    delay(100);
  }
  else{
    PRINTLN();
    PRINTLN("WiFi connected");
    PRINTLN("IP address: ");
    String locIP = WiFi.localIP().toString();
    PRINTLN(locIP);
    tmr.oscillate(LED_BUILTIN, 50, HIGH, 3);
  }
}
void reConnectWifi(){
  if (WiFi.status() != WL_CONNECTED && e_ssid!="__DEFAULT_WIFI__") connectWifi();
}
