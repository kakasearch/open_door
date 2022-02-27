#define BLINKER_WIFI
#define BLINKER_PRINT Serial
#define BLINKER_MIOT_OUTLET
#define BLINKER_DUEROS_OUTLET
#define BLINKER_WITHOUT_SSL
#include <Blinker.h>
#include <ESP8266WiFi.h>
#include <Wiegand.h>
#define blinksk "53edf7d7243f"
#define STASSID "HiWiFi_5C0B98"
#define STAPSK  "gzwl6009"
#define maxd 180
#define mind 5
#define ServoPin D8 //d8
#define PIN_D0 D2    
#define PIN_D1 D3    //d6
Wiegand wiegand;
WiFiServer server(35614);//开启板子的port 80
int oState = 0;
byte mdata[5][4] = { 
        {0x03,0x0B,0x31},
        {0x1C,0xE8,0xA7},
        {0x1E,0xC8,0x11},
        {0x17,0xAA,0xB0},
        {0x27,0xE7,0xAA}             
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
void open_door(int interval) {
  Serial.println("openning door");
  ServoControl(mind);
  delay(interval);
  ServoControl(maxd); 
  oState = 0;
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

ICACHE_RAM_ATTR void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}
ICACHE_RAM_ATTR void receivedData1(uint8_t* data, uint8_t bits, const char* message) {
    Serial.print(message);
    Serial.print(bits);
    Serial.print("bits / ");
    //Print value in HEX
    uint8_t bytes = (bits+7)/8;
    for (int i=0; i<bytes; i++) {
        Serial.print(data[i] >> 4, 16);
        Serial.print(data[i] & 0xF, 16);
    }
    Serial.println();
    for (byte i = 0; i <5; i++) {
      byte r_num = 0;
      for (byte j = 0; j <3; j++) {
        if (data[j] == mdata[i][j]){
          r_num ++;
        }
    }
    if(r_num == 3){
       Serial.println("card ok");
       oState = 4;
       return;
      }
    }
    Serial.println("card error");
}

ICACHE_RAM_ATTR void receivedData(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
    Serial.print(message);
    for (int i=0; i<4; i++) {
        Serial.print(rawData[i] >> 4, 16);
        Serial.print(rawData[i] & 0xF, 16);
    }
    Serial.println();
    for (byte i = 0; i <5; i++) {
      byte r_num = 0;
      for (byte j = 0; j <3; j++) {
        if (rawData[j] == mdata[i][j]){
          r_num ++;
        }
    }
    if(r_num == 4){
      oState = 4;
      Serial.print("card ok");
      }
    }
    Serial.print("card error");
}

void rfc_run(){
  noInterrupts();
  wiegand.flush();
  interrupts();
}
void setup() {
  wiegand.onReceiveError(receivedData, "Card read: ");
  wiegand.onReceive(receivedData1, "~Card read: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);
  pinMode(PIN_D0, INPUT);
  pinMode(PIN_D1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), pinStateChanged, CHANGE);
  pinStateChanged();

  pinMode(ServoPin,OUTPUT);
  Serial.begin(9600);//开启端口，速度为115200
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
  Serial.println("\nWiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
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
