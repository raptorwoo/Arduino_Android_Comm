/*
 * Programmer: Jihwan Kim 
 * Date: 2018.06.28
 * Objective: To Create a Arduino Program that enables Arduino Devices to
 * receive Data as JSON Format from Android Device, formulate them, 
 * and send them back to the Android Device 
 * 
 * The MIT License (MIT)

Copyright (c) <2018> <Jihwan Kim>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */





#include <Bounce2.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <MsTimer2.h>   

//PIN0(RX),PIN1(TX) 은 시리얼모니터 통신과 관계 있으므로 사용에 유의
#define LED_PIN 13
#define INPUT_BUTTON 7
#define BOUNCE_INTERVAL 5
#define RECEVE_MAX 20

SoftwareSerial hm10(2,3);   //RX, TX Connection 
Bounce golfShotButton = Bounce();

/*
 * 1. 안드로이드에서 Generate 된 Hard Coded Data를 아두이노로 보낸다. 
 * 2. 아두이노에서 데이터를 Parse 하고 Parse가 끝나면 LED를 두 번 깜빡인다. 
 * 3. 아두이노의 버튼을 누르면 hm10을 통해 아두이노에서 안드로이드로 Generate된 데이터를 보낸다
 * 4. 안드로이드에서 값을 받고 Parse 한다
 */
//=============GLOBAL VARIABLES=========
String payload = "";        //stores Data received from Bluetooth
String coreData = "";        //JSON DATA that has been extracted from the String 
String dataSize = "";        //Size of the Buffer extracted from the String  

//==============FLAG==================
boolean ledWinkerFlag = false;
boolean onOffFlag = false;
boolean generateAuthorizeFlag = false;
//===================================

//When Generating, the measurement values will rise by 1 for debugging purposes
//Global Variables for Generation
int ballSpeed = 100;
int clubSpeed = 50;
int startAngle = 30;
int startTando = 60;
int distance = 800;
int curve = 400;

//
int  rcvIdx=0;
char rcvData[RECEVE_MAX];

void setup() {
  Serial.begin(9600);
  hm10.begin(9600);   // Bluetooth Module
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);       //내부 LED OFF
  
//Golf Shot Mock-Up Button ---------------------------------------------
  pinMode(INPUT_BUTTON, INPUT);
  digitalWrite(INPUT_BUTTON, HIGH);   //아두이노 내부 풀업저항 작동
  golfShotButton.attach(INPUT_BUTTON);
  golfShotButton.interval(BOUNCE_INTERVAL);
//----------------------------------------------------------------------

  MsTimer2::set(1, buttonPressed);        //Setting Timer for a Function
  Serial.println(F("Hello! Setting Completed"));
}

void loop() { 
   
  receiveBleData();
}

void receiveBleData(){
  while(hm10.available()) {
    
    rcvData[rcvIdx]= hm10.read();       //whatever Data read from Android 
    /*
    Serial.print(rcvIdx);
    Serial.print(":");
    Serial.print(rcvData[rcvIdx]);
    Serial.print(",");
    */
    
    if (rcvIdx==0 && rcvData[rcvIdx] == 03) {   //Start of JSON String ASCII CODE 0X03
       //수신시작
       Serial.println("startdata");
    }
    if(rcvIdx==(RECEVE_MAX-1) && rcvData[rcvIdx]  == 04) {       //End of JSON String ASCII CODE OXO4
      //수신 완료
      Serial.println("enddata");
      rcvIdx=0;
      parseReceiveData();
      break;
    }
    rcvIdx++;
    delay(5);       //데이터가 완전히 읽어지게 하기 위해 5ms의 딜레이를 준다
  }

}

void parseReceiveData(){
   //수신데이타 출력
    Serial.print("[");
    for(int i = 0; i < RECEVE_MAX; i++)
    {
      Serial.print(i);
      Serial.print(":");
      Serial.print(rcvData[i],HEX);
      Serial.print(",");
    }
    Serial.println("]");

      int cmd =rcvData[1];
      int club = rcvData[2];
      //int target = rcvData[3]*256 + rcvData[4]; 
      int target = rcvData[3]*256*256*256 + rcvData[4]*256*256+ rcvData[5]*256+ rcvData[6];   
      int ball = rcvData[7];   
      int inout = rcvData[8];   
      
//----------------------Sectioning and Printing Parsed Data-----------------------------------
      Serial.print(F("cmd status::  "));
      Serial.println(cmd);

      Serial.print(F("Club Type::  "));
      Serial.println(club);

      Serial.print(F("Target Distance ::  "));
      Serial.println(target);

      Serial.print(F("Position of the ball  ::"));
      if (ball == 0) Serial.println(F( "On Tee" ));
      else Serial.println(F("On Ground"));

      Serial.print(F("Location  ::"));
      if (inout == 0) Serial.println(F( "Indoors" ));
      else Serial.println(F("Outdoors\nParsing Done"));
      
      payload = "";    // payload (블루투스 통신으로 받은 모든 정보를 갖는 변수) 초기화시킴
      coreData = "";    // coreData (payload에서 JSON DATA 를 잘라 포함받는 변수) 초기화시킴
      dataSize = "";

      digitalWrite(LED_PIN,HIGH);   //Turn the LED On When Parsing is Done
      delay(1);
      MsTimer2::start();
      generateAuthorizeFlag = true;
}

/*
 * When Parsing is Done, the program waits until button is pressed, with the LED Light On. 
 * When the button is Pressed, LED Goes Off and JSON Data is Generated through generateJsonData() method
 */
void buttonPressed(){
  boolean changed = golfShotButton.update();
    if(changed){    //If the pressed at any moment,
        Serial.println(F("#300 Button Pressed"));   
        boolean value = golfShotButton.read();
        if(value == HIGH) {
          //Button is not pressed
        }
        else generateJsonData();
    }
}



//------------------------------

void generateJsonData(){    //Generating Done in this Section =========================
  digitalWrite(LED_PIN,LOW);
  Serial.println(F("#400 Entering Generating Phase"));
  StaticJsonBuffer <200> jsonBuffer;  
  JsonObject& root = jsonBuffer.createObject();

    root["rt"] = 0;  
    root["Err_Msg"] = "";
    root["ball_speed"] = ballSpeed;
    root["club_speed"] = clubSpeed;
    root["start_angle"] = startAngle;
    root["start_tando"] = startTando;
    root["distance"] = distance;
    root["curve"] = 400;  
    root["x_point"] = "37.586133";
    root["y_point"] = "127.076411";
    
    char startByte = 03;        // ASCII OXO3 
    int dataLength = 1000;
    char jsonToChar[256]; // to generate a char[]:
    root.printTo(jsonToChar, sizeof(jsonToChar));       //Convert created JSON Data to Char data type 
    char endByte = 04;        
    Serial.print(F("Generated Data  :"));
    Serial.print(startByte);
    Serial.print(dataLength);
    Serial.print(jsonToChar);
    Serial.println(endByte);
    
    hm10.write(startByte);    //1st open 
    hm10.write(dataLength);   //2nd length of data fixed to 1000 bytes
    hm10.write(jsonToChar);   //3rd 
    hm10.write(endByte);      //4th enclose
    
    MsTimer2::stop();
    jsonBuffer.clear();   //clearing buffer for next use
    generateAuthorizeFlag = false;

    ballSpeed++;
    clubSpeed++;
    startAngle++;  
    startTando++;
    distance++;
    curve++;
}
