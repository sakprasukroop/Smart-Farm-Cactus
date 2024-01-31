/*
// Sample code for ESP32  //
#include <WiFi.h>
#include <MicroGear.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
*/

// Sample code for ESP8266
#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

const char* ssid     = "Surasak";        
const char* password = "0875318445";    

const char* host =  "http://jawitt-nodemcu.herokuapp.com/bot.php"; 
//NETPIE-Info
#define APPID       "SURASAKLINEBOT"             //Same data at Line#3 of pub.php
#define KEY         "JkdiSqkiB9BSRkJ"            //Same data at Line#4 of pub.php
#define SECRET      "RP7ijqApYsXjRNhXmyG3KDTDv"  //Same data at Line#5 of pub.php

#define ALIAS   "NodeMCU"                        //set name of device

WiFiClient client;
int timer = 0;
MicroGear microgear(client);

#define led1 D0                // ON-OFF ตามคำสั่ง LINE Bot
#define led2 D4                // ON-OFF ตามเงื่อนไข Analog 
#define sw   D3                // SW สรุปข้อมูลทาง LINE Bot
byte AnalogValue;              // ตัวแปรเก็บค่าอนาล็อก
byte setpoint;                 // ตัวแปร setpoint Analog 
byte flag_setpoint =0;         // ตัวแปร ให้ทำงานเพียงครั้งเดียว เมื่อเงื่อนไขเป็นจริง 

// ฟังก์ชั่นรับข้อความจาก LINE Bot มาเก็บไว้ในตัวแปร msgLINE

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) 
{
    Serial.print("LINE MSG is");
    msg[msglen] = '\0';
    Serial.println((char *)msg);
    String msgLINE = (char *)msg;
    
    if ( msgLINE == "สวัสดี" ) send_json("สวัสดี ผมชื่อจ่าวิทย์ ยินดีรับใช้ครับ แนะนำให้พิมพ์ Lamp หรือ Analog หรือ Show");

    else if ( msgLINE == "Lamp" ) 
      {
        send_json("พิมพ์ เปิดไฟ หรือ On หรือ on");
        send_json("หรือพิมพ์ ปิดไฟ หรือ Off หรือ off");
        send_json("หรือพิมพ์ ขอดูสถานะหลอดไฟ หรือ Status Lamp");
      }  
    
    else if ( msgLINE == "ON" || msgLINE == "On" || msgLINE == "on" || msgLINE == "เปิดไฟ") 
      {
        send_json("เปิดไฟแล้วครับ Led1 ON....");
        digitalWrite(led1, LOW);         // Led1 on
      } 
      
    else if ( msgLINE == "OFF" || msgLINE == "Off"  || msgLINE == "off" || msgLINE == "ปิดไฟ" ) 
      {
        send_json("ปิดไฟแล้วครับ Led1 OFF....");
        digitalWrite(led1, HIGH);          // Led1 off
      }
      
    else if ( msgLINE == "ขอดูสถานะหลอดไฟ" || msgLINE == "Status Lamp") 
      {
        if (digitalRead(led1)== LOW) send_json("สถานะหลอดไฟ เปิดอยู่ครับ (Led1 ON)");
        else if (digitalRead(led1)== HIGH) send_json("สถานะหลอดไฟ ปิดอยู่ครับ (Led1 OFF)");
      }

     else if ( msgLINE == "Analog" ) 
      {
        send_json("ต้องการดูค่าอนาล็อก พิมพ์ Analog Read");
        send_json("หรือต้องการดูค่าที่ตั้ง พิมพ์ Setpoint Read");
        send_json("หรือต้องการตั้งค่า (00-99) เช่น พิมพ์ Setpoint 45");
      }  
      
     else if ( msgLINE == "Analog Read") 
      {
        String text = "ค่าอนาล็อกขณะนี้มีค่าเท่ากับ : " + String(AnalogValue) + "  ครับผม";
        send_json(text);
      }
      
     else if ( msgLINE == "Setpoint Read") 
      {
        String text = "การตั้งค่าอนาล็อกขณะนี้อยู่ที่ มากกว่าหรือเท่ากับ : " + String(setpoint) + "  Led2 ทำงาน ครับผม";
        send_json(text);
      }
      
     else if ( msgLINE == "RESET" || msgLINE == "Reset"  || msgLINE == "reset" ) 
      {
        send_json("System Resetting");
        delay(5000);
        ESP.restart();
      }
      
     else if ( msgLINE == "Show" ) show_data();  // เรียกฟังก์ชั่น show_data(); แสดงข้อมูลทั้งหมด
     
     else  // ตรวจสอบการตั้งค่า setpoint
      {
         String find_text = getstring(msgLINE, "Setpoint "); 
          // ใช้ฟังก์ชั่น getstring(); ตรวจสอบข้อความ ค้นหาข้อความใน msgLINE หลังจากคำว่า "Setpoint "   
          // ตัวอย่างเช่น msgLINE = "Setpoint 45" ผลคือ find_text = "45"
          // ตัวอย่างเช่น msgLINE = "Setpoint 73" ผลคือ find_text = "73"
          // ตัวอย่างเช่น msgLINE = "setpoint 45" ผลคือ find_text = "-1" แสดงว่าไม่เจอข้อความ (S ต้องพิมพ์ใหญ่)
          // จากนั้น นำตัวแปร find_text มาตรวจสอบเงื่อนไข 
         
         if (find_text == "-1") 
           {
            // แสดงว่าไม่มีข้อความที่ค้นหา
            Serial.println("No Messege"); 
            send_json("ไม่รู้จักครับ แนะนำให้พิมพ์ Lamp หรือ Analog หรือ Show"); 
           }
           else 
           {
            // แสดงว่ามีข้อความที่ค้นหา
              Serial.println("val = ......... = "+find_text); // แสดงข้อความที่ค้นหา
              setpoint = find_text.toInt();      // แปลงค่า String เป็น Int  
              EEPROM.write(0,setpoint);          // เก็บลง setpoint ลง EEPROM address 0
              EEPROM.commit();
 
              String text = "ตั้งค่าอนาล็อกเท่ากับ  " + String(setpoint) + "  เรียบร้อยแล้ว ครับผม";
              send_json(text);   // แสดงค่า setpoint ที่ LINE Bot
           }
      }
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) 
{
  Serial.println("NETPIE is connected");
  microgear.setName(ALIAS);
}

