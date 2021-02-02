/*
 * EEPROM
 * ***********************************************************************************************
 * Use 512 ==> EEPROM.begin(512) 
 * Mapping address EEPROM (string)
 * --------------------------------------------------------------------
 * 0    - 99    --> use later (spare)
 * 100  - 149   --> ssid  --> SSID Wifi
 * 150  - 199   --> pass  --> Password Wifi
 * 200  - 249   --> ntp   --> ntp server
 * 250  - 259   --> gmt   --> time zone (GMT +XX) -12 ~ +12
 * etc...
 * --------------------------------------------------------------------
 */

#define START 0   // Address EEPROM
#define LENGTH 1  // length EEPROM
int eepr[2];
void * getListEEPROM(String e){
  // STRING DATA --------------------------------------------------------------------
  if (e == "ssid")      {eepr[START]=100; eepr[LENGTH]=50;}
  else if (e == "pass") {eepr[START]=150; eepr[LENGTH]=50;}
  else if (e == "ntp")  {eepr[START]=200; eepr[LENGTH]=50;}
  else if (e == "gmt")  {eepr[START]=250; eepr[LENGTH]=10;}
  
  //fish feeder time --> max 5x per day
  else if (e == "feed1") {eepr[START]=260; eepr[LENGTH]=10;}//00:00 = 5 digit --> set 10, if check length != 5 set default time
  else if (e == "feed2") {eepr[START]=270; eepr[LENGTH]=10;}
  else if (e == "feed3") {eepr[START]=280; eepr[LENGTH]=10;}
  else if (e == "feed4") {eepr[START]=290; eepr[LENGTH]=10;}
  else if (e == "feed5") {eepr[START]=300; eepr[LENGTH]=10;}
  
  // schedule on/off lamp
  else if (e == "lamp_on") {eepr[START]=310; eepr[LENGTH]=10;}
  else if (e == "lamp_off") {eepr[START]=320; eepr[LENGTH]=10;}
  else if (e == "lamp_color") {eepr[START]=330; eepr[LENGTH]=10;}//#9696ff

  // INTEGER DATA -------------------------------------------------------------------
  //save integer 0-255 --> LENGTH = 0
  //else if (e == "lamp_status") {eepr[START]=400; eepr[LENGTH]=0;}// status lamp --> 1/0 --> for on/off manual
  //else if (e == "R") {eepr[START]=401; eepr[LENGTH]=0;}// RED --> not use, change with hex color --> lamp_color
  //else if (e == "G") {eepr[START]=402; eepr[LENGTH]=0;}// GREEN
  //else if (e == "B") {eepr[START]=403; eepr[LENGTH]=0;}// BLUE
  else if (e == "lamp_brig") {eepr[START]=404; eepr[LENGTH]=0;}// brightness
  else if (e == "lamp_mode") {eepr[START]=405; eepr[LENGTH]=0;}// mode lamp
  else if (e == "lamp_speed") {eepr[START]=406; eepr[LENGTH]=0;}// speed change color
  
  else if (e == "feed_level") {eepr[START]=410; eepr[LENGTH]=0;}// how much was fed --> 1 ~ 10 map to 0 ~ 180 servo degree
  else if (e == "feed_loop") {eepr[START]=411; eepr[LENGTH]=0;}// how many times it was fed --> 1 ~ 10
  else if (e == "feed_duration") {eepr[START]=412; eepr[LENGTH]=0;}// duration feeder if device offline (every ?? hour)

  /*
   * NOTE:
   * Pump not save in EEPROM ==> if device boot always on
   */
}
void writeEEPROM(String e, String data){
  getListEEPROM(e);
  if(eepr[LENGTH]==0)//save integer 0-255
    EEPROM.write(eepr[START],data.toInt());
  else{
    for (int i = 0; i < eepr[LENGTH]; ++i) {
      i<data.length()?
        EEPROM.write(i + eepr[START],data[i]):
        EEPROM.write(i + eepr[START],' ');//add space if not use (trim on read)
    }
  }
  EEPROM.commit();//ESP need commit after write
}

String readEEPROM(String e){
  getListEEPROM(e);
  String eeprData = "";
  if(eepr[LENGTH]==0)//read integer 0-255
    eeprData = EEPROM.read(eepr[START]);
  else{
    for (int i = 0; i < eepr[LENGTH]; ++i)
      eeprData += char(EEPROM.read(i+eepr[START]));
    eeprData.trim();
  }
  return eeprData;
}

void readAllDataEEPROM(){
  //LAMP -----------------------------------------------------------
  lamp_time[0]  = readEEPROM("lamp_off");
  lamp_time[1]  = readEEPROM("lamp_on");
  //lamp_status   = readEEPROM("lamp_status").toInt();
  lamp_brig     = readEEPROM("lamp_brig").toInt();
  lamp_mode     = readEEPROM("lamp_mode").toInt();
  lamp_speed    = readEEPROM("lamp_speed").toInt();
  lamp_color    = readEEPROM("lamp_color");
  
  //Verification ==> if eeprom not set
  lamp_time[0]  = lamp_time[0].length()!=5?"":lamp_time[0];
  lamp_time[1]  = lamp_time[1].length()!=5?"":lamp_time[1];
  //lamp_status   = (lamp_status!=1 || lamp_status !=0)?1:lamp_status;
  lamp_brig     = (lamp_brig<1 || lamp_brig >10)?7:lamp_brig;
  lamp_mode     = (lamp_mode<1 || lamp_mode >5)?1:lamp_mode;
  lamp_speed    = (lamp_speed<1 || lamp_speed >120)?10:lamp_speed;
  lamp_color    = lamp_color.length()!=7?"#9696FF":lamp_color;

  //FEED -----------------------------------------------------------
  feed_time[0]  = readEEPROM("feed1");
  feed_time[1]  = readEEPROM("feed2");
  feed_time[2]  = readEEPROM("feed3");
  feed_time[3]  = readEEPROM("feed4");
  feed_time[4]  = readEEPROM("feed5");
  
  feed_level    = readEEPROM("feed_level").toInt();
  feed_loop     = readEEPROM("feed_loop").toInt();
  feed_duration = readEEPROM("feed_duration").toInt();
  
  //Verification ==> if eeprom not set
  feed_time[0]  = feed_time[0].length()!=5?"08:00":feed_time[0];
  feed_time[1]  = feed_time[1].length()!=5?"":feed_time[1];
  feed_time[2]  = feed_time[2].length()!=5?"":feed_time[2];
  feed_time[3]  = feed_time[3].length()!=5?"":feed_time[3];
  feed_time[4]  = feed_time[4].length()!=5?"":feed_time[4];
  
  feed_level    = (feed_level<1 || feed_level >10)?5:feed_level;
  feed_loop     = (feed_loop<1 || feed_loop >10)?1:feed_loop;
  feed_duration = (feed_duration<1 || feed_duration >24)?8:feed_duration;

  //WIFI &NTP -----------------------------------------------------------
  e_ssid  = readEEPROM("ssid");
  e_pass  = readEEPROM("pass");
  e_ntp   = readEEPROM("ntp");
  e_gmt   = readEEPROM("gmt");
  
  //Verification ==> if eeprom not set
  e_ssid  = e_ssid.length()<2?"__DEFAULT_WIFI__":e_ssid;
  e_pass  = e_pass.length()<8?"":e_pass;
  e_ntp   = e_ntp.length()<1?"pool.ntp.org":e_ntp;
  e_gmt   = (e_gmt.toInt()<-12 || e_gmt.toInt()>12)?"7":e_gmt;
}
