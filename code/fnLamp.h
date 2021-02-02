/*
 * COLOR MODE
 */
int modeColor1[] = {//RGB
  255,0,0,//red
  0,255,0,//green
  0,0,255,//blue
};
int modeColor2[] = {//mode 2
  255,200,20,
  70,255,70,
  150,150,255,
  255,255,255,
};
int modeColor3[] = {//mix all color
  255,  0,  0,
  255,  0,255,
  255,255,255,
  255,0  ,255,
  0  ,0  ,255,
  0  ,255,255,
  255,255,255,
  0  ,255,255,
  0  ,255,  0,
  255,255,  0,
  255,255,255,
  255,255,  0,
};





/*
 * Change hex color to RGB
 */
#define R 0
#define G 1
#define B 2
int hexInRGB[3]={150,150,255};// default light blue
void hexToRgb(String hexstring){
  PRINTLN("HEX to RGB : " +hexstring);
  hexstring.toUpperCase();
  long number = (long) strtol( &hexstring[1], NULL, 16);
  hexInRGB[R] = number >> 16;
  hexInRGB[G] = number >> 8 & 0xFF;
  hexInRGB[B] = number & 0xFF;
  PRINT(hexInRGB[R]);
  PRINT(",");
  PRINT(hexInRGB[G]);
  PRINT(",");
  PRINTLN(hexInRGB[B]);
}





/*
 * lamp function
 */
int rgbBrig;//brightness
int rgbIndex[3];
int rgbTmr;
int *rgbColor;
int rgbColorSize;
void fadeRGB(int v_color[], int sizeOfArrColor, int i){
  rgbColor = v_color;
  rgbColorSize = sizeOfArrColor;
  tmr.stop(rgbTmr);
  i=(i< rgbColorSize/3)?i:0;//jumlah / 3 ==> R+G+B ambil 3 index
  rgbIndex[0] = i;  //index color existing
  rgbIndex[1] = (i+1 < rgbColorSize/3)?i+1:0;//index ganti ke color selanjutnya; ==> jika warna terakhir ganti ke warna pertama
  rgbIndex[2] = 0;  //index looping perubahan color
  
  rgbTmr = tmr.every(50,[](){ //timer every 100(ms) * lamp_speed(detik) * 10(ms) = lamp_speed (ms)
    int buf;
    for (int i = 0; i < arrSize(PIN_LED); i++) {
      //map(value, fromLow, fromHigh, toLow, toHigh)
      //buf = map(rgbIndex[2], 0, lamp_speed*20, 255 - rgbColor[rgbIndex[0]*3+i], 255 - rgbColor[rgbIndex[1]*3+i]);//balik buat 
      buf = map(rgbIndex[2], 0, lamp_speed*20, rgbColor[rgbIndex[0]*3+i], rgbColor[rgbIndex[1]*3+i]);
      buf = map(buf, 0, 255, 0, rgbBrig);
      
      analogWrite(PIN_LED[i], buf);
      //PRINT(String(buf) + ", ");
    }
    //PRINTLN();
    
    if(rgbIndex[2]>=lamp_speed*20){ //timer every 100(ms) * lamp_speed(detik) * 10(ms) = lamp_speed (ms)
      tmr.stop(rgbTmr);
      fadeRGB(rgbColor,rgbColorSize,rgbIndex[0]+1);
    }
    else rgbIndex[2]++;
  });
}