// ฟังก์ชั่นส่งข้อความกลับไปยัง LINE Bot

void send_json(String data) 
{
  StaticJsonDocument<300> JSONencoder;

  JSONencoder["ESP"] = data;

  JsonArray values = JSONencoder.createNestedArray("values"); //JSON array
  values.add(20); //Add value to array
  values.add(21); //Add value to array
  values.add(23); //Add value to array

  char JSONmessageBuffer[300];
  serializeJsonPretty(JSONencoder, JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println(JSONmessageBuffer);

  HTTPClient http;

  http.begin(host);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(JSONmessageBuffer);
  String payload = http.getString();

  Serial.println(httpCode);
  Serial.println(payload);

  http.end();
}

void setup() 
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(sw, INPUT_PULLUP);
  digitalWrite(led1, HIGH);   // Led1 OFF
  digitalWrite(led2, HIGH);   // Led2 OFF
  EEPROM.begin(512);
  setpoint = EEPROM.read(0);
  
  microgear.setEEPROMOffset(3000);
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  if (WiFi.begin(ssid, password)) 
    {
      while (WiFi.status() != WL_CONNECTED) 
      {
        delay(500);
        Serial.print(".");
      }
    }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  microgear.resetToken();
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);

  // ส่งข้อความทักทายทาง LINE Bot เมื่อเริ่มต้นการทำงาน
  send_json("สวัสดี ผมชื่อจ่าวิทย์ ยินดีรับใช้ครับ แนะนำให้พิมพ์ Lamp หรือ Analog หรือ Show");
}

