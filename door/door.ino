#define BLINKER_WIFI
#define BLINKER_PRINT Serial
#define BLINKER_MIOT_OUTLET
#define BLINKER_DUEROS_OUTLET
#define BLINKER_WITHOUT_SSL
#include <Blinker.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         5           // 配置针脚
#define SS_PIN          4
#define blinksk "blink的密钥"
#define STASSID "wifi名字"
#define STAPSK  "wifi密码"



#define maxd 180
#define mind 5
#define led 10         //s3
#define buzzerPin 2 //D3 喇叭弃用
#define ServoPin 15 //d8
WiFiServer server(80);//开启板子的port 80
int oState = 0;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;
byte data[5][4] = { 0x19,0x04,0x24,0xBE, //管理员的16进制卡号，数组里的uid将放行
                    0x01,0xC1,0x1E,0x1C,                 
};
void ServoControl(int servoAngle)
{
  Serial.println("servo");
  Serial.println(servoAngle);
  double thisAngle =  (servoAngle - 0) * 2000 / (180.0) + 500; map(servoAngle, 0, 180, 500, 2500);//等比例角度值范围转换高电平持续时间范围
  unsigned char i = 50;//50Hz 每秒的周期次数(周期/秒) 即1S 50 个周期 每个周期20ms
  while (i--)
  {
    digitalWrite(ServoPin, HIGH); 
    delayMicroseconds(thisAngle); //高电平时间
    digitalWrite(ServoPin, LOW); 
    delayMicroseconds(20000 - thisAngle);//每个周期20ms减去高电平持续时间
  }
}

void beep(){
  int i;
  for (i=0; i<3; i++) { // goes from 180 degrees to 0 degrees
    tone(buzzerPin,800,500);
    delay(1000);
  };
  } 
void open_door(int interval) {
  ServoControl(mind);
  //beep();
  digitalWrite(led, HIGH);
  delay(interval);
  ServoControl(maxd); 
  //tone(buzzerPin,800,500);
  oState = 0;
  digitalWrite(led, LOW);
}


void miotPowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();
        oState = 1;
    }
    else if (state == BLINKER_CMD_OFF) {
        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();
        oState = 0;
    }
}
void DuerOSPowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
        BlinkerDuerOS.powerState("on");
        BlinkerDuerOS.print();
        oState = 2;
    }
    else if (state == BLINKER_CMD_OFF) {
        BlinkerDuerOS.powerState("off");
        BlinkerDuerOS.print();
        oState = 0;
    }
}


void miotQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_ALL_NUMBER :
            BLINKER_LOG("MIOT Query All");
            BlinkerMIOT.powerState(oState ? "on" : "off");
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
            BLINKER_LOG("MIOT Query Power State");
            BlinkerMIOT.powerState(oState ? "on" : "off");
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.powerState(oState ? "on" : "off");
            BlinkerMIOT.print();
            break;
    }
}

void dataRead(const String & data)
{
    BLINKER_LOG("Blinker readString: ", data);

    Blinker.vibrate();
    
    uint32_t BlinkerTime = millis();
    
    Blinker.print("millis", BlinkerTime);
}

void duerQuery(int32_t queryCode)
{
    BLINKER_LOG("DuerOS Query codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :        
            BLINKER_LOG("DuerOS Query power state");
            BlinkerDuerOS.powerState(oState ? "on" : "off");
            BlinkerDuerOS.print();
            break;
        case BLINKER_CMD_QUERY_TIME_NUMBER :
            BLINKER_LOG("DuerOS Query time");
            BlinkerDuerOS.time(millis());
            BlinkerDuerOS.print();
            break;
        default :
            BlinkerDuerOS.powerState(oState ? "on" : "off");
            BlinkerDuerOS.print();
            break;
    }
}


void server_run(){
    WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);//打印出收到的讯息
  client.flush();//流刷新，如果不flush，那么可能就堵塞了
    // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n ok!\n";
  // Send the response to the client
  client.print(s);
  // 比对收到的讯息，确定执行什么操作
  if (req.indexOf("?gpio=on") != -1){
    oState = 3;
  }else {
    Serial.println("invalid request");
  }
  client.flush();//刷新
}
int check_uid(byte *buffer) {
  Serial.print("uid:");
  for (byte i = 0; i <4; i++) {
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
  for (byte i = 0; i <5; i++) {
    byte r_num = 0;
    for (byte j = 0; j <4; j++) {
      if (buffer[j] == data[i][j]){
        r_num ++;
      }
    }
    if(r_num == 4){
      return 1;
      }
  }
  return 0;
}
void rfc_run(){
  
  // 寻找新卡
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //Serial.println("没有找到卡");
    return;
  }
  // 选择一张卡
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    Serial.println("没有卡可选");
    return;
  }
  if(check_uid(mfrc522.uid.uidByte)){
     Serial.println("card yes");
      oState = 4;
    }else{
      Serial.println("card error");
      oState = 0;
      }
  //停止 PICC
 // mfrc522.PICC_HaltA();
  //停止加密PCD
//  mfrc522.PCD_StopCrypto1();
  //return;
  }
void setup() {
  //pinMode(buzzerPin,OUTPUT);
  pinMode(ServoPin,OUTPUT);
  pinMode(led,OUTPUT);
  Serial.begin(115200);//开启端口，速度为115200
  // Connect to WiFi network
  Serial.print("Connecting to ");  
  // 初始化blinker
  BLINKER_DEBUG.stream(Serial);
  Blinker.begin(blinksk,STASSID,STAPSK);
  Blinker.attachData(dataRead);
  BlinkerMIOT.attachPowerState(miotPowerState);
  BlinkerMIOT.attachQuery(miotQuery);

  BlinkerDuerOS.attachPowerState(DuerOSPowerState);
  BlinkerDuerOS.attachQuery(duerQuery);
 while (WiFi.status() != WL_CONNECTED) {//连线成功后停止跳点
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
  //open_door();
  SPI.begin();        // SPI开始
  mfrc522.PCD_Init(); // Init MFRC522 card
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
}

void loop() {
  // Check if a client has connected
  Blinker.run();
  server_run();
  rfc_run();
  if(oState == 1){
    open_door(15000);
  }
  if(oState ==2){
    open_door(8000);
  }
  if(oState == 3){
    open_door(15000);
  }
  if(oState == 4){
    open_door(5000);
  }
}
