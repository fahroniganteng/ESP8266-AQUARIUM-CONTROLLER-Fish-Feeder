/*
 * FN NTP
 * ***************************************************************************************************
 */
void ntpInit(){
  Serial.print("Get NTP Offset : GMT ");
  timeZone = e_gmt.toFloat();
  Serial.println(e_gmt);
  
  Serial.print("Get NTP Server : \"");
  Serial.print(e_ntp);
  Serial.println("\"");

  timeClient.setPoolServerName(e_ntp.c_str());
  timeClient.setTimeOffset(timeZone*60*60); // convert hour to second
  timeClient.setUpdateInterval(60000);// update every 1 minute
  timeClient.update();
  //timeClient.forceUpdate();
  
}

String ntp_datetime;   // RAB, 01 JAN 2020 - 15:04:32
String ntp_time;   // 15:04:32
String ntp_date;   // RAB, 01 JAN 2020
String arr_time[3]; // {"15","04","32"}

// Default on indonesia laguage, change if needed 
char namaHari[7][15] = {"MIN", "SEN", "SEL", "RAB", "KAM", "JUM", "SAB"};
char namaBulan[12][15] = {"JAN", "FEB", "MAR", "APR", "MEI", "JUN", "JUL","AGU","SEP","OKT","NOV","DES"};

long oldLong = 0;
void getNtpDate(){
  unsigned long nowLong;
  
  nowLong = timeClient.getEpochTime();
  //Serial.println(nowLong);
  if(nowLong<=1000000000){//Avoid 
    //Serial.println(nowLong);
    ntp_datetime  = "--no data--";
    ntp_time      = "--no data--";
    ntp_date      = "--no data--";
    arr_time[0]   = "--";
    arr_time[1]   = "--";
    arr_time[2]   = "--";
    if(WiFi.status() == WL_CONNECTED && nowLong - oldLong > 20) {//update time every 20 second if wifi connected
      timeClient.update();
      oldLong = nowLong;
    }
  }
  else{
    struct tm * ptm;
    char buf[50];
    ptm = gmtime ((time_t *)&nowLong);
    snprintf(buf, 50, "%s, %02d %s %d %02d:%02d:%02d", namaHari[ptm->tm_wday], ptm->tm_mday, namaBulan[ptm->tm_mon], ptm->tm_year + 1900, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    ntp_datetime =  String(buf);
    snprintf(buf, 50, "%s, %02d %s %d", namaHari[ptm->tm_wday], ptm->tm_mday, namaBulan[ptm->tm_mon], ptm->tm_year + 1900);
    ntp_date =  String(buf);
    snprintf(buf, 50, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    ntp_time =  String(buf);

    arr_time[0]   = ntp_time.substring(0,2);
    arr_time[1]   = ntp_time.substring(3,5);
    arr_time[2]   = ntp_time.substring(6,8);
  }
  
//  snprintf(buf, sizeof(buf), "%02d", ptm->tm_min);
//  Serial.printf("%s, %d %s %d %02d:%02d:%02d \r\n",
//    namaHari[ptm->tm_wday],
//    ptm->tm_mday,
//    namaBulan[ptm->tm_mon],
//    ptm->tm_year + 1900,
//    ptm->tm_hour,
//    ptm->tm_min,
//    ptm->tm_sec
//  );

  
}
