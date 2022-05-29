#define BLINKER_WIFI
#define BLINKER_PRINT Serial
#define BLINKER_MIOT_OUTLET
#define BLINKER_DUEROS_OUTLET
#define BLINKER_ALIGENIE_OUTLET
#define BLINKER_WITHOUT_SSL
#include <Blinker.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wiegand.h>
#define maxd 180 //开门时舵机偏转角度
#define mind 5 //关门时舵机偏转角度
#define ServoPin D8 //d8 舵机引脚
#define PIN_D0 D2   //IC卡读卡器引脚0
#define PIN_D1 D3    //d6  IC读卡器引脚1


#define blinksk "sksksksksksksksk" //blinker的sk,需要自己去blinker注册
#define STASSID "wifiwifiwifi" //wifi名称
#define STAPSK  "password" //wifi密码
#define len_users 2 //用户数量
#define len_card 3 //卡号长度，1-3
byte users[len_users][len_card] = { 
        {0x34,,0x11,ox9F},
        {0x32,,0x12,ox92},
};
ESP8266WebServer server(35614);//自定义的网络端口号

Wiegand wiegand;
int oState = 0;
//////////////执行开关门动作
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
///////////////////语音助手
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

void aligeniePowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
        digitalWrite(LED_BUILTIN, HIGH);

        BlinkerAliGenie.powerState("on");
        BlinkerAliGenie.print();
        oState = 2;
    }
    else if (state == BLINKER_CMD_OFF) {
        digitalWrite(LED_BUILTIN, LOW);

        BlinkerAliGenie.powerState("off");
        BlinkerAliGenie.print();
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

//////////////////web服务器控制
void homepage() {
  server.send(200, "text/plain", "ok !");
  Serial.println("用户访问了网页");
  oState = 3;
}
void handleNotFound() {                                       // 当浏览器请求的网络资源无法在服务器找到时，
  server.send(404, "text/plain", "404: Not found");           // NodeMCU将调用此函数。
}

///////////////////////读卡器控制
void check_card(uint8_t* rawData){
      for (byte i = 0; i <len_users; i++) {
    //遍历每张卡 
      byte r_num = 0;
      Serial.print("验证卡号：");
      for (byte j = 0; j <len_card; j++) {
        Serial.print(users[i][j] >> 4, 16);
        Serial.print(users[i][j] & 0xF, 16);
    }
     Serial.println();
      for (byte j = 0; j <len_card; j++) {
        if (rawData[j] == users[i][j]){
          r_num ++;
        };
    }
    if(r_num == len_card){
    //判断卡id是否正确
      oState = 4;
      Serial.println("card ok");
      return;
      }
    }
    Serial.println("card error");
}
ICACHE_RAM_ATTR void pinStateChanged() {
  wiegand.setPin0State(digitalRead(PIN_D0));
  wiegand.setPin1State(digitalRead(PIN_D1));
}
ICACHE_RAM_ATTR void receivedData1(uint8_t* data, uint8_t bits, const char* message) {
    Serial.print(message);
    Serial.print(bits);
    Serial.print("bits, 卡号：");
    //Print value in HEX
    uint8_t bytes = (bits+7)/8;
    for (int i=0; i<bytes; i++) {
        Serial.print(data[i] >> 4, 16);
        Serial.print(data[i] & 0xF, 16);
    }
    Serial.println();
    check_card(data);
}

ICACHE_RAM_ATTR void receivedData(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
    Serial.print(message);
    for (int i=0; i<4; i++) {
        Serial.print(rawData[i] >> 4, 16);
        Serial.print(rawData[i] & 0xF, 16);
    }
    Serial.println();
    check_card(rawData);
}
void rfc_run(){
  noInterrupts();
  wiegand.flush();
  interrupts();
}



///////////////////////程序入口
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
  BlinkerAliGenie.attachPowerState(aligeniePowerState);
  Serial.println("\nWiFi connected");
  //初始化WebServer
  server.on("/gpio", homepage);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
}

void loop() {
  // Check if a client has connected
  Blinker.run();
  server.handleClient();
  rfc_run();
  // 不同途径开门会导致不同的开门等待时间
  if(oState == 1){
  //小爱同学开门
    open_door(15000);
  }
  if(oState ==2){
  //其他语音助手开门
    open_door(8000);
  }
  if(oState == 3){
  //通过http开门
    open_door(15000);
  }
  if(oState == 4){
  //通过刷卡开门
    open_door(5000);
  }
}
