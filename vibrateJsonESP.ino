#include <SoftwareSerial.h>
#include <DS1307RTC.h>
#include <ArduinoJson.h>

#define SHOCK 8 // 핀 설정(디지털 신호 받는 핀)

#include <WiFiEsp.h>
#include <ESP8266.h>
#include <ESP8266Client.h>

#define DEBUG true
#define TO (1000)

int RX = 2;
int TX = 3;
SoftwareSerial esp8266(RX, TX);

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600);
  esp8266.setTimeout(5000);

  sendData("AT+RST\r\n", TO*2, DEBUG); //reset module
  sendData("AT+CWMODE=1\r\n", TO, DEBUG); // Client mode
  sendData("AT+CWJAP=\"<WiFi>\",\"<PassWord>\"\r\n", TO*5, DEBUG); //사용할 공유기 설정
  sendData("AT+CIPMUX=1\r\n", TO, DEBUG); //multiple connections 설정
  sendData("AT+CIPSERVER=1,<port_num>\r\n", TO, DEBUG); // 공유기와의 연결 포트 번호를 8080번으로 설정. default 포트는 333이다.
  sendData("AT+CIFSR\r\n", TO, DEBUG); // 부여받은 IP 번호 확인

  Serial.println("# Trying to connect to web server...");
  while(sendData("AT+CIPSTART=1,\"TCP\",\"<server-address>\"\r\n",TO*5,DEBUG)[54]=='E') {
    Serial.println("# ReTrying to Connect Web Server After Close All Connection");
      sendData("AT+CIPCLOSE=5\r\n", TO, DEBUG); // 모든 연결 강제종료
      sendData("AT+CIPMUX=1\r\n", TO, DEBUG); // multiple connections 설정
      delay(TO*3);
  }
  Serial.println("# Connecting is completed.\n-----------loop function start-----------");
}

void loop() {

  StaticJsonDocument<256> jsonDoc;
  JsonObject root = jsonDoc.to<JsonObject>();

  if (digitalRead(SHOCK) == 1) {
    root["viberatevalue"] = digitalRead(SHOCK);
    //serializeJsonPretty(root, Serial);
    //Serial.write("\n");
    
    esp8266.println("AT+CIPSEND=100");
    delay(TO/2);
    esp8266.print("GET /req/event&json=");
    delay(100);
    esp8266.print("{\"viberatevalue\": 1}");
    delay(100);
    esp8266.print(" HTTP/1.1");
    delay(TO);
    }
 
  if (Serial.available()) {
    esp8266.write(Serial.read());
  }
  
  if (esp8266.available()) {
    Serial.write(esp8266.read());
  }

}

String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  esp8266.print(command); // Send command to ESP8266 module
  long int time = millis();

  while((time+timeout)>millis()) {
    while(esp8266.available()) {
      // It is neccessary to print out the value which ESP8266 module has.
      char c = esp8266.read();
      response+=c;
    }
  }
  if(debug) {
    Serial.print(response);
  }

  return response;
}