void loop() 
{
  if (microgear.connected()) 
      {
        Serial.println("NETPIE is connecting!");
        microgear.loop();
        timer = 0;
      }
  else 
     {
      Serial.println("NETPIE is reconnecting...");
      if (timer >= 5000) {
        microgear.connect(APPID);
        timer = 0;
     }
    else timer += 100;
  }
  delay(100);

  // อ่านค่า อนาล็อก 0-1023 และแปลงให้อยู่ระหว่าง 0-100
  AnalogValue = map(analogRead(A0),0,1023,0,100);
  Serial.print("Analog : "); Serial.println(AnalogValue);

  // ตรวจสอบเงื่อนใข ให้ led2 ทำงาน เมื่อค่า AnalogValue มีค่าเท่ากับหรือมากกว่า setpoint
  if (AnalogValue >= setpoint && flag_setpoint ==0)
    {
      Serial.print("led2 on : Active Setpoint at >= "); Serial.println(setpoint);
      digitalWrite(led2,LOW); // led2 on
      flag_setpoint = 1;
      delay(2000);
    }
  else  if (AnalogValue < setpoint && flag_setpoint ==1)
    {
      digitalWrite(led2,HIGH); // led2 off
      flag_setpoint =0;
    }

  // ตรวจสอบการกด sw เพื่อแสดงรายละเอียดทั้งหมด
  if (digitalRead(sw)==0) show_data();
 
}

//----- ฟังก์ชั่นค้นหาข้อความ หลังจากคำว่า "Setpoint xx" (xx คือข้อความที่ต้องการค้นหา)----------------

String getstring(String data, String str_start)
{
  String Mag; 
  int lastStringLength = data.length();  // หาค่าจำนวนตัวอักษรทั้งหมด เก็บไว้ในตัวแปรแบบ int ชื่อ lastStringLength
  //Serial.println(lastStringLength);    // lastStringLength = 11 (ข้อความ "Setpoint 45" มีทั้งหมด 11 ตัว รวมเว้นวรรคด้วย)

  int StrLength = str_start.length();    // หาค่าจำนวนตัวอักษรเฉพาะข้อความที่ต้งการ
  //Serial.println(StrLength);           // StrLength = 9 (ข้อความ str_start = "Setpoint " มีทั้งหมด 9 ตัว รวมเว้นวรรคด้วย)
                                               
  int stratidex = data.indexOf(str_start); // ค้นหาตำแหน่งของข้อความเริ่มต้น 
  //Serial.println(stratidex);             // stratidex = 0 (0 หมายถึงตำแหน่งเริ่มต้นของ S)
                                           // *กรณีไม่เจอหรือข้อความความไม่ตรง เช่น setpoint ไม่เหมือน "Setpoint"  
                                           // จะคืนค่าออกมาเป็น -1 คือ stratidex = -1  
  if (stratidex == -1)
  {
    Mag = "-1";                            // Mag = "-1"   หมายถึงไม่เจอข้อความที่ค้นหา   
    return Mag;                            // ส่งค่ากลับ Mag = "-1" แล้วออกจากฟังกชั่นเลย 
  }
  Mag  = data.substring(stratidex + StrLength, lastStringLength);  // ตัดคำ ตั้งแต่คำที่ 0+9 ถึง 11 (เริ่มคำที่คำที่ 9 ถึง 11) 
  //Serial.println(Mag);                   // ดังนั้น Mag = "45" 
  return Mag;                              // ส่งค่า Mag แล้วออกจากฟังก์ชั่น 
}

// ฟังก์ชั่น สรุปข้อมูลทั้งหมด

void show_data()
{
   String text;

   send_json("จ่าวิทย์ ขอรายงานข้อมูลให้ทราบ ครับผม");

       // ตรวจสอบสถานะหลอดไฟ
    if (digitalRead(led1)== LOW) text = "Led1 ON,";
        else if (digitalRead(led1)== HIGH)  text = "Led1 OFF,";

    if (digitalRead(led2)== LOW) text = text + " Led2 ON,";
        else if (digitalRead(led2)== HIGH) text = text + " Led2 OFF,";

     text = text + " Analog = " + String(AnalogValue) + ", Setpoint = " + String(setpoint);
     send_json(text);   
}