void blinkRGB(int v_color[], int sizeOfArrColor, int i){
  if(i==0){
    rgbColor = v_color;
    rgbColorSize = sizeOfArrColor;
    tmr.stop(rgbTmr);
  }
  i=(i< rgbColorSize/3)?i:0;//jumlah / 3 ==> R+G+B ambil 3 index
  rgbIndex[0] = i;  //index color existing
  rgbIndex[1] = (i+1 < rgbColorSize/3)?i+1:0;//index ganti ke color selanjutnya; ==> jika warna terakhir ganti ke warna pertama
  rgbIndex[2] = 0;  //index looping perubahan color
  hexToRgb(lamp_color);
  
  rgbTmr = tmr.every(lamp_speed*10,[](){ //timer every 10(ms) * lamp_speed(s) * 10(ms) = lamp_speed (ms)
    int buf;
    if(rgbIndex[2]%10==0 || (rgbIndex[2]-2)%10==0){
      for (int i = 0; i < arrSize(PIN_LED); i++) {
        if(lamp_mode==4)//manual blink
          buf = hexInRGB[i];
        else
          buf = map(rgbIndex[2], 0, 99, rgbColor[rgbIndex[0]*3+i], rgbColor[rgbIndex[1]*3+i]);
          
        buf = map(buf, 0, 255, 0, rgbBrig);      
        analogWrite(PIN_LED[i], buf);
      }
    }
    else{
      digitalWrite(PIN_LED[0], LOW);
      digitalWrite(PIN_LED[1], LOW);
      digitalWrite(PIN_LED[2], LOW);
    }
    if(rgbIndex[2]<99){ //timer every 100(ms) * lamp_speed(detik) * 10(ms) = lamp_speed (ms)
      rgbIndex[2]++;
    }
    else {
      rgbIndex[0] = (rgbIndex[0]< rgbColorSize/3)?rgbIndex[0]+1:0;//jumlah / 3 ==> R+G+B ambil 3 index
      rgbIndex[1] = (rgbIndex[0]+1 < rgbColorSize/3)?rgbIndex[0]+1:0;//index ganti ke color selanjutnya; ==> jika warna terakhir ganti ke warna pertama
      rgbIndex[2] = 0;  //index looping perubahan color
    }
  });
}
void manualRGB(){
  tmr.stop(rgbTmr);
  hexToRgb(lamp_color);
  rgbTmr = tmr.every(100,[](){// continues loop --> for check brightness change
    int buf;
    for (int i = 0; i < arrSize(PIN_LED); i++) {
      buf = map(hexInRGB[i], 0, 255, 0, rgbBrig);
      analogWrite(PIN_LED[i], buf);
      //PRINT(String(buf) + ", ");
    }
    //PRINTLN();
  });
}
void manualFadeRGB(){
  tmr.stop(rgbTmr);
  rgbIndex[0] = 0;
  hexToRgb(lamp_color);
  rgbTmr = tmr.every(50,[](){
    int buf;
    for (int i = 0; i < arrSize(PIN_LED); i++) {
        buf = map(hexInRGB[i], 0, 255, 0, rgbBrig);
        if(rgbIndex[0]<lamp_speed*20/2)
          buf = map(rgbIndex[0], 0-(lamp_speed*20/5), lamp_speed*20/2, 0, buf);
        else
          buf = map(rgbIndex[0], lamp_speed*20/2, (lamp_speed*20)+(lamp_speed*20/5), buf, 0);
        analogWrite(PIN_LED[i], buf);
        //PRINT(String(buf) + ", ");
    }
    //PRINTLN("");
    rgbIndex[0] = rgbIndex[0]>=lamp_speed*20?0:rgbIndex[0]+1;
  });
}
void defaultRGB(){
  tmr.stop(rgbTmr);
  rgbTmr = tmr.every(100,[](){// continues loop --> for check brightness change
    int buf[3]={150,150,255};
    for (int i = 0; i < arrSize(PIN_LED); i++) {
      analogWrite(PIN_LED[i], map(buf[i], 0, 255, 0, rgbBrig));
    }
  });
}

void fn_RGBstop(){
  tmr.stop(rgbTmr);
  for (int i = 0; i < arrSize(PIN_LED); i++) {
    digitalWrite(PIN_LED[i], LOW);
  }
}
void fn_RGBstart(){
  PRINT("Lamp Mode : ");
  PRINTLN(lamp_mode);
  //ESP8266 ==> analog write 0-1023
  rgbBrig     = map(lamp_brig,1,10,100,1023);//minimal brightness = 100 ==> ESP8266 analogWrite 0-1023
  switch(lamp_mode){
    case 1:// default
      defaultRGB();
      break;
    case 2://manual color
      manualRGB();
      break;
    case 3://manual fade
      manualFadeRGB();
      break;
    case 4:// manual blink
      blinkRGB(modeColor1,3,0);//modeColor1 buat dummy data ==> di fungsi blinkRGB langsung ambil dari EE
      break;
    
    //FADE---------------------------------------
    case 5:
      fadeRGB(modeColor1,arrSize(modeColor1),0);
      break;
    case 6:
      fadeRGB(modeColor2,arrSize(modeColor2),0);
      break;
    case 7:
      fadeRGB(modeColor3,arrSize(modeColor3),0);
      break;
      
    //BLINK----------------------------------------
    case 8:
      blinkRGB(modeColor1,arrSize(modeColor1),0);
      break;
    case 9:
      blinkRGB(modeColor2,arrSize(modeColor2),0);
      break;
    case 10:
      blinkRGB(modeColor3,arrSize(modeColor3),0);
      break;
  }
}
